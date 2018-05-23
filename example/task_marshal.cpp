//测试将回调接口的函数，包装成任务链节点
#include <iostream>
#include <string>

#include "task.h"
#include "task_context.h"
#include "threadpool_context.h"

using namespace std::literals;

//foo_callback 需要满足以下要求：
//一、返回值没有意义
//二、cb必然会被调用，且只调用一次。除非内部抛了异常
//三、cb没有返回值
void foo_callback(int val, const std::function<void(int, std::string)> & cb, std::string str)
{
	std::cout << "foo_callback: " << val << " @" << std::this_thread::get_id() << std::endl;
	cb(val * 2, str);
}

void test_task_marshal()
{
	using namespace st;

	threadpool_context pool_context{ 1 };

	auto t = marshal_task(&foo_callback, 1, st::_cb, "first run "s)
		.marshal(pool_context, &foo_callback, std::placeholders::_2, st::_cb, std::placeholders::_3)
		.then(async_context, [](int val, std::string str)
		{
			std::cout << str << val << "@" << std::this_thread::get_id() << std::endl;
			return val * 2;
		})
		;

	t();		//开始运行任务链

	auto f = t.get_future();
	auto val = f.get();
	std::cout << "end value is " << val << "@" << std::this_thread::get_id() << std::endl;
}
