//测试基本的任务链。每次then后返回一个新的task<node_first, node_last>对象
#include "task.h"
#include "task_context.h"
#include "log_print.h"

using namespace std::literals;

int fn_first(int val)
{
	log_print("first");
	return val * 3;
}
std::string fn_second(int val)
{
	log_print("second", val);
	return "abcde"s;
}
std::tuple<int, std::string> fn_third(std::string str)
{
	log_print("third", str);
	return std::make_tuple(2, "cdefg"s);
}
void fn_four(int val, std::string str)
{
	log_print("four ", val, " ", str);
}

void test_task_link()
{
	using namespace st;

	task<task_node<int, int>, task_node<int, int>> t1 = make_task(std::bind(fn_first, 3));
	task<task_node<std::string, int>, task_node<int, int>> t2 = t1.then(fn_second);
	task<task_node<std::tuple<int, std::string>, std::string>, task_node<int, int>> t3 = t2.then(imm_context, fn_third);
	task<task_node<void, int, std::string>, task_node<int, int>> t4 = t3.then(imm_context, fn_four);

	auto t = t4.then([]
	{
		log_print("lambda ");
		return 0;
	});

	auto f = t.get_future();
	imm_context.add(t.get_executor());

	log_print("end value is ", f.get());
}
