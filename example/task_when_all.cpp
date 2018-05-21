#include "stdafx.h"
#include "task_when_all.h"

void test_task_when_all()
{
	using namespace st;

	auto t1 = make_task([] 
	{
		std::cout << "first task." << std::endl;
		return 1;
	});
	auto t2 = make_task([]
	{
		std::cout << "first task." << std::endl;
		return 2.0;
	});

	when_all(t1, t2);
}
