#pragma once
#include "task_when_node.inl"

namespace lib_shark_task
{
	template<class _Ttype, class _ResultArgs>
	struct task_allv_node : public node_impl<std::vector<_ResultArgs>, std::function<void()>, std::function<void(std::vector<_ResultArgs>)>>
	{
		using this_type = task_allv_node<_Ttype, _ResultArgs>;

		using element_type = _ResultArgs;
		using result_type = std::vector<_ResultArgs>;				//本节点的结果的类型
		using result_tuple = std::tuple<result_type>;				//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<>;						//本节点的入参打包成tuple<>后的类型

		using task_vector = std::vector<_Ttype>;
		task_vector			_All_tasks;

		template<class _Iter>
		task_allv_node(const task_set_exception_agent_sptr & exp, _Iter _First, _Iter _Last)
			: node_impl(exp)
		{
			_All_tasks.reserve(std::distance(_First, _Last));
			for (; _First != _Last; ++_First)
				_All_tasks.emplace_back(std::move(*_First));
			
			_Result_count = _All_tasks.size();
			_Peek_value().resize(_Result_count);
		}
		task_allv_node(task_allv_node && _Right) = default;
		task_allv_node & operator = (task_allv_node && _Right) = default;
		task_allv_node(const task_allv_node & _Right) = delete;
		task_allv_node & operator = (const task_allv_node & _Right) = delete;

	private:
		std::atomic<intptr_t> _Result_count;
	public:
		template<size_t _Idx, class... _PrevArgs2>
		void _Set_value_partial(size_t idx, _PrevArgs2&&... args)
		{
			assert(idx < _Peek_value().size());

			std::unique_lock<std::mutex> _Lock(_Mtx());
			detail::_Fill_to_tuple<_Idx>(_Peek_value()[idx], std::forward<_PrevArgs2>(args)...);
		}
		template<size_t _Idx, class _PrevTuple>
		void _Set_value_partial_t(size_t idx, _PrevTuple&& args)
		{
			assert(idx < _Peek_value().size());

			std::unique_lock<std::mutex> _Lock(_Mtx());
			_Peek_value()[idx] = std::forward<_PrevTuple>(args);
			//detail::_Move_to_tuple<_Idx>(_Peek_value()[idx], std::forward<_PrevTuple>(args));
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
				task_vector all_task = std::move(_All_tasks);
				_Lock.unlock();

				if (all_task.size() > 0)
				{
					for (auto & ts : all_task)
						ts(args...);
				}
				else
				{
					_Ptr()->_Set_value(false);
					_Ready = true;
					return true;
				}
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
				task_vector all_task = std::move(_All_tasks);
				_Lock.unlock();

				if (all_task.size() > 0)
				{
					for (auto & ts : all_task)
						std::apply(ts, args);
				}
				else
				{
					_Ptr()->_Set_value(false);
					_Ready = true;
					return true;
				}
			}
			catch (...)
			{
				_Set_Agent_exception(std::current_exception());
			}

			return false;
		}
	};
	template<class _Ttuple, class... _ResultArgs>
	struct task_allv_node<_Ttuple, std::tuple<_ResultArgs...>> : public task_allv_node<_Ttuple, _ResultArgs...>
	{
		using task_allv_node<_Ttuple, _ResultArgs...>::task_allv_node;
	};

}
