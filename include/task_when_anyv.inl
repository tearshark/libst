#pragma once
#include "task_when_node.inl"

namespace lib_shark_task
{
	template<class _Ttype, class... _ResultArgs>
	struct task_anyv_node : public node_impl<std::tuple<size_t, _ResultArgs...>, std::function<void()>, std::function<void(size_t, _ResultArgs...)>>
	{
		using base_type = node_impl<std::tuple<size_t, _ResultArgs...>, std::function<void()>, std::function<void(size_t, _ResultArgs...)>>;
		using this_type = task_anyv_node<_Ttype, _ResultArgs...>;

		using element_type = std::tuple<size_t, _ResultArgs...>;
		using result_type = std::tuple<size_t, _ResultArgs...>;		//本节点的结果的类型
		using result_tuple = result_type;							//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<>;						//本节点的入参打包成tuple<>后的类型

		using task_vector = std::vector<_Ttype>;
		task_vector			_All_tasks;

		using task_function = typename base_type::task_function;
		using then_function = typename base_type::then_function;

		template<class _Iter>
		task_anyv_node(const task_set_exception_agent_sptr & exp, _Iter _First, _Iter _Last)
			: base_type(exp)
		{
			_All_tasks.reserve(std::distance(_First, _Last));
			for (; _First != _Last; ++_First)
				_All_tasks.emplace_back(std::move(*_First));
		}
		task_anyv_node(task_anyv_node && _Right) = default;
		task_anyv_node & operator = (task_anyv_node && _Right) = default;
		task_anyv_node(const task_anyv_node & _Right) = delete;
		task_anyv_node & operator = (const task_anyv_node & _Right) = delete;

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
				task_vector all_task = std::move(_All_tasks);
				_Lock.unlock();

				if (all_task.size() > 0)
				{
					for (auto & ts : all_task)
						ts(args...);
				}
				else
				{
					this->_Set_value();
					this->_Ready = true;
					return true;
				}
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
				task_vector all_task = std::move(_All_tasks);
				_Lock.unlock();

				if (all_task.size() > 0)
				{
					for (auto & ts : all_task)
						std::apply(ts, args);
				}
				else
				{
					this->_Set_value();
					this->_Ready = true;
					return true;
				}
			}
			catch (...)
			{
				this->_Set_Agent_exception(std::current_exception());
			}

			return false;
		}

		void invoke_then_if()
		{
			if (!this->_Ready)
				return;

			then_function fn = this->_Move_then();
			if (!fn)
				return;

			try
			{
				detail::_Apply_then(fn, std::move(this->_Get_value()));
			}
			catch (...)
			{
				this->_Set_Agent_exception(std::current_exception());
			}
		}

		template<class _NextFx>
		void _Set_then_if(_NextFx && fn)
		{
			this->_Set_retrieved();

			if (this->_Ready)
			{
				try
				{
					detail::_Apply_then2<then_function>(std::forward<_NextFx>(fn), std::move(this->_Get_value()));
				}
				catch (...)
				{
					this->_Set_Agent_exception(std::current_exception());
				}
			}
			else
			{
				std::unique_lock<std::mutex> _Lock(this->_Mtx());
				this->_Then = then_function{ std::forward<_NextFx>(fn) };
			}
		}
	};
	template<class _Ttuple, class... _ResultArgs>
	struct task_anyv_node<_Ttuple, std::tuple<_ResultArgs...>> : public task_anyv_node<_Ttuple, _ResultArgs...>
	{
		using task_anyv_node<_Ttuple, _ResultArgs...>::task_anyv_node;
	};

}
