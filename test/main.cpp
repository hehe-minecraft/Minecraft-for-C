import std;
import core.thread;

namespace test
{
	const std::string base_test_name = "main";
	class TestPack final
	{
		private:
			static std::string combine_labels(const std::vector<std::string>& label_stack)
			{
				if (label_stack.empty())
				{
					return "";
				};
				std::string result = label_stack[0];
				for (std::vector<std::string>::const_iterator label_index = label_stack.begin() + 1; label_index < label_stack.end(); label_index++)
				{
					result += " > ";
					result += *label_index;
				};
				return result;
			}
			void test_all(std::vector<std::string>& label_stack)
			{
				std::string label = combine_labels(label_stack);
				std::cout << "[  GROUP  ]\t" << label << std::endl;
				this->successes = 0;
				this->failures = 0;
				for (auto& [each_function_name, each_function] : this->functions)
				{
					label_stack.push_back(each_function_name);
					std::string inner_label = combine_labels(label_stack);
					std::cout << "[ RUNNING ]\t" << inner_label << '\r';
					if (each_function())
					{
						std::cout << "[ SUCCESS ]\t" << inner_label << std::endl;
						this->successes++;
					}
					else
					{
						std::cout << "[ FAILURE ]\t" << inner_label << std::endl;
						this->failures++;
					};
					label_stack.pop_back();
				};
				for (auto& [each_subpack_name, each_subpack] : this->subpacks)
				{
					label_stack.push_back(each_subpack_name);
					each_subpack.test_all(label_stack);
					this->successes += each_subpack.successes;
					this->failures += each_subpack.failures;
					label_stack.pop_back();
				};
				std::cout << std::format("[ {:>3}/{:<3} ]\t", this->failures, this->successes + this->failures) << label << std::endl;
			};
		public:
			int successes = 0;
			int failures = 0;
			std::map<std::string, TestPack> subpacks;
			std::map<std::string, std::function<bool()>> functions;
			TestPack() = default;
			TestPack(const TestPack&) = delete;
			TestPack(TestPack&&) = delete;
			TestPack& operator[](std::string name)
			{
				return this->subpacks[name];
			};
			inline void test_all()
			{
				std::vector<std::string> test_stack{ base_test_name };
				this->test_all(test_stack);
			};
	};
	namespace assert
	{
		template <typename content>
			requires std::equality_comparable<content>
		bool starts_with(const std::vector<content>& target, const std::vector<content>& pattern)
		{
			if (target.size() < pattern.size())
			{
				return false;
			};
			for (char i = 1; i < pattern.size(); i++)
			{
				if (target.at(i) != pattern.at(i))
				{
					return false;
				};
			};
			return true;
		};
	};
	namespace thread_test
	{
		bool worker()
		{
			thread::TaskQueue queue;
			thread::Worker worker{ queue };
			bool finished = false;
			queue.add([&finished] {finished = true; });
			worker.start();
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Wait for the worker to start working.
			worker.join();
			return finished;
		};
		bool distributor_static()
		{
			thread::TaskQueue queue;
			thread::Worker worker{ queue };
			thread::Distributor distributor;
			std::vector<char> output;
			distributor.add_loop_task(queue, [&output] {output.push_back(1); }, std::chrono::milliseconds(20)); // task 1
			distributor.add_loop_task(queue, [&output] {output.push_back(2); }, std::chrono::milliseconds(30), std::chrono::milliseconds(10)); // task 2
			distributor.add_single_task(queue, [&output] {output.push_back(3); }, std::chrono::milliseconds(15)); // task 3
			worker.start();
			distributor.start();
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Multithreading is uncontrollable, so let it work a little longer.
			distributor.join();
			worker.join();
			// It should work as follows:
			// 10ms 2 15ms 3 20ms 1 40ms 1 2 60ms 1 70ms 2
			return assert::starts_with(output, { 2, 3, 1, 1, 2, 1, 2 });
		};
		bool distributor_dynamic()
		{
			thread::TaskQueue queue;
			thread::Worker worker{ queue };
			thread::Distributor distributor;
			std::vector<char> output;
			distributor.add_loop_task(queue, [&output] {output.push_back(1); }, std::chrono::milliseconds(30), std::chrono::milliseconds(20)); // task 1
			distributor.add_single_task(queue, [&output] {output.push_back(2); }, std::chrono::milliseconds(20)); // task 2
			worker.start();
			distributor.start();
			std::this_thread::sleep_for(std::chrono::milliseconds(90));
			distributor.add_single_task(queue, [&output] {output.push_back(3); }, std::chrono::milliseconds(10)); // task 3
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			distributor.add_single_task(queue, [&output] {output.push_back(4); }, std::chrono::milliseconds(25)); // task 4
			std::this_thread::sleep_for(std::chrono::milliseconds(75)); // Multithreading is uncontrollable, so let it work a little longer.
			distributor.join();
			worker.join();
			// It should work as follows:
			// 20ms 1 40ms 2 50ms 1 80ms 1 100ms 3 110ms 1 120ms 4 140ms 1
			return assert::starts_with(output, { 1, 2, 1, 1, 3, 1, 4, 1 });
		};
		bool distributor_late_start()
		{
			thread::TaskQueue queue;
			thread::Worker worker{ queue };
			thread::Distributor distributor;
			char finished_count = 0;
			distributor.add_loop_task(queue, [&finished_count] {finished_count++; }, std::chrono::milliseconds(10)); // task 1
			worker.start();
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			distributor.start();
			std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Wait for the distributor to finish its task.
			distributor.join();
			worker.join();
			// It should distribute 5 tasks at a time.
			return 5 <= finished_count and finished_count <= 6; // Allow some small error caused by systematic thread scheduling.
		};
		bool group()
		{
			thread::Group group;
			std::set<std::thread::id> worker_ids;
			std::function<void()> task = [&worker_ids]
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				worker_ids.insert(std::this_thread::get_id());
			};
			group.add_task(task);
			group.add_task(task);
			group.add_worker();
			group.add_worker();
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Wait for the workers to finish their tasks.
			group.join();
			return worker_ids.size() == 2;
		};
		bool group_move()
		{
			thread::Group group_to_be_moved;
			group_to_be_moved.add_worker();
			group_to_be_moved.add_worker();
			thread::Group group_target{ std::move(group_to_be_moved) };
			return group_to_be_moved.size() == 0 and group_target.size() == 2;
		}
		bool group_distribute()
		{
			thread::Group group;
			thread::Distributor distributor;
			char finished_count = 0;
			group.add_worker();
			distributor.add_single_task(group, [&finished_count] { finished_count++; }, std::chrono::milliseconds(0));
			distributor.add_loop_task(group, [&finished_count] { finished_count++; }, std::chrono::milliseconds(10), std::chrono::milliseconds(0)); // Should run twice.
			distributor.add_immediate_task(group, [&finished_count] { finished_count++; });
			distributor.start();
			std::this_thread::sleep_for(std::chrono::milliseconds(15));
			return finished_count == 4;
		}
		bool max_tasks_0()
		{
			thread::TaskQueue queue;
			queue.max_tasks = 0;
			try
			{
				queue.add([] {});
			}
			catch (errors::ThreadTooManyTasksError)
			{
				return true;
			};
			return false;
		};
		bool tasks_overflow()
		{
			thread::TaskQueue queue;
			queue.max_tasks = 10;
			try
			{
				for ([[maybe_unused]] int _ : std::ranges::views::repeat(0, 10))
				{
					queue.add([] {});
				};
			}
			catch (errors::ThreadTooManyTasksError)
			{
				return false;
			};
			try
			{
				queue.add([] {});
			}
			catch (errors::ThreadTooManyTasksError)
			{
				return true;
			};
			return false;
		};
	};
};

int main()
{
	std::cout << "Note: [   1/3   ] refers to 1 failure occurred in 3 tests." << std::endl << std::endl;
	std::cout << "CONSTRUCTING TESTS" << std::endl;
	test::TestPack test_pack;
	test_pack["thread"].functions["worker"] = test::thread_test::worker;
	test_pack["thread"]["distributor"].functions["static"] = test::thread_test::distributor_static;
	test_pack["thread"]["distributor"].functions["dynamic"] = test::thread_test::distributor_dynamic;
	test_pack["thread"]["distributor"].functions["late_start"] = test::thread_test::distributor_late_start;
	test_pack["thread"].functions["group"] = test::thread_test::group;
	test_pack["thread"].functions["group_move"] = test::thread_test::group_move;
	test_pack["thread"].functions["group_distribute"] = test::thread_test::group_distribute;
	test_pack["thread"].functions["max_tasks=0"] = test::thread_test::max_tasks_0;
	test_pack["thread"].functions["tasks_overflow"] = test::thread_test::tasks_overflow;
	std::cout << "TEST BEGINS" << std::endl;
	test_pack.test_all();
	std::cout << "TEST FINISHES WITH " << test_pack.successes << " SUCCESSES AND " << test_pack.failures << " FAILURES" << std::endl;
};