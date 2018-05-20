#pragma once

namespace lib_shark_task
{
	template<class _Rtype, class... _Args>
	struct task_node;
	template<class _Cbtype, class... _Args>
	struct task_cbnode;

	namespace detail
	{
		template<class _Fx>
		using result_of_t = typename invoke_traits<_Fx>::result_type;			//萃取函数(对象)_Fx的返回值
		template<class _Fx>
		using args_tuple_t = typename invoke_traits<_Fx>::args_tuple_type;		//萃取函数(对象)_Fx的参数列表，这个参数列表用std::tuple<>来表达
		template<size_t _Idx, class _Fx>
		using args_of_t = typename invoke_traits<_Fx>::template args_element<_Idx>::type;	//萃取函数(对象)_Fx的特定索引的参数类型

#if 0
		template<size_t _Idx, class... _Args>
		void _Copy_to_tuple_impl(std::tuple<_Args...> & t) {}

		template<size_t _Idx, class... _Args, class _Ty, class... _Rest>
		void _Copy_to_tuple_impl(std::tuple<_Args...> & t, _Ty && val, _Rest&&... args)
		{
			std::get<_Idx>(t) = std::forward<_Ty>(val);
			_Copy_to_tuple_impl<_Idx + 1>(t, std::forward<_Rest>(args)...);
		}

		template<class... _Args>
		void _Copy_to_tuple(std::tuple<_Args...> & t, _Args&&... args)
		{
			_Copy_to_tuple_impl<0>(t, std::forward<_Args>(args)...);
		}
		template<class... _Args>
		void _Copy_to_tuple(std::tuple<_Args...> & t, const _Args&... args)
		{
			_Copy_to_tuple_impl<0>(t, args...);
		}
		template<class... _Args>
		void _Copy_to_tuple(std::tuple<_Args...> & t, std::tuple<_Args...> && t2)
		{
			t = std::forward<std::tuple<_Args...>>(t2);
		}
		template<class... _Args>
		void _Copy_to_tuple(std::tuple<_Args...> & t, const std::tuple<_Args...> & t2)
		{
			t = t2;
		}
#endif

		//使用上一个节点的返回值，作为下一个节点的参数调用下一个任务节点
		//如果上一个节点的返回值是std::tuple<...>，则拆包tuple为变参模板
		template<class _PrevArgs>
		struct _Invoke_then_impl
		{
			template<class _Rtype>
			struct unpack_tuple_node
			{
				using type = task_node<_Rtype, _PrevArgs>;
			};
			template<class _Rtype>
			struct unpack_tuple_cbnode
			{
				using type = task_cbnode<_Rtype, _PrevArgs>;
			};
			template<class _Rtype>
			struct unpack_tuple_function
			{
				using type = std::function<_Rtype(_PrevArgs)>;
			};

			template<class _Fx>
			static decltype(auto) Invoke(_Fx & f, const _PrevArgs & args)
			{
				return f(args);
			}
			template<class _Fx>
			static decltype(auto) Invoke(_Fx & f, _PrevArgs && args)
			{
				return f(std::forward<_PrevArgs>(args));
			}
		};
		template<>
		struct _Invoke_then_impl<void>
		{
			template<class _Rtype>
			struct unpack_tuple_node
			{
				using type = task_node<_Rtype>;
			};
			template<class _Rtype>
			struct unpack_tuple_cbnode
			{
				using type = task_cbnode<_Rtype>;
			};
			template<class _Rtype>
			struct unpack_tuple_function
			{
				using type = std::function<_Rtype()>;
			};

			template<class _Fx>
			static decltype(auto) Invoke(_Fx & f)
			{
				return f(args);
			}
		};
		template<class... _PrevArgs>
		struct _Invoke_then_impl<std::tuple<_PrevArgs...>>
		{
			using tuple_type = std::tuple<_PrevArgs...>;

			template<class _Rtype>
			struct unpack_tuple_node
			{
				using type = task_node<_Rtype, _PrevArgs...>;
			};
			template<class _Rtype>
			struct unpack_tuple_cbnode
			{
				using type = task_cbnode<_Rtype, _PrevArgs...>;
			};
			template<class _Rtype>
			struct unpack_tuple_function
			{
				using type = std::function<_Rtype(_PrevArgs...)>;
			};

			template<class _Fx>
			static decltype(auto) Invoke(_Fx & f, const tuple_type & args)
			{
				return std::apply(f, args);
			}
			template<class _Fx>
			static decltype(auto) Invoke(_Fx & f, tuple_type && args)
			{
				return std::apply(f, std::forward<tuple_type>(args));
			}
		};

		template<class _Fx, class _PrevArgs>
		void _Invoke_then(_Fx & f, _PrevArgs && val)
		{
			using value_type = std::remove_reference_t<_PrevArgs>;
			_Invoke_then_impl<value_type>::template Invoke<_Fx>(f, std::forward<_PrevArgs>(val));
		}
		template<class _Rtype, class _PrevArgs>
		using unpack_tuple_fn_t = typename _Invoke_then_impl<_PrevArgs>::template unpack_tuple_function<_Rtype>::type;
		template<class _Rtype, class _PrevArgs>
		using unpack_tuple_node_t = typename _Invoke_then_impl<_PrevArgs>::template unpack_tuple_node<_Rtype>::type;
		template<class _Rtype, class _PrevArgs>
		using unpack_tuple_cbnode_t = typename _Invoke_then_impl<_PrevArgs>::template unpack_tuple_cbnode<_Rtype>::type;


		template<class _Stype>
		struct _Set_then_helper
		{
			std::shared_ptr<_Stype> _Next;

			template<class... _Args>
			auto operator ()(_Args&&... args) const
			{
				if (_Next->invoke_thiz(std::forward<_Args>(args)...))
					_Next->invoke_then_if();
			}
		};

		template<class _Context, class _Stype>
		struct _Set_then_ctx_helper
		{
			_Context * _Ctx;
			std::shared_ptr<_Stype> _Next;

			template<class... _Args>
			auto operator ()(_Args&&... args) const
			{
				using executor_type = task_executor<_Stype>;
				auto exe = std::make_shared<executor_type>(_Next, std::forward<_Args>(args)...);

				_Ctx->add(exe);
			}
		};
	}
}
