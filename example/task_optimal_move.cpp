//测试任务链内部数据在传递过程中，尽量采用了移动语义(move)，而不是拷贝语义
#include <iostream>
#include <string>

#include "task.h"
#include "task_context.h"

using namespace std::literals;

struct print_move
{
	int value;

	print_move() :value(0)
	{
		printf("0x%p default\n", this);
	}
	print_move(int val) :value(val)
	{
		printf("0x%p init %d\n", this, val);
	}
	print_move(const print_move & _Right) :value(_Right.value)
	{
		printf("0x%p copy from 0x%p\n", this, &_Right);
	}
	print_move & operator = (const print_move & _Right)
	{
		if (this != &_Right)
		{
			value = _Right.value;
			printf("0x%p copy assign 0x%p\n", this, &_Right);
		}
		return *this;
	}

	print_move(print_move && _Right) :value(_Right.value)
	{
		_Right.value = 0;
		printf("0x%p move from 0x%p\n", this, &_Right);
	}
	print_move & operator = (print_move && _Right)
	{
		if (this != &_Right)
		{
			value = _Right.value;
			_Right.value = 0;
			printf("0x%p move assign 0x%p\n", this, &_Right);
		}
		return *this;
	}
};

void test_task_optimal_move()
{
	using namespace st;

	auto t = make_task([]
	{
		std::cout << "delay run " << std::this_thread::get_id() << std::endl;
		return print_move{ 1 };
	}).then(async_context, [](print_move val)
	{
		std::this_thread::sleep_for(5s);
		std::cout << "run in another thread " << std::this_thread::get_id() << std::endl;
		return val;
	});

	t();

#if 1
	auto f = t.get_future();
	std::cout << "end value is " << f.get().value << std::endl;
#else
	//也可以不取future，这样等待任务自然链自然结束
	std::cout << "press any key to continue." << std::endl;
	_getch();
#endif
}
