
#include "task.h"
#include "task_context.h"

namespace lib_shark_task
{
	immediate_task_context imm_context;
	async_task_context async_context;
}

void test_task_link();
void test_task_async();
void test_task_optimal_move();
void test_task_conflict();
void test_task_exception();
void test_task_marshal();

void test_task_thread_safe();
void test_task_when_all();
void test_task_when_any();

int test_main()
{
	//test_task_link();
	//test_task_async();
	//test_task_optimal_move();
	////test_task_conflict();
	//test_task_exception();
	//test_task_marshal();
	//test_task_thread_safe();
	test_task_when_all();
	test_task_when_any();

	return 0;
}

