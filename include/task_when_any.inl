#pragma once
#include <any>
#include "task_when_node.inl"

namespace lib_shark_task
{
	namespace detail
	{
		template<class... _Types>
		struct make_any_unpack_tuple_impl
		{
			static std::any make(_Types&&... args)
			{
				return std::make_any(std::forward<_Types>(args)...);
			}
		};
		template<>
		struct make_any_unpack_tuple_impl<>
		{
			static std::any make()
			{
				return std::any{};
			}
		};
		template<class _Ty>
		struct make_any_unpack_tuple_impl<_Ty>
		{
			static std::any make(_Ty && val)
			{
				return std::make_any<typename std::remove_reference<_Ty>::type>(std::forward<_Ty>(val));
			}
		};
		template<class _Ty>
		struct make_any_unpack_tuple_impl<std::tuple<_Ty>>
		{
			static std::any make(std::tuple<_Ty> && val)
			{
				return std::make_any(std::get<0>(std::forward<std::tuple<_Ty>>(val)));
			}
		};

		template<class... _Types>
		std::any make_any_unpack(_Types&&... args)
		{
			return make_any_unpack_tuple_impl<_Types...>::make(std::forward<_Types>(args)...);
		}
		template<class _Tuple, size_t... _Idx>
		std::any make_any_unpack_tuple_2(_Tuple&& args, std::index_sequence<_Idx...>)
		{
			return make_any_unpack(std::get<_Idx>(std::forward<_Tuple>(args))...);
		}
		template<class... _Types>
		std::any make_any_unpack_tuple(std::tuple<_Types...>&& args)
		{
			return make_any_unpack_tuple_2(std::forward<std::tuple<_Types...>>(args), std::make_index_sequence<sizeof...(_Types)>{});
		}
	}

	template<class _Ttuple>
	struct task_any_node : public node_impl<std::tuple<size_t, std::any>, std::function<void()>, std::function<void(size_t, std::any)>>
	{
		using base_type = node_impl<std::tuple<size_t, std::any>, std::function<void()>, std::function<void(size_t, std::any)>>;
		using this_type = task_when_one<_Ttuple>;

		using element_type = std::tuple<size_t, std::any>;
		using result_type = std::tuple<size_t, std::any>;		//本节点的结果的类型
		using result_tuple = result_type;					//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<>;				//本节点的入参打包成tuple<>后的类型

		using task_tuple = detail::package_tuple_t<_Ttuple>;
		std::shared_ptr<task_tuple>	_All_tasks;

		template<class... _Tasks>
		task_any_node(const task_set_exception_agent_sptr & exp, _Tasks&&... ts)
			: base_type(exp)
			, _All_tasks(std::make_shared<task_tuple>(std::forward<_Tasks>(ts)...))
		{
		}
		task_any_node(task_any_node && _Right) = default;
		task_any_node & operator = (task_any_node && _Right) = default;
		task_any_node(const task_any_node & _Right) = delete;
		task_any_node & operator = (const task_any_node & _Right) = delete;

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
				std::get<0>(this->_Peek_value()) = idx;
				std::get<1>(this->_Peek_value()) = detail::make_any_unpack(std::forward<_PrevArgs2>(args)...);
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
				std::get<1>(this->_Peek_value()) = detail::make_any_unpack_tuple(std::forward<_PrevTuple>(args));
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

		void break_link()
		{
			_All_tasks = nullptr;
		}

		template<class... Args2>
		bool invoke_thiz(Args2&&... args)
		{
			static_assert(sizeof...(Args2) >= std::tuple_size<args_tuple_type>::value, "");

			try
			{
				std::unique_lock<std::mutex> _Lock(this->_Mtx());
				auto all_task = std::move(_All_tasks);
				_Lock.unlock();

				for_each(*all_task, [&](auto & ts)
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
				auto all_task = std::move(_All_tasks);
				_Lock.unlock();

				for_each(*all_task, [&](auto & ts)
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
}
