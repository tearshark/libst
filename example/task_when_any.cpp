#include <iostream>
#include <string>
#include <list>
#include <chrono>

#include "libtask.h"

using namespace std::literals;

static auto create_task_foo(int val)
{
	return st::make_task(st::async_context, [=]
	{
		std::this_thread::sleep_for(1ms * (rand() % 100));

		std::cout << "task foo, val = " << val << std::endl;
		return val;
	});
}

void test_task_when_any_1()
{
	srand((int)time(nullptr));

	using namespace st;

	std::list<decltype(create_task_foo(0))> v;
	for (int i = 1; i < 10; ++i)
		v.emplace_back(create_task_foo(i));

	auto tall = when_any(v.begin(), v.end())
		.then([](size_t idx, int val)
		{
			std::cout << "task(" << idx << ") completed. value is " << val << std::endl;
			return val * idx;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();
	std::cout << "end value is " << val << std::endl;
}

void test_task_when_any_2()
{
	srand((int)time(nullptr));

	using namespace st;

	auto t1 = create_task_foo(1);
	auto t2 = create_task_foo(2);
	auto t3 = create_task_foo(3);

	auto tall = when_any(t1, t2, t3)
		.then([](size_t idx, int val)
		{
			std::cout << "task(" << idx << ") completed. value is " << val << std::endl;
			return val * idx;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();
	std::cout << "end value is " << val << std::endl;
}

void test_task_when_any()
{
	//test_task_when_any_1();
	test_task_when_any_2();
}
