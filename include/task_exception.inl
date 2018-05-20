#pragma once

namespace lib_shark_task
{
	struct task_set_exception
	{
		friend struct task_set_exception_agent;
	private:
		virtual void _Set_exception(std::exception_ptr && val) = 0;
	};

	struct task_set_exception_agent
	{
		std::atomic<task_set_exception *> _Impl = nullptr;
		void _Set_exception(std::exception_ptr && val)
		{
			auto exp = _Impl.load();
			exp->_Set_exception(std::forward<std::exception_ptr>(val));
		}
	};
	using task_set_exception_agent_sptr = std::shared_ptr<task_set_exception_agent>;
}
