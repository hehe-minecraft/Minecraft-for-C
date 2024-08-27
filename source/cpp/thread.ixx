export module core.thread;

import std;
import core.constant;

export namespace thread
{
	using task = std::function<void()>;
	class TaskQueue
	{
		friend class Worker;
		protected:
			std::queue<task> queue;
			std::condition_variable alarm;
			std::mutex mutex;
		public:
			unsigned int max_tasks = constants::thread::max_tasks;
			TaskQueue() = default;
			TaskQueue(const TaskQueue&) = delete;
			TaskQueue(TaskQueue&&) = delete;
			inline bool empty() const noexcept
			{
				return this->queue.empty();
			};
			void add(const task& task)
			{
				std::unique_lock<std::mutex> lock{ this->mutex };
				if (this->queue.size() >= this->max_tasks)
				{
					throw errors::ThreadTooManyTasksError();
				};
				this->queue.push(task);
				this->alarm.notify_one();
			};
			std::optional<task> try_get() noexcept
			{
				std::unique_lock<std::mutex> lock{ this->mutex };
				if (this->queue.empty())
				{
					return std::nullopt;
				}
				else
				{
					task task = std::move(this->queue.front());
					this->queue.pop();
					return std::move(task);
				};
			};
			task get() noexcept
			{
				std::unique_lock<std::mutex> lock{ this->mutex };
				while (this->queue.empty())
				{
					this->alarm.wait(lock);
				};
				task task = std::move(this->queue.front());
				this->queue.pop();
				return std::move(task);
			};
	};
	class Thread
	{
		protected:
			std::thread thread;
			choices::thread::status status;
			virtual void run() = 0;
		public:
			std::string name;
			Thread() noexcept :
				status{ choices::thread::status::not_started },
				name{ constants::thread::default_name }
			{};
			Thread(const Thread&) = delete;
			Thread(Thread&& source) noexcept :
				thread{ std::move(source.thread) },
				status{ source.status },
				name{ source.name }
			{};
			virtual ~Thread() noexcept
			{
				this->join();
			};
			virtual void start() = 0;
			virtual inline void join() noexcept
			{
				if (this->thread.joinable())
				{
					this->thread.join();
					this->status = choices::thread::status::terminated;
				};
			};
			virtual inline void detach() noexcept
			{
				if (this->thread.joinable())
				{
					this->thread.detach();
				};
			};
			inline choices::thread::status get_status() const noexcept
			{
				return this->status;
			};
	};
	class Worker : public Thread
	{
		protected:
			TaskQueue& queue;
			bool stopping;
			void run()
			{
				this->status = choices::thread::status::idle;
				task task;
				while (not this->stopping)
				{
					std::unique_lock<std::mutex> lock{ this->queue.mutex };
					while (this->queue.empty() and not this->stopping)
					{
						this->status = choices::thread::status::sleeping;
						this->queue.alarm.wait(lock);
					};
					if (this->stopping)
					{
						break;
					};
					this->status = choices::thread::status::running;
					task = this->queue.queue.front();
					this->queue.queue.pop();
					lock.unlock();
					task();
				};
				this->status = choices::thread::status::terminated;
			};
		public:
			Worker() = delete;
			Worker(const Worker&) = delete;
			Worker(Worker&& source) noexcept :
				Thread{ std::move(source) },
				queue{ source.queue },
				stopping{ source.stopping }
			{};
			explicit Worker(TaskQueue& queue) noexcept :
				Thread{},
				queue{ queue },
				stopping{ false }
			{};
			~Worker() noexcept
			{
				this->join();
			};
			void start()
			{
				if (this->status != choices::thread::status::not_started)
				{
					throw errors::ThreadStartedError();
				};
				this->thread = std::thread{ std::bind(&Worker::run, this) };
			};
			inline void stop_after_current_work() noexcept
			{
				this->stopping = true;
				this->queue.alarm.notify_all();
			};
			inline void join() noexcept
			{
				this->stop_after_current_work();
				Thread::join();
			};
	};
	class Distributor : public Thread
	{
		using duration = std::chrono::milliseconds;
		using time_point = std::chrono::steady_clock::time_point;
		struct DistributingTask
		{
			time_point next_running_time;
			choices::thread::distribute_task_type type;
			task function;
			duration interval;
			std::reference_wrapper<TaskQueue> queue;
			inline bool operator<(const DistributingTask& other) const noexcept
			{
				return this->next_running_time > other.next_running_time; // Earliest first
			};
		};
		protected:
			std::priority_queue<DistributingTask> task_queue;
			std::condition_variable alarm;
			std::mutex mutex;
			bool stopping;
			void run()
			{
				this->status = choices::thread::status::idle;
				while (true)
				{
					std::unique_lock<std::mutex> lock{ this->mutex };
					while (this->task_queue.empty() and not this->stopping)
					{
						this->status = choices::thread::status::idle;
						this->alarm.wait(lock);
					};
					if (this->stopping)
					{
						break;
					};
					this->status = choices::thread::status::sleeping;
					this->alarm.wait_until(lock, this->task_queue.top().next_running_time);
					if (this->stopping)
					{
						break;
					};
					this->status = choices::thread::status::running;
					DistributingTask task{ std::move(this->task_queue.top()) };
					this->task_queue.pop();
					if (task.type == choices::thread::distribute_task_type::loop)
					{
						DistributingTask next_task{ task };
						next_task.next_running_time = task.next_running_time + next_task.interval;
						this->task_queue.emplace(std::move(next_task));
					};
					task.queue.get().add(task.function);
				};
				this->status = choices::thread::status::terminated;
			};
			inline void wake() noexcept
			{
				if (this->status == choices::thread::status::idle)
				{
					this->alarm.notify_one();
				};
			};
		public:
			void start()
			{
				if (this->status != choices::thread::status::not_started)
				{
					throw errors::ThreadStartedError();
				};
				this->thread = std::thread{ std::bind(&Distributor::run, this) };
			};
			inline void add_immediate_task(TaskQueue& queue, const task& task) const noexcept
			{
				queue.add(task);
			};
			void add_single_task(TaskQueue& queue, const task& task, const duration& delay) noexcept
			{
				DistributingTask single_task
				{
					.next_running_time = std::chrono::steady_clock::now() + delay,
					.type = choices::thread::distribute_task_type::single,
					.function = task,
					.interval = delay,
					.queue = queue
				};
				std::unique_lock<std::mutex> lock{ this->mutex };
				this->task_queue.emplace(std::move(single_task));
				this->wake();
			};
			inline void add_loop_task(TaskQueue& queue, const task& task, const duration& interval) noexcept
			{
				this->add_loop_task(queue, task, interval, interval);
			};
			void add_loop_task(TaskQueue& queue, const task& task, const duration& interval, const duration& delay) noexcept
			{
				DistributingTask loop_task
				{
					.next_running_time = std::chrono::steady_clock::now() + delay,
					.type = choices::thread::distribute_task_type::loop,
					.function = task,
					.interval = interval,
					.queue = queue
				};
				std::unique_lock<std::mutex> lock{ this->mutex };
				this->task_queue.emplace(std::move(loop_task));
				this->wake();
			};
			inline void stop() noexcept
			{
				this->stopping = true;
				this->alarm.notify_one();
			};
			inline void join() noexcept
			{
				this->stop();
				Thread::join();
			};
	};
	class Group
	{
		protected:
			std::deque<Worker> workers;
			TaskQueue tasks;
		public:
			Group() noexcept = default;
			Group(const Group&) = delete;
			Group(Group&& source)
			{
				source.join();
				std::exchange(this->workers, std::move(source.workers));
			};
			std::size_t inline size() const noexcept
			{
				return this->workers.size();
			};
			void add_worker(const std::string& name = constants::thread::default_name) noexcept
			{
				this->workers.emplace_back(this->tasks);
				this->workers.back().start();
			};
			void inline add_task(const task& task) noexcept
			{
				this->tasks.add(task);
			};
			void join() noexcept
			{
				for (Worker& each_worker : this->workers)
				{
					each_worker.join();
				};
				this->workers.clear();
			};
	};
};