#pragma once

namespace lib_shark_task
{
	struct immediate_task_context
	{
		inline void add(const executor_sptr & runner) const
		{
			assert(runner != nullptr);
			runner->run_once();
		}
	};
	extern immediate_task_context imm_context;

	struct async_task_context
	{
		inline void add(const executor_sptr & runner) const
		{
			assert(runner != nullptr);
			std::async(std::launch::async, [=] 
			{
				runner->run_once();
			});
		}
	};
	extern async_task_context async_context;
}
