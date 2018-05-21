#include "stdafx.h"
#include "task_when_all.h"

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
		return 2.0;
	});

	auto tall = when_all(t1, t2)
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

void test_task_when_all()
{
	test_task_when_all_1();
}
