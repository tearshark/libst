//测试get_future() 和 then() 之间的冲突导致运行时的异常
#include "task.h"
#include "task_context.h"
#include "log_print.h"

using namespace std::literals;

void test_task_conflict()
{
	using namespace st;

	auto t = make_task([]
	{
		log_print("delay run ", std::this_thread::get_id());
		return 1;
	});

	auto f = t.get_future();		//随后不能再then()了。

	//继续then(),导致异常
	t.then(async_context, [](int val)
	{
		std::this_thread::sleep_for(5s);
		log_print("run in another thread ", std::this_thread::get_id());
		return val;
	});

	t();

#if 1
	log_print("end value is ", f.get());
#else
	//也可以不取future，这样等待任务自然链自然结束
	log_print("press any key to continue.");
	_getch();
#endif
}
