//测试基本的任务链。每次then后返回一个新的task<node_first, node_last>对象

#include "stdafx.h"

int fn_first(int val)
{
	std::cout << "first" << std::endl;
	return val * 3;
}
std::string fn_second(int val)
{
	std::cout << "second " << val << std::endl;
	return "abcde"s;
}
std::tuple<int, std::string> fn_third(std::string str)
{
	std::cout << "third " << str << std::endl;
	return std::make_tuple(2, "cdefg"s);
}
void fn_four(int val, std::string str)
{
	std::cout << "four " << val << " " << str << std::endl;
}

void test_task_link()
{
	using namespace st;

	task<task_node<int>, task_node<int>> t1 = make_task(fn_first, 3);
	task<task_node<std::string, int>, task_node<int>> t2 = t1.then(fn_second);
	task<task_node<std::tuple<int, std::string>, std::string>, task_node<int>> t3 = t2.then(imm_context, fn_third);
	task<task_node<void, int, std::string>, task_node<int>> t4 = t3.then(imm_context, fn_four);

	auto t = t4.then([]
	{
		std::cout << "lambda " << std::endl;
		return 0;
	});

	auto f = t.get_future();
	imm_context.add(t.get_executor());

	std::cout << "end value is " << f.get() << std::endl;
}
