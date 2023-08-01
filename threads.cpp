#include <unistd.h>
#include "threads.h"

BasicThread::BasicThread(const char* name)
{
	this->name.assign(name);
	this->state = choices::state_free;
	this->thread = new std::thread(&BasicThread::run, this);
};

choices::state BasicThread::get_state()
{
	return this->state;
};

[[nodiscard]]
std::unique_lock<std::mutex> * BasicThread::pause()
{
	this->state = choices::state_paused;
	std::unique_lock<std::mutex> *lock = new std::unique_lock<std::mutex>(this->mutex);
	return lock;
};

void BasicThread::resume(std::unique_lock<std::mutex> &lock)
{
	lock.unlock();
	this->alarm.notify_all();
};

void BasicThread::run()
{
	this->lock = new std::unique_lock<std::mutex>(this->mutex);
	this->lock->unlock();
	std::function<void ()> function;
	while (this->running)
	{
		this->state = choices::state_running;
		function = this->queue.front();
		function();
		this->queue.pop();
		if (this->queue.empty())
		{
			this->sleep();
		};
	};
};

void BasicThread::sleep()
{
	this->state = choices::state_free;
	this->alarm.wait(*this->lock);
};

void BasicThread::stop()
{
	this->running = false;
	this->alarm.notify_all();
	this->thread->join();
	this->state = choices::state_stopped;
};

void BasicThread::work(std::function<void ()> function)
{
	this->queue.push(function);
};

BasicThread * DistributeThread::create(const char* name)
{
	std::string name_string = name;
	if (this->threads.find(name_string) != this->threads.end())
	{
		throw errors::ThreadExistError();
	};
	BasicThread *new_thread = new BasicThread(name);
	this->threads[name_string] = new_thread;
	this->locks[name_string] = std::unique_lock<std::mutex>(new_thread->mutex);
	return new_thread;
};

bool DistributeThread::start()
{
	if (this->plans.empty())
	{
		return false;
	};
	this->thread = new std::thread(DistributeThread::run, this);
	return true;
};

BasicThread * DistributeThread::get(const char *thread_name)
{
	std::string thread_string = thread_name;
	if (this->threads.find(thread_string) == this->threads.end())
	{
		return this->create(thread_name);
	}
	else
	{
		return this->threads[thread_string];
	};
};

void DistributeThread::repeat_work(const char* thread_name, unsigned int delay, std::function<void ()> function)
{
	plan new_plan;
	new_plan.delay = delay;
	new_plan.function = function;
	new_plan.thread = this->get(thread_name);
	this->plans[last_id] = new_plan;
	this->queue.push(queue_item(delay, last_id));
	last_id++;
};

void DistributeThread::run()
{
	queue_item function;
	plan function_plan;
	while (this->running)
	{
		function = this->queue.top();
		if (function.first > this->game_time)
		{
			sleep((function.first - this->game_time) / 1000);  // Change milliseconds into seconds
			this->game_time = function.first;
			continue;
		};
		this->queue.pop();
		function_plan = this->plans[function.second];
		function_plan.thread->work(function_plan.function);
		this->queue.push(queue_item(function.first + function_plan.delay, function.second));
	};
};

void DistributeThread::stop()
{
	this->running = false;
	for (auto iterator : this->threads)
	{
		iterator.second->stop();
	};
	this->thread->join();
};

void DistributeThread::work(const char* thread_name, std::function<void ()> function)
{
	this->get(thread_name)->work(function);
};
