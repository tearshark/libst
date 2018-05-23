#include <iostream>
#include <string>
#include <list>

#include "task.h"
#include "task_when_all.h"

using namespace std::literals;

void test_task_when_all_1()
{
	using namespace st;

	auto t1 = make_task([]()
	{
		std::cout << "first task." << std::endl;
		return std::tuple<int, std::string>{ 1, "abc"s };
	});
	auto t2 = make_task([]
	{
		std::cout << "second task." << std::endl;
	});
	auto t3 = make_task([]
	{
		std::cout << "second task." << std::endl;
		return 2.0;
	});

	auto tall = when_all(t1, t2, t3)
		.then([](int v1, std::string, double v2)
		{
			std::cout << "all completed." << std::endl;
			return v1 + v2;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();

	std::cout << "end value is " << val << std::endl;
}

auto create_task_foo(int val)
{
	return st::make_task([=]
	{
		std::cout << "task foo, val = " << val << std::endl;
		return val;
	});
}

void test_task_when_all_2()
{
	using namespace st;

	std::list<decltype(create_task_foo(0))> v;
/*
	for (int i = 1; i < 10; ++i)
		v.emplace_back(create_task_foo(i));
*/

	auto tall = when_all(v.begin(), v.end())
		.then([](std::vector<int> v)
		{
			if (v.empty())
				std::cout << "none task." << std::endl;
			else
				std::cout << v.size() << " task(s) completed." << std::endl;

			int val = 0;
			for (auto t : v)
				val += t;
			return val;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();
	std::cout << "end value is " << val << std::endl;
}

void test_task_when_all()
{
	test_task_when_all_1();
	test_task_when_all_2();
}
