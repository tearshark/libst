//测试将任务节点分配到指定的线程池环境里运行

#include <iostream>
#include <string>

#include "task.h"
#include "task_context.h"
#include "threadpool_context.h"

using namespace std::literals;

void test_task_async()
{
	using namespace st;

	threadpool_context pool_context{ 1 };

	auto t = make_task([]
	{
		std::cout << "delay run " << std::this_thread::get_id() << std::endl;
		return std::this_thread::get_id();
	}).then(pool_context, [](std::thread::id tid)
	{
		std::this_thread::sleep_for(5s);
		std::cout << "run in another thread " << std::this_thread::get_id() << " " << (tid == std::this_thread::get_id()) << std::endl;
		return 2;
	}).then(imm_context, [](int result)
	{
		std::cout << "run inplace thread " << std::this_thread::get_id() << " " << result << std::endl;
		return result * 2;
	});

	t();

#if 1
	auto f = t.get_future();
	std::cout << "end value is " << f.get() << std::endl;
#else
	std::cout << "press any key to continue." << std::endl;
	_getch();
#endif
}
