#pragma once
#include "task_when_node.inl"

namespace lib_shark_task
{
	namespace detail
	{
		template<class _Task>
		decltype(auto) declval_task_last_node(const _Task&)
		{
			using task_type = std::decay_t<_Task>;
			return std::declval<typename task_type::last_node>();
		}
		template<class _Task>
		auto declval_task_last_node_result_tuple(const _Task&)
		{
			using task_type = std::decay_t<_Task>;
			using node_type = typename task_type::last_node;
			return std::declval<typename node_type::result_tuple>();
		}

		template<size_t _Idx, class _Anode>
		void when_all_impl2(_Anode * all_node, size_t node_idx)
		{
		}

		template<size_t _Idx, class _Anode, class _Task>
		auto when_all_one_impl(_Anode * all_node, size_t node_idx, _Task && tf)
		{
			static_assert(invoke_enable<std::remove_reference_t<_Task>>::value, "the task must be call without parames");

			using tuple_type = decltype(declval_task_last_node_result_tuple<_Task>(tf));

			using task_type = std::remove_reference_t<_Task>;
			using result_tuple = typename _Anode::result_tuple;
			using node_args_type = when_node_args<_Anode, result_tuple, _Idx>;

			using next_node_type = task_when_one<node_args_type, tuple_type>;

			task_set_exception_agent_sptr exp = tf._Get_exception_agent();
			exp->_Impl = all_node;

			auto st_next = std::make_shared<next_node_type>(exp, all_node, node_idx, all_node->_Peek_tuple());
			return tf.template _Then_node<next_node_type>(st_next);
		}

		template<size_t _Idx, class _Anode, class _Task, class... _TaskRest>
		void when_all_impl2(_Anode * all_node, size_t node_idx, _Task && tf, _TaskRest&&... rest)
		{
			when_all_one_impl<_Idx>(all_node, node_idx, tf);

			using tuple_type = decltype(declval_task_last_node_result_tuple<_Task>(tf));
			when_all_impl2<_Idx + std::tuple_size_v<tuple_type>>(all_node, ++node_idx, std::forward<_TaskRest>(rest)...);
		}

		template<class _Anode, class _Ttuple, size_t... Idx>
		void when_all_impl(_Anode * all_node, _Ttuple & tasks, std::index_sequence<Idx...>)
		{
			when_all_impl2<0>(all_node, 0, std::get<Idx>(tasks)...);
		}
	}

	template<class _Ttuple, class... _ResultArgs>
	struct task_all_node : public node_impl<std::tuple<_ResultArgs...>, std::function<void()>, std::function<void(_ResultArgs...)>>
	{
		using this_type = task_when_one<_Ttuple, _ResultArgs...>;

		using result_type = std::tuple<_ResultArgs...>;		//本节点的结果的类型
		using result_tuple = result_type;					//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<>;				//本节点的入参打包成tuple<>后的类型

		using task_tuple = detail::package_tuple_t<_Ttuple>;
		task_tuple			_All_tasks;

		template<class... _Tasks>
		task_all_node(const task_set_exception_agent_sptr & exp, _Tasks&&... ts)
			: node_impl(exp)
			, _All_tasks(std::forward<_Tasks>(ts)...)
			, _Result_count(std::tuple_size_v<_Ttuple>)
		{
		}
		task_all_node(task_all_node && _Right) = default;
		task_all_node & operator = (task_all_node && _Right) = default;
		task_all_node(const task_all_node & _Right) = delete;
		task_all_node & operator = (const task_all_node & _Right) = delete;

	private:
		std::atomic<intptr_t> _Result_count;
	public:
		result_tuple & _Peek_tuple()
		{
			return _Peek_value();
		}
		std::mutex & _Peek_mutex()
		{
			return _Mtx();
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

		void invoke_then_if()
		{
			if (!_Ready)
				return;

			then_function fn = _Move_then();
			if (!fn)
				return;

			try
			{
				detail::_Apply_then(fn, std::move(_Get_value()));
				//detail::_Invoke_then(fn, std::move(_Get_value()));
			}
			catch (...)
			{
				_Set_Agent_exception(std::current_exception());
			}
		}

		template<class _NextFx>
		void _Set_then_if(_NextFx && fn)
		{
			_Set_retrieved();

			if (_Ready)
			{
				try
				{
					detail::_Apply_then2<then_function>(std::forward<_NextFx>(fn), std::move(_Get_value()));
					//detail::_Invoke_then(fn, std::move(_Get_value()));
				}
				catch (...)
				{
					_Set_Agent_exception(std::current_exception());
				}
			}
			else
			{
				std::unique_lock<std::mutex> _Lock(_Mtx());
				_Then = then_function{ std::forward<_NextFx>(fn) };
			}
		}
	};
	template<class _Ttuple, class... _ResultArgs>
	struct task_all_node<_Ttuple, std::tuple<_ResultArgs...>> : public task_all_node<_Ttuple, _ResultArgs...>
	{
		using task_all_node<_Ttuple, _ResultArgs...>::task_all_node;
	};


	//等待多个任务完成
	//多个任务的结果，放在一个拼合的tuple<>里
	//首先做一个全新的task<task_all_node, task_all_node>
	//	task_all_node::invoke_thiz 主要负责调用所有的任务，以便于开始任务
	//		为每个任务造一个task_when_one。
	//		task_when_one 负责将结果放入到 task_all_node的拼合tuple<>里，然后通知 task_all_node 有一个任务完成
	//		task_all_node 在所有任务完成后，调用invoke_then_if
	//
	template<class _Task, class... _TaskRest>
	auto when_all(_Task& tfirst, _TaskRest&... rest)
	{
		using cated_task_t = std::tuple<std::remove_reference_t<_Task>, std::remove_reference_t<_TaskRest>...>;
		using cated_result_t = decltype(std::tuple_cat(detail::declval_task_last_node_result_tuple<_Task>(tfirst), detail::declval_task_last_node_result_tuple<_TaskRest>(rest)...));

		task_set_exception_agent_sptr exp = tfirst._Get_exception_agent();

		using first_node_type = task_all_node<cated_task_t, cated_result_t>;
		auto st_first = std::make_shared<first_node_type>(exp, std::move(tfirst), std::move(rest)...);
		exp->_Impl = st_first.get();

		detail::when_all_impl(st_first.get(), st_first->_All_tasks, std::make_index_sequence<std::tuple_size_v<cated_task_t>>{});

		return task<first_node_type, first_node_type>{exp, st_first, st_first};
	}
}
