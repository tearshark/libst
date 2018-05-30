//测试将任务节点分配到指定的线程池环境里运行
#include "task.h"
#include "task_context.h"
#include "threadpool_context.h"
#include "log_print.h"

using namespace std::literals;

void test_task_async()
{
	using namespace st;

	threadpool_context pool_context{ 1 };

	auto t = make_task([]
	{
		log_print("delay run ", std::this_thread::get_id());
		return std::this_thread::get_id();
	}).then(pool_context, [](std::thread::id tid)
	{
		std::this_thread::sleep_for(5s);
		log_print("run in another thread ", std::this_thread::get_id(), " ", tid == std::this_thread::get_id());
		return 2;
	}).then(imm_context, [](int result)
	{
		log_print("run inplace thread ", std::this_thread::get_id(), " ", result);
		return result * 2;
	});

	t();

#if 1
	auto f = t.get_future();
	log_print("end value is ", f.get());
#else
	log_print("press any key to continue.");
	_getch();
#endif
}
