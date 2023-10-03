#pragma once
#include <condition_variable>
#include <functional>
#include <thread>
#include <map>
#include <mutex>
#include <queue>
#include <unordered_map>
#include "constants.hpp"

class BasicThread
{
	friend class DistributeThread;
	public:
		std::string name = "Basic";
		BasicThread(const char*);
		choices::state get_state();
		std::unique_lock<std::mutex> * pause();
		void resume(std::unique_lock<std::mutex> &);
		void stop();
		void work(std::function<void ()> &function);
	protected:
		std::condition_variable alarm;
		std::unique_lock<std::mutex> *lock = nullptr;
		std::mutex mutex;
		std::queue<std::function<void ()>> queue;
		bool running = true;
		choices::state state = choices::state_free;
		std::thread *thread = nullptr;
		void run();
		void sleep();
};

class DistributeThread
{
	typedef std::pair<unsigned long long, unsigned int> queue_item;
	struct plan
	{
		unsigned int delay;
		std::function<void ()> function = nullptr;
		BasicThread* thread = nullptr;
	};
	public:
		unsigned long long game_time = 0;
		std::string name = "Distributor";
		BasicThread * create(const char*);
		BasicThread * get(const char*);
		void join();
		void repeat_work(const char* thread_name, unsigned int delay, std::function<void ()> function);
		bool start();
		void stop();
		void work(const char* thread_name, std::function<void ()> function);
	protected:
		unsigned int last_id = 0;
		std::unordered_map<std::string, std::unique_lock<std::mutex>> locks;
		std::map<unsigned int, plan> plans;
		std::priority_queue<queue_item, std::vector<queue_item>, std::greater<queue_item>> queue;
		bool running = true;
		std::unordered_map<std::string, BasicThread*> threads;
		std::thread *thread = nullptr;
		void run();
};