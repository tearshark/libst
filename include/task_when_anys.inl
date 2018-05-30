#pragma once
#include <any>
#include "task_when_node.inl"

namespace lib_shark_task
{
	//when_any(_Task1, _Task2, ...)
	//当所有的任务的返回值都相同的时候，可以将结果简化为(size_t, result_type...)。
	//task_anys_node就是用于这种情况
	//
	//_All_tasks 由于任务的类型不同，导致只能用一个std::tuple<>来保存
	//因此，invoke_thiz/invoke_thiz_tuple采用for_each(std::tuple<>)语法来调用_All_tasks
	//
	//由于返回结果相同
	//_Set_value_partial/_Set_value_partial_t 将idx存到get<0>(result_type)里，其他参数放在result_type后续的参数里
	//
	//_On_result 检测结果已经获得，并且之前保存的get<0>(result_type)与idx相同，则调用invoke_then_if
	template<class _Ttuple, class... _ResultArgs>
	struct task_anys_node : public node_impl<std::tuple<size_t, _ResultArgs...>, std::function<void()>, std::function<void(size_t, _ResultArgs...)>>
	{
		using base_type = node_impl<std::tuple<size_t, _ResultArgs...>, std::function<void()>, std::function<void(size_t, _ResultArgs...)>>;
		using this_type = task_when_one<_Ttuple, _ResultArgs...>;

		using element_type = std::tuple<size_t, _ResultArgs...>;
		using result_type = std::tuple<size_t, _ResultArgs...>;		//本节点的结果的类型
		using result_tuple = result_type;							//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<>;						//本节点的入参打包成tuple<>后的类型

		using task_tuple = detail::package_tuple_t<_Ttuple>;
		task_tuple			_All_tasks;

		using task_function = typename base_type::task_function;
		using then_function = typename base_type::then_function;

		template<class... _Tasks>
		task_anys_node(const task_set_exception_agent_sptr & exp, _Tasks&&... ts)
			: base_type(exp)
			, _All_tasks(std::forward<_Tasks>(ts)...)
		{
		}
		task_anys_node(task_anys_node && _Right) = default;
		task_anys_node & operator = (task_anys_node && _Right) = default;
		task_anys_node(const task_anys_node & _Right) = delete;
		task_anys_node & operator = (const task_anys_node & _Right) = delete;

	private:
		std::atomic<bool> _Result_retrieved{ false };
	public:
		template<size_t _Idx, class... _PrevArgs2>
		void _Set_value_partial(size_t idx, _PrevArgs2&&... args)
		{
			std::unique_lock<std::mutex> _Lock(this->_Mtx());
			if (!_Result_retrieved)
			{
				_Result_retrieved = true;
				detail::_Fill_to_tuple<0>(this->_Peek_value(), idx, std::forward<_PrevArgs2>(args)...);
			}
		}
		template<size_t _Idx, class _PrevTuple>
		void _Set_value_partial_t(size_t idx, _PrevTuple&& args)
		{
			std::unique_lock<std::mutex> _Lock(this->_Mtx());
			if (!_Result_retrieved)
			{
				_Result_retrieved = true;
				std::get<0>(this->_Peek_value()) = idx;
				detail::_Move_to_tuple<1>(this->_Peek_value(), std::forward<_PrevTuple>(args));
			}
		}

		void _On_result(size_t idx)
		{
			std::unique_lock<std::mutex> _Lock(this->_Mtx());

			if (_Result_retrieved && idx == std::get<0>(this->_Peek_value()))
			{
				_Lock.unlock();

				this->_Set_value();
				this->_Ready = true;
				this->invoke_then_if();
			}
		}

		template<class... Args2>
		bool invoke_thiz(Args2&&... args)
		{
			static_assert(sizeof...(Args2) >= std::tuple_size<args_tuple_type>::value, "");

			try
			{
				std::unique_lock<std::mutex> _Lock(this->_Mtx());
				task_tuple all_task = std::move(_All_tasks);
				_Lock.unlock();

				for_each(all_task, [&](auto & ts)
				{
					ts(args...);
				});
			}
			catch (...)
			{
				this->_Set_Agent_exception(std::current_exception());
			}

			return false;
		}

		template<class _PrevTuple>
		bool invoke_thiz_tuple(_PrevTuple&& args)
		{
			static_assert(std::tuple_size<_PrevTuple>::value >= std::tuple_size<args_tuple_type>::value, "");

			try
			{
				std::unique_lock<std::mutex> _Lock(this->_Mtx());
				task_tuple all_task = std::move(_All_tasks);
				_Lock.unlock();

				for_each(all_task, [&](auto & ts)
				{
					std::apply(ts, args);
				});
			}
			catch (...)
			{
				this->_Set_Agent_exception(std::current_exception());
			}

			return false;
		}
	};
	template<class _Ttuple, class... _ResultArgs>
	struct task_anys_node<_Ttuple, std::tuple<_ResultArgs...>> : public task_anys_node<_Ttuple, _ResultArgs...>
	{
		using task_anys_node<_Ttuple, _ResultArgs...>::task_anys_node;
	};
}
