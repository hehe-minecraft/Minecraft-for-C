module core.element;

import std;
import core.constant;

constexpr std::size_t max_string_size = std::numeric_limits<std::uint8_t>::max();

class SerializeContext
{
	using byte = serialization::byte;
	using serial = serialization::serial;
	struct memo
	{
		std::size_t index = 0;
		bool used = false;
	};
	protected:
		std::deque<serial> instructions;
		std::unordered_map<const element::Element*, memo> memos; // The classic pointers are not to be de-referenced.
		std::uint32_t current_memo_id = 0;
	public:
		SerializeContext() = default;
		SerializeContext(const SerializeContext&) = delete;
		SerializeContext(SerializeContext&&) = delete;
		template <typename number_type>
			requires std::is_arithmetic_v<number_type>
		static inline serial generate_instruction_with_number(choices::serialization::op_code op_code, number_type number) noexcept
		{
			auto data = std::bit_cast<std::array<byte, sizeof(number_type)>>(number);
			// Switch to big endian mode to serialize numbers.
			if constexpr (std::endian::native != std::endian::big)
			{
				std::reverse(data.begin(), data.end());
			};
			serial instruction;
			instruction.reserve(sizeof(choices::serialization::op_code) + sizeof(number_type));
			instruction.emplace_back(op_code);
			instruction.insert_range(instruction.end(), data);
			return instruction;
		};
		inline void add_instruction(serial&& instruction)
		{
			this->instructions.push_back(std::move(instruction));
		};
		template <typename number_type>
			requires std::is_integral_v<number_type> or std::is_floating_point_v<number_type>
		inline void add_instruction_with_number(choices::serialization::op_code op_code, number_type number) noexcept
		{
			this->add_instruction(generate_instruction_with_number(op_code, number));
		};
		bool search_memo(const element::Element* object_pointer)
		{
			using choices::serialization::op_code;
			auto object_memo_index = this->memos.find(object_pointer);
			if (object_memo_index == this->memos.end())
			{
				return false;
			};
			if (not object_memo_index->second.used) // "second" refers to value.
			{
				serial& memo_instruction = this->instructions[object_memo_index->second.index];
				if (std::in_range<std::uint8_t>(this->current_memo_id))
				{
					memo_instruction = this->generate_instruction_with_number(op_code::memoize_used_1, static_cast<std::uint8_t>(this->current_memo_id));
				}
				else if (std::in_range<std::uint16_t>(this->current_memo_id))
				{
					memo_instruction = this->generate_instruction_with_number(op_code::memoize_used_2, static_cast<std::uint16_t>(this->current_memo_id));
				}
				else // It must be shorter than 4 bytes.
				{
					memo_instruction = this->generate_instruction_with_number(op_code::memoize_used_4, static_cast<std::uint32_t>(this->current_memo_id));
				};
				object_memo_index->second.index = this->current_memo_id;
				object_memo_index->second.used = true;
				if (this->current_memo_id == std::numeric_limits<std::uint32_t>::max())
				{
					throw errors::SerializeMemoOverflowError();
				};
				this->current_memo_id++;
			};
			const auto memo_id = static_cast<std::uint32_t>(object_memo_index->second.index);
			if (std::in_range<uint8_t>(memo_id))
			{
				this->add_instruction_with_number(op_code::ref_1, static_cast<std::uint8_t>(memo_id));
			}
			else if (std::in_range<uint16_t>(memo_id))
			{
				this->add_instruction_with_number(op_code::ref_2, static_cast<std::uint16_t>(memo_id));
			}
			else // It must be shorter than 4 bytes, since current_memo_id is always "uint32_t".
			{
				this->add_instruction_with_number(op_code::ref_4, static_cast<std::uint32_t>(memo_id));
			};
			return true;
		};
		void add_memo(const element::Element* object_pointer) noexcept
		{
			this->add_instruction({ choices::serialization::op_code::memoize_unused });
			this->memos.insert({ object_pointer, memo
			{
				.index = this->instructions.size() - 1,
				.used = false
			} });
		};
	public:
		serial arrange_output() const noexcept
		{
			serial optimized_instructions{ this->generate_instruction_with_number(choices::serialization::op_code::version_2, constants::serialization::version) };
			optimized_instructions.reserve(this->instructions.size() * constants::serialization::average_instruction_length);
			for (const serial& instruction : this->instructions)
			{
				if (instruction[0] == choices::serialization::op_code::memoize_unused)
				{
					continue;
				};
				optimized_instructions.insert_range(optimized_instructions.end(), instruction);
			};
			optimized_instructions.emplace_back(choices::serialization::op_code::stop);
			return std::move(optimized_instructions);
		};
};

class DeserializeContext
{
	using byte = serialization::byte;
	using serial = serialization::serial;
	struct stack_frame
	{
		const choices::serialization::deserialize_frame_type type;
		std::queue<element::element_ptr> items;
	};
	struct repeat_frame
	{
		// Using std::ranges::views here only makes it more complex.
		std::uint32_t begin;
		std::uint32_t index;
		std::uint32_t end;
		std::uint32_t repeat_times_left;
	};
	protected:
		const std::span<byte> instructions;
		std::stack<stack_frame> frame_stack;
		std::queue<element::element_ptr> current_frame;
		std::stack<repeat_frame> repeat_stack;
		std::unordered_map<std::uint32_t, element::element_ptr> memos;
		std::vector<byte> pop_instruction_length(std::uint32_t length)
		{
			std::vector<byte> result{};
			result.reserve(length);
			repeat_frame top_repeat_frame = this->repeat_stack.top();
			std::uint32_t length_left = top_repeat_frame.end - top_repeat_frame.index;
			while (length > length_left) // Repeat overflow
			{
				result.insert_range(result.end(), this->instructions.subspan(top_repeat_frame.index, length_left));
				length -= length_left;
				if (top_repeat_frame.repeat_times_left)
			{
					top_repeat_frame.repeat_times_left--;
					top_repeat_frame.index = top_repeat_frame.begin;
				}
				else if (this->repeat_stack.size() == 1) // Only the bottom layer left.
				{
				throw errors::DeserializeEOFError();
				}
				else
				{
					this->repeat_stack.pop();
					top_repeat_frame = this->repeat_stack.top();
			};
				length_left = top_repeat_frame.end - top_repeat_frame.index;
			};
			result.insert_range(result.end(), this->instructions.subspan(top_repeat_frame.index, length));
			top_repeat_frame.index += length;
			this->repeat_stack.top() = top_repeat_frame; // Modify the original content.
			return result;
		};
		template <typename target>
			requires std::is_default_constructible_v<target>
		target pop_instruction_as()
		{
			std::array<byte, sizeof(target)> result{};
			std::vector<byte> instruction = this->pop_instruction_length(sizeof(target));
			if constexpr (std::is_arithmetic_v<target> and std::endian::native == std::endian::little)
			{
				std::reverse_copy(instruction.begin(), instruction.end(), result.begin());
			}
			else
			{
				std::copy(instruction.begin(), instruction.end(), result.begin());
			};
			return std::bit_cast<target>(result);
		};
		inline element::element_ptr pop_element()
		{
			element::element_ptr element = this->current_frame.front(); // The element pushed at first
			this->current_frame.pop();
			return std::move(element);
		};
		element::element_ptr get_latest_element() const
		{
			if (not this->current_frame.empty())
			{
				return this->current_frame.back(); // The element pushed at last
			};
			if (this->frame_stack.empty() or this->frame_stack.top().items.empty())
			{
				return nullptr;
			};
			return this->frame_stack.top().items.back(); // The element pushed at last
		};
		template <choices::serialization::deserialize_frame_type type>
		inline void push_frame() noexcept
		{
			stack_frame new_frame =
			{
				.type = type,
				.items = std::move(this->current_frame)
			};
			this->current_frame = {};
			this->frame_stack.push(std::move(new_frame));
		};
		inline void pop_frame()
		{
			if (not this->current_frame.empty())
			{
				throw errors::DeserializeIncompleteDataError();
			};
			this->current_frame = std::move(this->frame_stack.top().items);
			this->frame_stack.pop();
		};
		template <typename target>
			requires std::is_base_of_v<element::Element, target>
		inline std::shared_ptr<target> get_top_container() const noexcept
		{
			return std::dynamic_pointer_cast<target>(this->frame_stack.top().items.back()); // The element pushed at last
		};
		template <typename memo_id_type>
			requires std::is_integral_v<memo_id_type>
		inline void generate_memo()
		{
			int memo_id = this->pop_instruction_as<memo_id_type>();
			if (this->memos.contains(memo_id))
			{
				throw errors::DeserializeMemoDuplicateError();
			};
			this->memos.insert({ memo_id, this->get_latest_element() });
		};
		template <typename memo_id_type>
			requires std::is_integral_v<memo_id_type>
		inline void ref_memo()
		{
			int memo_id = this->pop_instruction_as<memo_id_type>();
			if (not this->memos.contains(memo_id))
			{
				throw errors::DeserializeMemoInvalidError();
			};
			this->current_frame.emplace(this->memos[memo_id]);
		};
		void deserialize_string(choices::serialization::op_code current_op_code)
		{
			using choices::serialization::op_code;
			std::string result;
			while (true)
			{
				switch (current_op_code)
				{
					case op_code::string_255:
						goto end_string;
					case op_code::string_extend:
						result.append_range(this->pop_instruction_length(max_string_size));
						break;
					default:
						throw errors::DeserializeInvalidOpCodeError();
				};
				current_op_code = this->pop_instruction_as<op_code>();
			}; // Never stops until "goto" here.
			end_string:
			// The op code must be "string_255" and has been popped previously.
			result.append_range(this->pop_instruction_length(this->pop_instruction_as<std::uint8_t>()));
			this->current_frame.push(std::make_shared<element::GameDataString>(result));
		};
		std::uint32_t get_repeat_parameter_from_instruction()
		{
			using choices::serialization::op_code;
			switch (this->pop_instruction_as<op_code>())
			{
				case op_code::uint_1:
					return this->pop_instruction_as<std::uint8_t>();
				case op_code::uint_4:
					return this->pop_instruction_as<std::uint32_t>();
				default:
					throw errors::DeserializeInvalidOpCodeError();
			};
		};
	public:
		DeserializeContext() = delete;
		DeserializeContext(const DeserializeContext&) = delete;
		DeserializeContext(DeserializeContext&&) = delete;
		DeserializeContext(std::span<byte> instructions) :
			instructions{ instructions }
		{
			if (not std::in_range<std::uint32_t>(instructions.size()))
			{
				throw errors::DeserializeTooLongError();
			};
			repeat_frame default_frame
			{
				.begin = 0,
				.index = 0,
				.end = static_cast<std::uint32_t>(instructions.size()),
				.repeat_times_left = 0
			};
			this->repeat_stack.push(std::move(default_frame));
		};
		element::element_ptr deserialize()
		{
			using choices::serialization::op_code;
			// Detect the head of the serial.
			switch (this->pop_instruction_as<op_code>())
			{
				case op_code::version_2:
					if (this->pop_instruction_as<std::uint16_t>() == constants::serialization::version)
					{
						break; // Version matches then continue.
					};
					[[fallthrough]];
				case op_code::version_256:
					throw errors::DeserializeVersionError();
				default:
					throw errors::DeserializeInvalidOpCodeError();
			};
			while (true)
			{
				// Parse each op code.
				switch (const op_code current_op_code = this->pop_instruction_as<op_code>())
				{
					case op_code::int_fast:
						this->current_frame.push(std::make_shared<element::GameDataFastInt>(this->pop_instruction_as<std::int32_t>()));
						break;
					case op_code::double_8:
						this->current_frame.push(std::make_shared<element::GameDataFloat>(this->pop_instruction_as<double>()));
						break;
					case op_code::bool_true:
						this->current_frame.push(std::make_shared<element::GameDataBool>(true));
						break;
					case op_code::bool_false:
						this->current_frame.push(std::make_shared<element::GameDataBool>(false));
						break;
					case op_code::null:
						this->current_frame.push(std::make_shared<element::GameDataNull>());
						break;
					case op_code::string_255:
					case op_code::string_extend:
						this->deserialize_string(current_op_code);
						break;
					case op_code::stop:
						goto stop;
					case op_code::repeat:
					{
						std::uint32_t begin_index = get_repeat_parameter_from_instruction();
						std::uint32_t length = get_repeat_parameter_from_instruction();
						std::uint32_t repeat_times = get_repeat_parameter_from_instruction();
						std::uint32_t end_index = begin_index + length;
						if (this->instructions.size() > end_index and end_index >= this->repeat_stack.top().index)
						{
							throw errors::DeserializeRepeatEOFError();
						};
						repeat_frame new_repeat_frame
						{
							.begin = begin_index,
							.index = begin_index,
							.end = end_index,
							.repeat_times_left = repeat_times - 1 // The first repeat time is built in by "index".
						};
						this->repeat_stack.push(std::move(new_repeat_frame));
						break;
					};
					case op_code::build:
						this->pop_frame();
						break;
					case op_code::memoize_unused:
						break; // Does nothing.
					case op_code::memoize_used_1:
						this->generate_memo<std::uint8_t>();
						break;
					case op_code::memoize_used_2:
						this->generate_memo<std::uint16_t>();
						break;
					case op_code::memoize_used_4:
						this->generate_memo<std::uint32_t>();
						break;
					case op_code::ref_1:
						this->ref_memo<std::uint8_t>();
						break;
					case op_code::ref_2:
						this->ref_memo<std::uint16_t>();
						break;
					case op_code::ref_4:
						this->ref_memo<std::uint32_t>();
						break;
					case op_code::list:
						this->current_frame.push(std::make_shared<element::GameDataList>());
						this->push_frame<choices::serialization::deserialize_frame_type::list>();
						break;
					case op_code::set:
						this->current_frame.push(std::make_shared<element::GameDataSet>());
						this->push_frame<choices::serialization::deserialize_frame_type::set>();
						break;
					case op_code::dict:
						this->current_frame.push(std::make_shared<element::GameDataDict>());
						this->push_frame<choices::serialization::deserialize_frame_type::dict>();
						break;
					default:
						throw errors::DeserializeInvalidOpCodeError();
				};
				// Choose the additional work based on the top stack type.
				if (this->frame_stack.size() != 0) // Currently not in the bottom layer.
				{
					switch (this->frame_stack.top().type)
					{
						case choices::serialization::deserialize_frame_type::list:
							while (this->current_frame.size() >= 1)
							{
								this->get_top_container<element::GameDataList>()->append(this->pop_element());
							};
							break;
						case choices::serialization::deserialize_frame_type::set:
							while (this->current_frame.size() >= 1)
							{
								this->get_top_container<element::GameDataSet>()->append(this->pop_element());
							};
							break;
						case choices::serialization::deserialize_frame_type::dict:
							while (this->current_frame.size() >= 2)
							{
								element::element_ptr key = this->pop_element();
								element::element_ptr value = this->pop_element();
								this->get_top_container<element::GameDataDict>()->append(key, value);
							};
							break;
						default:
							break;
					};
				};
			}; // Never stops until "goto" here.
			stop:
			if (not this->frame_stack.empty())
			{
				throw errors::DeserializeEOFError();
			};
			if (this->current_frame.empty())
			{
				throw errors::DeserializeNoResultError();
			}
			else if (this->current_frame.size() >= 2)
			{
				throw errors::DeserializeMultipleResultError();
			};
			if (this->repeat_stack.size() != 1 or this->repeat_stack.top().index != this->repeat_stack.top().end)
			{
				throw errors::DeserializeStopEarlyError();
			};
			return this->current_frame.front();
		};
};

namespace element
{
	void GameDataNull::serialize(SerializeContext& context) const
	{
		if (context.search_memo(this))
		{
			return;
		};
		context.add_instruction({ choices::serialization::op_code::null });
		context.add_memo(this);
	};
	void GameDataBool::serialize(SerializeContext& context) const
	{
		using choices::serialization::op_code;
		if (context.search_memo(this))
		{
			return;
		};
		if (this->value)
		{
			context.add_instruction({ op_code::bool_true });
		}
		else
		{
			context.add_instruction({ op_code::bool_false });
		};
		context.add_memo(this);
	};
	void GameDataFastInt::serialize(SerializeContext& context) const
	{
		using choices::serialization::op_code;
		if (context.search_memo(this))
		{
			return;
		};
		context.add_instruction_with_number(op_code::int_fast, this->value);
		context.add_memo(this);
	};
	void GameDataFloat::serialize(SerializeContext& context) const
	{
		using choices::serialization::op_code;
		if (context.search_memo(this))
		{
			return;
		};
		context.add_instruction_with_number(op_code::double_8, this->value);
		context.add_memo(this);
	};
	void GameDataString::serialize(SerializeContext& context) const
	{
		using choices::serialization::op_code;
		if (context.search_memo(this))
		{
			return;
		};
		std::string_view remaining_string{ this->value };
		serialization::serial instruction;
		while (not std::in_range<std::uint8_t>(remaining_string.size()))
		{
			instruction.reserve(sizeof op_code + max_string_size);
			instruction.emplace_back(op_code::string_extend);
			instruction.insert(instruction.end(), remaining_string.begin(), remaining_string.begin() + max_string_size);
			context.add_instruction(std::move(instruction));
			remaining_string.remove_prefix(max_string_size);
		};
		instruction.reserve(sizeof op_code + sizeof std::uint8_t + remaining_string.size());
		instruction.emplace_back(op_code::string_255);
		instruction.emplace_back(static_cast<std::uint8_t>(remaining_string.size()));
		instruction.insert_range(instruction.end(), remaining_string);
		context.add_instruction(std::move(instruction));
		context.add_memo(this);
	};
	void GameDataList::serialize(SerializeContext& context) const
	{
		using choices::serialization::op_code;
		if (context.search_memo(this))
		{
			return;
		};
		context.add_instruction({ op_code::list });
		context.add_memo(this);
		for (const element_ptr& item : this->items)
		{
			item->serialize(context);
		};
		context.add_instruction({ op_code::build });
	};
	void GameDataSet::serialize(SerializeContext& context) const
	{
		using choices::serialization::op_code;
		if (context.search_memo(this))
		{
			return;
		};
		context.add_memo(this);
		for (const element_ptr& item : this->items)
		{
			item->serialize(context);
		};
		context.add_instruction({ op_code::build });
	}
	void GameDataDict::serialize(SerializeContext& context) const
	{
		using choices::serialization::op_code;
		if (context.search_memo(this))
		{
			return;
		};
		context.add_instruction({ op_code::dict });
		context.add_memo(this);
		for (const auto& item : this->items)
		{
			item.first->serialize(context);
			item.second->serialize(context);
		};
		context.add_instruction({ op_code::build });
	};
};

namespace serialization
{
	serial serialize(const element::element_ptr& object)
	{
		SerializeContext context;
		object->serialize(context);
		return context.arrange_output();
	};
	element::element_ptr deserialize(std::span<byte> serial)
	{
		DeserializeContext context(serial);
		return context.deserialize();
	};
};