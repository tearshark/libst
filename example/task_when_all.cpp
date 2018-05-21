#include "stdafx.h"
#include "task_when_all.h"

void test_task_when_all()
{
	using namespace st;

	auto t1 = make_task([]()
	{
		std::cout << "first task." << std::endl;
		return 1;
	});
	auto t2 = make_task([]
	{
		std::cout << "second task." << std::endl;
		return 2.0;
	});

	auto tall = when_all(std::move(t1), std::move(t2));
	tall();

	auto f = tall.get_future();
	f.get();
}
