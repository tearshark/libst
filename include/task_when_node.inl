#pragma once
#include <functional>

namespace lib_shark_task
{
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
		using this_type = task_when_one<_Ndone, _PrevArgs...>;

		using result_type = void;									//本节点的结果的类型
		using result_tuple = std::tuple<>;							//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<_PrevArgs...>;			//本节点的入参打包成tuple<>后的类型

		using notify_node = typename _Ndone::notify_node;
		using cated_tuple = typename _Ndone::cated_tuple;
		static const size_t cated_tuple_index = typename _Ndone::index;
	private:
		notify_node *						_Notify;
		size_t								_Index;
	public:
		task_when_one(const task_set_exception_agent_sptr & exp, notify_node * nn, size_t idx)
			: node_impl(exp)
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
			static_assert(sizeof...(_PrevArgs2) >= typename std::tuple_size<args_tuple_type>::value, "");

			try
			{
				_Notify->template _Set_value_partial<cated_tuple_index>(_Index, std::forward<_PrevArgs2>(args)...);
				_Set_value(0);
				_Ready = true;
			}
			catch (...)
			{
				_Set_Agent_exception(std::current_exception());
			}

			return _Ready;
		}

		template<class _PrevTuple>
		bool invoke_thiz_tuple(_PrevTuple&& args)
		{
			static_assert(typename std::tuple_size<_PrevTuple>::value >= typename std::tuple_size<args_tuple_type>::value, "");

			try
			{
				_Notify->template _Set_value_partial_t<cated_tuple_index>(_Index, std::forward<_PrevTuple>(args));
				_Set_value(0);
				_Ready = true;
			}
			catch (...)
			{
				_Set_Agent_exception(std::current_exception());
			}

			return _Ready;
		}

		void invoke_then_if()
		{
			if (_Notify == nullptr)
				std::_Throw_future_error(
					std::make_error_code(std::future_errc::future_already_retrieved));

			_Notify->_On_result(_Index);
			_Notify = nullptr;
		}
	};

	template<class _Ndone, class... _PrevArgs>
	struct task_when_one<_Ndone, std::tuple<_PrevArgs...>> : public task_when_one<_Ndone, _PrevArgs...>
	{
		using task_when_one<_Ndone, _PrevArgs...>::task_when_one;
	};
}
