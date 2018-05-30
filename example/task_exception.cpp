//测试任务链将异常传递给future，在future::get()时获得任务链执行过程中的异常
#include "task.h"
#include "task_context.h"
#include "threadpool_context.h"
#include "log_print.h"

using namespace std::literals;

namespace std
{
	class divide_0_exception : public exception
	{
	public:
		virtual const char* what() const _NOEXCEPT
		{
			return "divide by zero";
		}
	};
}

void test_task_exception()
{
	using namespace st;

	auto t = make_task([](int val)
	{
		log_print("delay run ", std::this_thread::get_id());
		if (val == 0) throw std::divide_0_exception();
		return 10 / val;
	}).then(async_context, [](int val)
	{
		std::this_thread::sleep_for(1s);
		log_print("run in another thread ", std::this_thread::get_id());
		if (val == 0) throw std::divide_0_exception();
		return 10 / val;
	});

	t(0);		//5 : 正常; 20 : then()后的代码出错; 0 : make_task()后的代码出错

	auto f = t.get_future();
	try
	{
		log_print("end value is ", f.get());
	}
	catch (std::exception ex)
	{
		log_print(ex.what());
	}
	catch (...)
	{
		log_print("had some exception!");
	}
}
