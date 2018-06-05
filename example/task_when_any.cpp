#include <list>
#include <chrono>

#include "libtask.h"
#include "log_print.h"

using namespace std::literals;

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
			log_print("task(", idx, ") completed. value is ", val);
			return val * idx;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();
	log_print("end value is ", val);
}

void test_task_when_any_2()
{
	srand((int)time(nullptr));

	using namespace st;

	auto t1 = create_task_foo(1);
	auto t2 = create_task_foo(2);
	auto t3 = create_task_foo(3);
	static_assert(std::is_same<decltype(t1), decltype(t2)>::value, "");
	static_assert(std::is_same<decltype(t1), decltype(t3)>::value, "");

	auto tall = when_any(t1, t2, t3)
		.then([](size_t idx, int val)
		{
			log_print("task(", idx, ") completed. value is ", val);
			return val * idx;
		})
		;

	tall();

	auto f = tall.get_future();
	auto val = f.get();
	log_print("end value is ", val);
}

void test_task_when_any_3()
{
	srand((int)time(nullptr));

	using namespace st;

	task<task_node<int>, task_node<void>> t1 = create_task_foo(1);
	task<task_node<float>, task_node<void>> t2 = create_task_foo(2.0f);
	task<task_node<std::string>, task_node<void>> t3 = create_task_foo("abc"s);
	static_assert(!std::is_same<decltype(t1), decltype(t2)>::value, "");
	static_assert(!std::is_same<decltype(t1), decltype(t3)>::value, "");
	static_assert(!std::is_same<decltype(t2), decltype(t3)>::value, "");

	auto tall = when_any(t1, t2, t3)
		.then([](size_t idx, std::any val)
		{
			if (idx == 0)
				log_print("task(", idx, ") completed. value is ", std::any_cast<int>(val));
			if (idx == 1)
				log_print("task(", idx, ") completed. value is ", std::any_cast<float>(val));
			if (idx == 2)
				log_print("task(", idx, ") completed. value is ", std::any_cast<std::string>(val));

			return idx;
		})
		;

/*
	tall();

	auto f = tall.get_future();
	auto val = f.get();
	log_print("end index is ", val);
*/
}

void test_task_when_any()
{
	using namespace st;
	
	test_task_when_any_1();
	test_task_when_any_2();
#if LIBTASK_DEBUG_MEMORY
	for (int i = 0; i < 10; ++i)
	{
		test_task_when_any_3();
		std::this_thread::sleep_for(100ms);
		assert(g_node_counter.load() == 0);
		assert(g_task_counter.load() == 0);
	}
#else
	test_task_when_any_3();
#endif
}
