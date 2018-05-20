
#pragma once

namespace lib_shark_task
{
	struct executor
	{
		virtual ~executor() {}

		virtual void run_once() = 0;
	};
	using executor_sptr = std::shared_ptr<executor>;

	template<class _St>
	struct task_executor : public executor
	{
		using node_type = _St;
		using result_type = typename node_type::result_type;
		using args_tuple_type = typename node_type::args_tuple_type;

	private:
		std::shared_ptr<node_type>	_State;
	public:
		args_tuple_type				_Parames;

		template<class... _Rest>
		task_executor(const std::shared_ptr<node_type> & st, _Rest&&... args)
			: _State(st)
			, _Parames{std::forward<_Rest>(args)...}
		{
			assert(_State != nullptr);
		}

		task_executor(const task_executor & st) = default;
		task_executor(task_executor && st) = default;
		task_executor & operator = (const task_executor & st) = default;
		task_executor & operator = (task_executor && st) = default;

		virtual void run_once() override
		{
			assert(_State != nullptr);

			if (!_State->is_ready())
			{
				if (_State->invoke_thiz_tuple(std::move(_Parames)))
					_State->invoke_then_if();
			}
		}
	};

}
