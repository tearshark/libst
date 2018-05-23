//测试get_future() 和 then() 之间的冲突导致运行时的异常

#include <iostream>
#include <string>

#include "task.h"
#include "task_context.h"

using namespace std::literals;

void test_task_conflict()
{
	using namespace st;

	auto t = make_task([]
	{
		std::cout << "delay run " << std::this_thread::get_id() << std::endl;
		return 1;
	});

	auto f = t.get_future();		//随后不能再then()了。

	//继续then(),导致异常
	t.then(async_context, [](int val)
	{
		std::this_thread::sleep_for(5s);
		std::cout << "run in another thread " << std::this_thread::get_id() << std::endl;
		return val;
	});

	t();

#if 1
	std::cout << "end value is " << f.get() << std::endl;
#else
	//也可以不取future，这样等待任务自然链自然结束
	std::cout << "press any key to continue." << std::endl;
	_getch();
#endif
}
