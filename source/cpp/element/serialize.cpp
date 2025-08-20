module core.element;

import std;
import core.constant;

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
};