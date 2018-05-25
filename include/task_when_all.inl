#pragma once
#include "task_when_node.inl"

namespace lib_shark_task
{
	template<class _Ttuple, class... _ResultArgs>
	struct task_all_node : public node_impl<std::tuple<_ResultArgs...>, std::function<void()>, std::function<void(_ResultArgs...)>>
	{
		using this_type = task_all_node<_Ttuple, _ResultArgs...>;

		using result_type = std::tuple<_ResultArgs...>;		//本节点的结果的类型
		using result_tuple = result_type;					//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<>;				//本节点的入参打包成tuple<>后的类型

		using task_tuple = detail::package_tuple_t<_Ttuple>;
		task_tuple			_All_tasks;

		template<class... _Tasks>
		task_all_node(const task_set_exception_agent_sptr & exp, _Tasks&&... ts)
			: node_impl(exp)
			, _All_tasks(std::forward<_Tasks>(ts)...)
			, _Result_count(std::tuple_size<_Ttuple>::value)
		{
		}
		task_all_node(task_all_node && _Right) = default;
		task_all_node & operator = (task_all_node && _Right) = default;
		task_all_node(const task_all_node & _Right) = delete;
		task_all_node & operator = (const task_all_node & _Right) = delete;

	private:
		std::atomic<intptr_t> _Result_count;
	public:
		template<size_t _Idx, class... _PrevArgs2>
		void _Set_value_partial(size_t, _PrevArgs2&&... args)
		{
			std::unique_lock<std::mutex> _Lock(_Mtx());
			detail::_Fill_to_tuple<_Idx>(_Peek_value(), std::forward<_PrevArgs2>(args)...);
		}
		template<size_t _Idx, class _PrevTuple>
		void _Set_value_partial_t(size_t, _PrevTuple&& args)
		{
			std::unique_lock<std::mutex> _Lock(_Mtx());
			detail::_Move_to_tuple<_Idx>(_Peek_value(), std::forward<_PrevTuple>(args));
		}

		void _On_result(size_t)
		{
			if (--_Result_count == 0)
			{
				_Ptr()->_Set_value(false);
				_Ready = true;
				invoke_then_if();
			}
		}

		template<class... Args2>
		bool invoke_thiz(Args2&&... args)
		{
			static_assert(sizeof...(Args2) >= typename std::tuple_size<args_tuple_type>::value, "");

			try
			{
				std::unique_lock<std::mutex> _Lock(_Mtx());
				task_tuple all_task = std::move(_All_tasks);
				_Lock.unlock();

				for_each(all_task, [&](auto & ts)
				{
					ts(args...);
				});
			}
			catch (...)
			{
				_Set_Agent_exception(std::current_exception());
			}

			return false;
		}

		template<class _PrevTuple>
		bool invoke_thiz_tuple(_PrevTuple&& args)
		{
			static_assert(typename std::tuple_size<_PrevTuple>::value >= typename std::tuple_size<args_tuple_type>::value, "");

			try
			{
				std::unique_lock<std::mutex> _Lock(_Mtx());
				task_tuple all_task = std::move(_All_tasks);
				_Lock.unlock();

				for_each(all_task, [&](auto & ts)
				{
					std::apply(ts, args);
				});
			}
			catch (...)
			{
				_Set_Agent_exception(std::current_exception());
			}

			return false;
		}
	};
	template<class _Ttuple, class... _ResultArgs>
	struct task_all_node<_Ttuple, std::tuple<_ResultArgs...>> : public task_all_node<_Ttuple, _ResultArgs...>
	{
		using task_all_node<_Ttuple, _ResultArgs...>::task_all_node;
	};
}
