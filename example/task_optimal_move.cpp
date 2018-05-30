//测试任务链内部数据在传递过程中，尽量采用了移动语义(move)，而不是拷贝语义
#include "task.h"
#include "task_context.h"
#include "log_print.h"

using namespace std::literals;

struct print_move
{
	int value;

	print_move() :value(0)
	{
		log_print(this, " default");
	}
	print_move(int val) :value(val)
	{
		log_print(this, " init ", val);
	}
	print_move(const print_move & _Right) :value(_Right.value)
	{
		log_print(this, " copy from ", &_Right);
	}
	print_move & operator = (const print_move & _Right)
	{
		if (this != &_Right)
		{
			value = _Right.value;
			log_print(this, " copy assign ", &_Right);
		}
		return *this;
	}

	print_move(print_move && _Right) :value(_Right.value)
	{
		_Right.value = 0;
		log_print(this, " move from ", &_Right);
	}
	print_move & operator = (print_move && _Right)
	{
		if (this != &_Right)
		{
			value = _Right.value;
			_Right.value = 0;
			log_print(this, " move assign ", &_Right);
		}
		return *this;
	}
};

void test_task_optimal_move()
{
	using namespace st;

	auto t = make_task([]
	{
		log_print("delay run ", std::this_thread::get_id());
		return print_move{ 1 };
	}).then(async_context, [](print_move val)
	{
		std::this_thread::sleep_for(5s);
		log_print("run in another thread ", std::this_thread::get_id());
		return val;
	});

	t();

#if 1
	auto f = t.get_future();
	log_print("end value is ", f.get());
#else
	//也可以不取future，这样等待任务自然链自然结束
	log_print("press any key to continue.");
	_getch();
#endif
}
