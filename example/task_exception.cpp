//测试任务链将异常传递给future，在future::get()时获得任务链执行过程中的异常

#include <iostream>
#include <string>

#include "task.h"
#include "task_context.h"
#include "threadpool_context.h"

using namespace std::literals;

void test_task_exception()
{
	using namespace st;

	auto t = make_task([](int val)
	{
		std::cout << "delay run " << std::this_thread::get_id() << std::endl;
		if (val == 0) throw std::exception("divide by zero");
		return 10 / val;
	}).then(async_context, [](int val)
	{
		std::this_thread::sleep_for(1s);
		std::cout << "run in another thread " << std::this_thread::get_id() << std::endl;
		if (val == 0) throw std::exception("divide by zero");
		return 10 / val;
	});

	t(0);		//5 : 正常; 20 : then()后的代码出错; 0 : make_task()后的代码出错

	auto f = t.get_future();
	try
	{
		std::cout << "end value is " << f.get() << std::endl;
	}
	catch (std::exception ex)
	{
		std::cout << ex.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "had some exception!" << std::endl;
	}
}
