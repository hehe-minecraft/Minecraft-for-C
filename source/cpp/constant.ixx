export module core.constant;

import std;

export namespace constants
{
	namespace thread
	{
		const std::string default_name = "Worker";
		constexpr unsigned int max_tasks = 1024;
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

export namespace errors
{
	class BasicError : public std::exception {};
	class ThreadError : public BasicError {};
	class ThreadStatusError : public ThreadError {};
	class ThreadStartedError : public ThreadStatusError {};
	class ThreadTooManyTasksError : public ThreadError {};
};