#pragma once

namespace lib_shark_task
{
	template<class _Rtype, class... _Args>
	struct task_node;
	template<class _Cbtype, class... _Args>
	struct task_cbnode;

	namespace detail
	{
		struct dummy_method_
		{
			void operator()()const
			{}
		};

		template<class _Fx>
		using result_of_t = typename invoke_traits<_Fx>::result_type;			//萃取函数(对象)_Fx的返回值
		template<class _Fx>
		using args_tuple_t = typename invoke_traits<_Fx>::args_tuple_type;		//萃取函数(对象)_Fx的参数列表，这个参数列表用std::tuple<>来表达
		template<size_t _Idx, class _Fx>
		using args_of_t = typename invoke_traits<_Fx>::template args_element<_Idx>::type;	//萃取函数(对象)_Fx的特定索引的参数类型

		template<size_t _Idx, class _Tuple>
		inline void _Fill_to_tuple_impl(_Tuple & t) {}
		template<size_t _Idx, class _Tuple, class _Ty, class... _Rest>
		inline void _Fill_to_tuple_impl(_Tuple & t, _Ty && val, _Rest&&... args)
		{
			std::get<_Idx>(t) = std::forward<_Ty>(val);
			_Fill_to_tuple_impl<_Idx + 1>(t, std::forward<_Rest>(args)...);
		}
		template<size_t _Idx>
		struct _Copy_to_tuple_impl
		{
			template<size_t _Offset, class _T1, class _T2>
			static inline void copy(_T1 & target, _T2&& source)
			{
				std::get<_Offset + _Idx - 1>(target) = std::get<_Idx - 1>(source);
				_Copy_to_tuple_impl<_Idx - 1>::template copy<_Offset>(target, source);
			}			
			template<size_t _Offset, class _T1, class _T2>
			static inline void move(_T1 & target, _T2&& source)
			{
				std::get<_Offset + _Idx - 1>(target) = std::move(std::get<_Idx - 1>(source));
				_Copy_to_tuple_impl<_Idx - 1>::template move<_Offset>(target, source);
			}
		};
		template<>
		struct _Copy_to_tuple_impl<0>
		{
			template<size_t _Offset, class _T1, class _T2>
			static inline void copy(_T1 & target, _T2&& source)
			{
			}
			template<size_t _Offset, class _T1, class _T2>
			static inline void move(_T1 & target, _T2&& source)
			{
			}
		};

		template<class _Tuple>
		struct _Fill_to_tuple_selector
		{
			template<size_t _Idx, class _Ty, class... _Rest>
			static inline void _fill(_Tuple & t, _Ty && val, _Rest&&... args)
			{
				t = std::forward<_Ty>(val);
			}
		};
		template<class... _Args>
		struct _Fill_to_tuple_selector<std::tuple<_Args...>>
		{
			using _Tuple = std::tuple<_Args...>;
			template<size_t _Idx, class... _Rest>
			static inline void _fill(_Tuple & t, _Rest&&... args)
			{
				_Fill_to_tuple_impl<_Idx>(t, std::forward<_Rest>(args)...);
			}
		};

		template<size_t _Idx, class _Tuple, class... _Args>
		inline void _Fill_to_tuple(_Tuple & t, _Args&&... args)
		{
			_Fill_to_tuple_selector<_Tuple>::template _fill<_Idx>(t, std::forward<_Args>(args)...);
		}
		template<size_t _Idx, class _Tuple, class _Tuple2>
		inline void _Copy_to_tuple(_Tuple & target, _Tuple2 && source)
		{
			static_assert(std::tuple_size<_Tuple>::value >= std::tuple_size<_Tuple2>::value +_Idx, "index is out of range, _Idx must less than t1 size - t2 size");
			_Copy_to_tuple_impl<std::tuple_size<_Tuple2>::value>::copy<_Idx>(target, source);
		}
		template<size_t _Idx, class _Tuple, class _Tuple2>
		inline void _Move_to_tuple(_Tuple & target, _Tuple2 && source)
		{
			static_assert(std::tuple_size<_Tuple>::value >= std::tuple_size<_Tuple2>::value +_Idx, "index is out of range, _Idx must less than t1 size - t2 size");
			_Copy_to_tuple_impl<std::tuple_size<_Tuple2>::value>::move<_Idx>(target, source);
		}


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

#pragma warning(push)
#pragma warning(disable: 4100)	// TRANSITION, VSO#181496, unreferenced formal parameter
			template<class _Fx, class... _Rest>
			static inline decltype(auto) _Apply_impl(_Fx&& f, const _PrevArgs& args, _Rest&&... rest)
			{
				return std::forward<_Fx>(f)(args);
			}
			template<class _Fx, class... _Rest>
			static inline decltype(auto) _Apply_impl(_Fx&& f, _PrevArgs& args, _Rest&&... rest)
			{
				return std::forward<_Fx>(f)(std::forward<_PrevArgs>(args));
			}
			template<class _Fx, class... _Rest>
			static inline decltype(auto) _Apply_impl(_Fx&& f, _PrevArgs&& args, _Rest&&... rest)
			{
				return std::forward<_Fx>(f)(std::forward<_PrevArgs>(args));
			}
#pragma warning(pop)
			template<class _Fx, class... _PrevArgs2>
			static inline decltype(auto) Invoke(_Fx&& f, _PrevArgs2&&... args)
			{
				return _Apply_impl(std::forward<_Fx>(f), std::forward<_PrevArgs2>(args)...);
				//return std::forward<_Fx>(f)(std::forward<_PrevArgs2>(args)...);
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
			static inline decltype(auto) Invoke(_Fx & f)
			{
				return f();
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

#pragma warning(push)
#pragma warning(disable: 4100)	// TRANSITION, VSO#181496, unreferenced formal parameter
			template<class _Fx, class... _Rest>
			static inline decltype(auto) _Apply_impl2(_Fx&& f, _PrevArgs&&... args, _Rest&&... rest)
			{
				return std::forward<_Fx>(f)(std::forward<_PrevArgs>(args)...);
			}

			template<class _Fx, class _Tuple, size_t... _Idx>
			static inline decltype(auto) _Apply_impl(_Fx& f, _Tuple&& t, std::index_sequence<_Idx...>)
			{
				return _Apply_impl2(std::forward<_Fx>(f), std::get<_Idx>(std::forward<_Tuple>(t))...);
			}
#pragma warning(pop)

			template<class _Fx, class _Tuple>
			static inline decltype(auto) Invoke(_Fx&& f, _Tuple&& args)
			{
				//return _Apply_impl(std::forward<_Fx>(f), std::forward<_Tuple>(args), std::make_index_sequence<std::tuple_size<_Tuple>::value>{});
				return std::apply(std::forward<_Fx>(f), std::forward<_Tuple>(args));
			}
		};

		template<class _Fx, class _PrevArgs>
		inline void _Invoke_then(_Fx&& f, _PrevArgs&& val)
		{
			using value_type = std::remove_reference_t<_PrevArgs>;
			_Invoke_then_impl<value_type>::template Invoke<_Fx>(std::forward<_Fx>(f), std::forward<_PrevArgs>(val));
		}
		template<class _Rtype, class _PrevArgs>
		using unpack_tuple_fn_t = typename _Invoke_then_impl<_PrevArgs>::template unpack_tuple_function<_Rtype>::type;
		template<class _Rtype, class _PrevArgs>
		using unpack_tuple_node_t = typename _Invoke_then_impl<_PrevArgs>::template unpack_tuple_node<_Rtype>::type;
		template<class _Rtype, class _PrevArgs>
		using unpack_tuple_cbnode_t = typename _Invoke_then_impl<_PrevArgs>::template unpack_tuple_cbnode<_Rtype>::type;

		template<class _Ty>
		struct _Apply_function;

		template<class _Rx, class... _PrevArgs>
		struct _Apply_function<std::function<_Rx(_PrevArgs...)>>
		{
			using function_type = std::function<_Rx(_PrevArgs...)>;

			template<class _Fx, class... _Rest>
			static inline decltype(auto) _Apply_impl2(_Fx&& f, _PrevArgs&&... args, _Rest&&... rest)
			{
				return std::forward<_Fx>(f)(std::forward<_PrevArgs>(args)...);
			}

			template<class _Fx, class _Tuple, size_t... _Idx>
			static inline decltype(auto) _Apply_impl(_Fx&& f, _Tuple&& t, std::index_sequence<_Idx...>)
			{
				return _Apply_impl2(std::forward<_Fx>(f), std::get<_Idx>(std::forward<_Tuple>(t))...);
			}
			template<class _Fx, class _Ty1, class _Tuple, size_t... _Idx>
			static inline decltype(auto) _Apply_cat_impl(_Fx&& f, _Ty1&& t1, _Tuple&& t, std::index_sequence<_Idx...>)
			{
				return _Apply_impl2(std::forward<_Fx>(f), std::forward<_Ty1>(t1), std::get<_Idx>(std::forward<_Tuple>(t))...);
			}

			//-------------------std::tuple<>版本-----------------------------------------------------------------------
			template<class _Fx, class... _Tuple>
			static inline decltype(auto) _Apply(_Fx&& f, std::tuple<_Tuple...>&& t)
			{
				return _Apply_impl(std::forward<_Fx>(f), std::forward<std::tuple<_Tuple...>>(t), std::make_index_sequence<sizeof...(_Tuple)>{});
			}
			template<class _Fx, class... _Tuple>
			static inline decltype(auto) _Apply(_Fx&& f, std::tuple<_Tuple...>& t)
			{
				return _Apply_impl(std::forward<_Fx>(f), t, std::make_index_sequence<sizeof...(_Tuple)>{});
			}
			template<class _Fx, class... _Tuple>
			static inline decltype(auto) _Apply(_Fx&& f, const std::tuple<_Tuple...>& t)
			{
				return _Apply_impl(std::forward<_Fx>(f), t, std::make_index_sequence<sizeof...(_Tuple)>{});
			}

			template<class _Fx, class _Ty1, class... _Tuple>
			static inline decltype(auto) _Apply_cat(_Fx&& f, _Ty1&& t1, std::tuple<_Tuple...>&& t)
			{
				return _Apply_cat_impl(std::forward<_Fx>(f), std::forward<_Ty1>(t1), std::forward<std::tuple<_Tuple...>>(t), std::make_index_sequence<sizeof...(_Tuple)>{});
			}
			template<class _Fx, class _Ty1, class... _Tuple>
			static inline decltype(auto) _Apply_cat(_Fx&& f, _Ty1&& t1, std::tuple<_Tuple...>& t)
			{
				return _Apply_cat_impl(std::forward<_Fx>(f), std::forward<_Ty1>(t1), t, std::make_index_sequence<sizeof...(_Tuple)>{});
			}
			template<class _Fx, class _Ty1, class... _Tuple>
			static inline decltype(auto) _Apply_cat(_Fx&& f, _Ty1&& t1, const std::tuple<_Tuple...>& t)
			{
				return _Apply_cat_impl(std::forward<_Fx>(f), std::forward<_Ty1>(t1), t, std::make_index_sequence<sizeof...(_Tuple)>{});
			}
			//-------------------std::tuple<>版本-----------------------------------------------------------------------

			//-------------------变参版本-------------------------------------------------------------------------------
			template<class _Fx, class... _PrevArgs2>
			static inline decltype(auto) _Apply(_Fx&& f, _PrevArgs2&&... t)
			{
				return _Apply_impl2(std::forward<_Fx>(f), std::forward<_PrevArgs2>(t)...);
			}
			//-------------------变参版本-------------------------------------------------------------------------------
		};
		template<class _Fx, class... _PrevArgs2>
		inline decltype(auto) _Apply_then(_Fx&& f, _PrevArgs2&&... args)
		{
			using function_type = std::remove_reference_t<_Fx>;
			return _Apply_function<function_type>::template _Apply(std::forward<_Fx>(f), std::forward<_PrevArgs2>(args)...);
		}
		template<class _Fx, class _Fx2, class... _PrevArgs2>
		inline decltype(auto) _Apply_then2(_Fx2&& f, _PrevArgs2&&... args)
		{
			using function_type = std::remove_reference_t<_Fx>;
			return _Apply_function<function_type>::template _Apply(std::forward<_Fx2>(f), std::forward<_PrevArgs2>(args)...);
		}

		template<class _Tuple>
		struct package_tuple
		{
			using type = std::tuple<_Tuple>;
		};
		template<class... _Args>
		struct package_tuple<std::tuple<_Args...>>
		{
			using type = std::tuple<_Args...>;
		};
		template<class _Tuple>
		using package_tuple_t = typename package_tuple<_Tuple>::type;

		template<class _Ty>
		struct add_shared_ptr
		{
			using type = std::shared_ptr<_Ty>;
		};
		template<class _Ty>
		struct add_shared_ptr<std::shared_ptr<_Ty>>
		{
			using type = std::shared_ptr<_Ty>;
		};
		template<class _Ty>
		using add_shared_ptr_t = typename add_shared_ptr<_Ty>::type;

		template<class _Tuple>
		struct add_tuple_shared_ptr
		{
			using type = typename add_shared_ptr<_Tuple>::type;
		};
		template<class... _Args>
		struct add_tuple_shared_ptr<std::tuple<_Args...>>
		{
			using type = std::tuple<add_shared_ptr_t<_Args>...>;
		};
		template<class _Ty>
		using add_tuple_shared_ptr_t = typename add_tuple_shared_ptr<_Ty>::type;

		template<typename _Ty>
		struct __has_member_function_break_link
		{
		private:
			template<typename U> 
			static auto check_(int) -> decltype(std::declval<U>().break_link(), std::true_type());
			template<typename U> 
			static std::false_type check_(...); 
		public: 
			using type = decltype(check_<_Ty>(0));
			static const bool value = std::is_same_v<type, std::true_type>;
		};
		template<class _Ty>
		void _Break_link_impl(_Ty & node, std::true_type)
		{
			node.break_link();
		}
		template<class _Ty>
		void _Break_link_impl(_Ty & node, std::false_type)
		{
		}
		template<class _Ty>
		void _Break_link(_Ty & node)
		{
			_Break_link_impl(node, typename __has_member_function_break_link<_Ty>::type{});
		}
	}
}
