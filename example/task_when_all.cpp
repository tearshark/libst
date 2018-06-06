#include <list>

#include "libtask.h"
#include "log_print.h"

using namespace std::literals;

//Windows : MSVC OK
//Android : failure
void test_task_when_all_1()
{
	using namespace st;

	auto t1 = make_task([]()
	{
		log_print("first task.");
		return std::tuple<int, std::string>{ 1, "abc"s };
	});
	auto t2 = make_task([]
	{
		log_print("second task.");
	});
	auto t3 = make_task([]
	{
		log_print("third task.");
		return 2.0;
	});

	auto tall = when_all(t1, t2, t3)
		.then([](int v1, std::string, double v2)
		{
			log_print("all completed.");
			return v1 + v2;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();

	log_print("end value is ", val);
}

template<class _Ty>
static auto create_task_foo(const _Ty & val)
{
	return st::make_task(st::async_context, [=]
	{
		std::this_thread::sleep_for(1ms * (rand() % 100));

		log_print("task foo, val = ", val);
		return val;
	});
}

void test_task_when_all_2()
{
	using namespace st;

	std::list<decltype(create_task_foo(0))> v;
	for (int i = 1; i < 10; ++i)
		v.emplace_back(create_task_foo(i));

	auto tall = when_all(v.begin(), v.end())
		.then([](std::vector<int> v)
		{
			if (v.empty())
				log_print("none task.");
			else
				log_print(v.size(), " task(s) completed.");

			int val = 0;
			for (auto t : v)
				val += t;
			return val;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();
	log_print("end value is ", val);
}

void test_task_when_all_break_link()
{
	using namespace st;

	std::list<decltype(create_task_foo(0))> v;
	for (int i = 1; i < 10; ++i)
		v.emplace_back(create_task_foo(i));

	auto tall = when_all(v.begin(), v.end())
		.then([](std::vector<int> v)
		{
			if (v.empty())
				log_print("none task.");
			else
				log_print(v.size(), " task(s) completed.");

			int val = 0;
			for (auto t : v)
				val += t;
			return val;
		})
		;
}

void test_task_when_all()
{
	using namespace st;

	test_task_when_all_1();
#if LIBTASK_DEBUG_MEMORY
	for (int i = 0; i < 100; ++i)
	{
		test_task_when_all_2();
		assert(g_node_counter.load() == 0);
		assert(g_task_counter.load() == 0);

		test_task_when_all_break_link();
		assert(g_node_counter.load() == 0);
		assert(g_task_counter.load() == 0);
	}
#else
	test_task_when_all_2();
#endif
}
