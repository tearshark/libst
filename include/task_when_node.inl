#pragma once

namespace lib_shark_task
{
	namespace detail
	{
		template<class _Task>
		decltype(auto) declval_task_last_node()
		{
			using task_type = std::decay_t<_Task>;
			static_assert(detail::is_task<task_type>::value, "make sure '_Task' is a task");
			return std::declval<typename task_type::last_node>();
		}
		template<class _Task>
		auto declval_task_last_node_result_tuple()
		{
			using task_type = std::decay_t<_Task>;
			static_assert(detail::is_task<task_type>::value, "make sure '_Task' is a task");
			using node_type = typename task_type::last_node;
			return std::declval<typename node_type::result_tuple>();
		}
		template<class _Task>
		int check_task_type()
		{
			static_assert(detail::is_task<_Task>::value, "use 'make_task' or 'marshal_task' to create a task");
			return 0;
		}
	}

	template<class _Anode, class _Ctuple, size_t _Idx>
	struct when_node_args
	{
		using notify_node = _Anode;
		using cated_tuple = _Ctuple;
		static const size_t index = _Idx;
	};

	//_Ndone : when_node_args<>类型
	//_PrevArgs : 上一个节点的返回值
	//最后要将_PrevArgs...合并到 when_node_args<>::cated_tuple 里面去，然后通知 when_node_args<>::notify_node::_On_result()
	template<class _Ndone, class... _PrevArgs>
	struct task_when_one : public node_impl<int, std::function<void(_PrevArgs...)>>
	{
		using base_type = node_impl<int, std::function<void(_PrevArgs...)>>;
		using this_type = task_when_one<_Ndone, _PrevArgs...>;

		using result_type = void;									//本节点的结果的类型
		using result_tuple = std::tuple<>;							//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<_PrevArgs...>;			//本节点的入参打包成tuple<>后的类型

		using notify_node = typename _Ndone::notify_node;
		using cated_tuple = typename _Ndone::cated_tuple;
		static const size_t cated_tuple_index = _Ndone::index;
	private:
		notify_node *						_Notify;
		size_t								_Index;
	public:
		task_when_one(const task_set_exception_agent_sptr & exp, notify_node * nn, size_t idx)
			: base_type(exp)
			, _Notify(nn)
			, _Index(idx)
		{
		}
		task_when_one(task_when_one && _Right) = default;
		task_when_one & operator = (task_when_one && _Right) = default;
		task_when_one(const task_when_one & _Right) = delete;
		task_when_one & operator = (const task_when_one & _Right) = delete;

		template<class... _PrevArgs2>
		bool invoke_thiz(_PrevArgs2&&... args)
		{
			static_assert(sizeof...(_PrevArgs2) >= std::tuple_size<args_tuple_type>::value, "");

			try
			{
				_Notify->template _Set_value_partial<cated_tuple_index>(_Index, std::forward<_PrevArgs2>(args)...);
				this->_Set_value(0);
				this->_Ready = true;
			}
			catch (...)
			{
				this->_Set_Agent_exception(std::current_exception());
			}

			return this->_Ready;
		}

		template<class _PrevTuple>
		bool invoke_thiz_tuple(_PrevTuple&& args)
		{
			static_assert(std::tuple_size<_PrevTuple>::value >= std::tuple_size<args_tuple_type>::value, "");

			try
			{
				_Notify->template _Set_value_partial_t<cated_tuple_index>(_Index, std::forward<_PrevTuple>(args));
				this->_Set_value(0);
				this->_Ready = true;
			}
			catch (...)
			{
				this->_Set_Agent_exception(std::current_exception());
			}

			return this->_Ready;
		}

		void invoke_then_if()
		{
			if (_Notify == nullptr)
				throw std::future_error(std::make_error_code(std::future_errc::future_already_retrieved));

			_Notify->_On_result(_Index);
			_Notify = nullptr;
		}
	};

	template<class _Ndone, class... _PrevArgs>
	struct task_when_one<_Ndone, std::tuple<_PrevArgs...>> : public task_when_one<_Ndone, _PrevArgs...>
	{
		using task_when_one<_Ndone, _PrevArgs...>::task_when_one;
	};

	namespace detail
	{
		template<size_t _Idx, class _Anode, class _Task>
		auto when_wait_one_impl(_Anode * all_node, size_t node_idx, _Task & tf)
		{
			using tuple_type = decltype(declval_task_last_node_result_tuple<_Task>());

			using task_type = std::remove_reference_t<_Task>;
			using result_tuple = typename _Anode::element_type;
			using node_args_type = when_node_args<_Anode, result_tuple, _Idx>;

			using next_node_type = task_when_one<node_args_type, tuple_type>;

			task_set_exception_agent_sptr exp = tf._Get_exception_agent();
			exp->_Impl = all_node;

			auto st_next = std::make_shared<next_node_type>(exp, all_node, node_idx);
			return tf.template _Then_node<next_node_type>(st_next);
		}

		template<class _Anode, class _Cont>
		void when_iter_impl(_Anode * all_node, _Cont & c)
		{
			size_t idx = 0;
			for (auto & t : c)
				when_wait_one_impl<0>(all_node, idx++, t);
		}

		template<class _Node, class _Iter>
		auto when_iter(_Iter _First, _Iter _Last)
		{
			task_set_exception_agent_sptr exp;
			if (_First != _Last)
				exp = _First->_Get_exception_agent();
			else
				exp = std::make_shared<task_set_exception_agent>();

			using first_node_type = _Node;
			auto st_first = std::make_shared<first_node_type>(exp, _First, _Last);
			exp->_Impl = st_first.get();

			when_iter_impl(st_first.get(), st_first->_All_tasks);

			return task<first_node_type, first_node_type>{exp, st_first, st_first};
		}

	}
}
