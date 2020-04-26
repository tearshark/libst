#pragma once

#include <stack>
#include <vector>
#include "task_semaphore.h"

struct threadpool_context
{
	std::vector<std::thread> th_pool;
	std::stack<st::executor_sptr> tasks;
	std::mutex mtx_task;
	std::semaphore cv_task;

	threadpool_context(size_t count = 0)
	{
		if (count == 0)
			count = std::thread::hardware_concurrency();
		for (size_t i = 0; i < count; ++i)
			th_pool.emplace_back(&threadpool_context::loop_task, this);
	}
	~threadpool_context()
	{
		{
			std::unique_lock<std::mutex> lock__(mtx_task);
			while (!tasks.empty())
				tasks.pop();
		}
		cv_task.release((int)th_pool.size());

		for (auto & th : th_pool)
			th.join();
	}

	void add(const st::executor_sptr & runner)
	{
		assert(runner != nullptr);
		{
			std::unique_lock<std::mutex> lock__(mtx_task);
			tasks.push(runner);
		}
		cv_task.release();
	}

private:
	void loop_task()
	{
		st::executor_sptr runner;
		for (;;)
		{
			cv_task.acquire();
			{
				std::unique_lock<std::mutex> lock__(mtx_task);
				if (tasks.empty())
					break;

				runner = tasks.top();
				tasks.pop();
			}
			runner->run_once();
		}
	}
};
