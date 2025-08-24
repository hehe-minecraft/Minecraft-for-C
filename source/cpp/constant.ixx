export module core.constant;

import std;

export namespace constants
{
	namespace thread
	{
		constexpr std::string_view default_name = "Thread";
		constexpr std::size_t max_tasks = 1024;
	};
	namespace serialization
	{
		constexpr std::uint16_t version = 0;
		constexpr unsigned char average_instruction_length = 3; // Partly overestimated.
	};
};

export namespace choices
{
	namespace thread
	{
		enum struct status : char
		{
			not_started,
			idle,
			running,
			sleeping,
			terminated
		};
		enum struct distribute_task_type : char
		{
			single,
			loop
		};
	};
};
	namespace serialization
	{
		enum op_code : std::uint8_t // Intended to be unscoped for serialize purposes.
		{
			// 1, 2 refers to bytes, not bits.
			int_1 = 0b00000000,
			int_4 = 0b00000001,
			int_extend = 0b00000010,
			int_fast = 0b00000011,
			uint_1 = 0b00000100,
			uint_4 = 0b00000101,
			uint_extend = 0b00000110,
			uint_fast = 0b00000111,
			double_8 = 0b00001000,
			bool_true = 0b00001100,
			bool_false = 0b00001101,
			null = 0b00001110,
			string_255 = 0b00010000,
			string_extend = 0b00010001,
			version_2 = 0b01000000,
			version_256 = 0b01000001,
			stop = 0b01000010,
			repeat = 0b01000011,
			build = 0b01000100,
			memoize_unused = 0b01000111,
			memoize_used_1 = 0b01001000,
			memoize_used_2 = 0b01001001,
			memoize_used_4 = 0b01001010,
			ref_1 = 0b01001100,
			ref_2 = 0b01001101,
			ref_4 = 0b01001110,
			list = 0b10000000,
			set = 0b10000001,
			dict = 0b10000010
		};
		enum class deserialize_frame_type
		{
			repeat,
			list,
			set,
			dict
		};
	};
};

export namespace errors
{
	class BasicError : public std::exception {};
	class ThreadError : public BasicError {};
	class ThreadStatusError : public ThreadError {};
	class ThreadStartedError : public ThreadStatusError {};
	class ThreadTooManyTasksError : public ThreadError {};
	class SerializeError : public BasicError {};
	class SerializeMemoOverflowError : public SerializeError {};
	class DeserializeError : public BasicError {};
	class DeserializeVersionError : public DeserializeError {};
	class DeserializeTooLongError : public DeserializeError {};
	class DeserializeInvalidError : public DeserializeError {};
	class DeserializeInvalidOpCodeError : public DeserializeInvalidError {};
	class DeserializeMemoError : public DeserializeInvalidError {};
	class DeserializeMemoDuplicateError : public DeserializeMemoError {};
	class DeserializeMemoInvalidError : public DeserializeMemoError {};
	class DeserializeIncompleteError : public DeserializeInvalidError {};
	class DeserializeIncompleteDataError : public DeserializeIncompleteError {};
	class DeserializeNoResultError : public DeserializeIncompleteError {};
	class DeserializeEOFError : public DeserializeIncompleteError {};
	class DeserializeRepeatEOFError : public DeserializeEOFError {};
	class DeserializeRedundantError : public DeserializeInvalidError {};
	class DeserializeStopEarlyError : public DeserializeRedundantError {};
	class DeserializeMultipleResultError : public DeserializeRedundantError {};
};