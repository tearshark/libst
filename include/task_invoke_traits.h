#pragma once

namespace lib_shark_task
{
	namespace detail
	{
		template<class _Ty>
		struct invoke_traits;

		//函数_Ret(_Args...)的特征
		template<class _Ret, class... _Args>
		struct native_invoke_traits
		{
			using result_type = _Ret;							//返回值
			using type = _Ret(_Args...);						//函数类型
			using args_tuple_type = std::tuple<_Args...>;		//将函数参作包装成tuple的类型
			using std_function_type = std::function<type>;		//对应的std::function<>类

			using funptr_type = std::true_type;					//是函数指针
			using memfun_type = std::false_type;				//不是成员函数
			using functor_type = std::false_type;				//不是仿函数

			enum { args_size = sizeof...(_Args) };				//函数参数个数

			//通过指定的索引获取函数参数的类型
			template<size_t _Idx>
			struct args_element
			{
				static_assert(_Idx < args_size, "index is out of range, index must less than sizeof _Args");
				using type = typename std::tuple_element<_Idx, args_tuple_type>::type;
			};

			//调用此类函数的辅助方法
			template<class _Fx, class... _Rest>
			static inline _Ret Invoke_(const _Fx & f, _Args&&... args, _Rest...)
			{
				return f(std::forward<_Args>(args)...);
			}
		};

		//普通函数.
		template<class _Ret, class... _Args>
		struct invoke_traits<_Ret(_Args...)> : public native_invoke_traits<_Ret, _Args...>
		{
		};
		
		//函数指针.
		template<class _Ret, class... _Args>
		struct invoke_traits<_Ret(*)(_Args...)> : public native_invoke_traits<_Ret, _Args...>
		{
		};

		//std::function<>
		template<class _Ret, class... _Args>
		struct invoke_traits<std::function<_Ret(_Args...)> > : public native_invoke_traits<_Ret, _Args...>
		{
		};
		template<class _Ret, class... _Args>
		struct invoke_traits<const std::function<_Ret(_Args...)> &> : public native_invoke_traits<_Ret, _Args...>
		{
		};
		template<class _Ret, class... _Args>
		struct invoke_traits<std::function<_Ret(_Args...)> &&> : public native_invoke_traits<_Ret, _Args...>
		{
		};


		//成员函数，普通版本
		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...)> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = callee_type & ;

			typedef _Ret(_Ctype::*pointer_type)(_Args...);
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		//成员函数，const版本
		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) &> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = callee_type & ;

			typedef _Ret(_Ctype::*pointer_type)(_Args...);
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) &&> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = callee_type && ;

			typedef _Ret(_Ctype::*pointer_type)(_Args...);
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) const> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = const callee_type &;

			typedef _Ret(_Ctype::*pointer_type)(_Args...) const;
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		//成员函数，volatile版本
		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) volatile> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = volatile callee_type &;

			typedef _Ret(_Ctype::*pointer_type)(_Args...) volatile;
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		//成员函数，const volatile版本
		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) const volatile> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = const volatile callee_type &;

			typedef _Ret(_Ctype::*pointer_type)(_Args...) const volatile;
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) const &> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = const callee_type &;

			typedef _Ret(_Ctype::*pointer_type)(_Args...);
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) volatile &> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = volatile callee_type & ;

			typedef _Ret(_Ctype::*pointer_type)(_Args...);
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		template<class _Ret, class _Ctype, class... _Args>
		struct invoke_traits<_Ret(_Ctype::*)(_Args...) volatile &&> : public native_invoke_traits<_Ret, _Args...>
		{
			using memfun_type = std::true_type;
			using callee_type = _Ctype;
			using this_args_type = volatile callee_type && ;

			typedef _Ret(_Ctype::*pointer_type)(_Args...);
			template<class... _Rest>
			static inline _Ret Invoke_(const pointer_type & f, this_args_type obj, _Args&&... args, _Rest...)
			{
				return (obj.*f)(std::forward<_Args>(args)...);
			}
		};

		//函数对象.
		template<class _Tobj>
		struct invoke_traits
		{
			using base_type = invoke_traits<decltype(&_Tobj::operator())>;

			using result_type = typename base_type::result_type;
			using type = typename base_type::type;
			using args_tuple_type = typename base_type::args_tuple_type;
			using std_function_type = typename base_type::std_function_type;

			using functor_type = std::true_type;
			using callee_type = typename base_type::callee_type;
			using this_args_type = typename base_type::this_args_type;

			enum { args_size = base_type::args_size };

			template<size_t _Idx>
			struct args_element
			{
				static_assert(_Idx < args_size, "index is out of range, index must less than sizeof _Args");
				using type = typename std::tuple_element<_Idx, args_tuple_type>::type;
			};

			template<class... _Rest>
			static inline decltype(auto) Invoke_(this_args_type obj, _Rest&&... args)
			{
				return base_type::template Invoke_(&_Tobj::operator(), obj, std::forward<_Rest>(args)...);
			}
		};

#ifdef _WIN32
#ifdef _MSC_VER
		template<class _Ret, class _Fx, class... _Types>
		using binder_type = std::_Binder<_Ret, _Fx, _Types...>;
#else
#error "Unknown Compiler on Windows" 
#endif
#elif __APPLE__ 
#error "Unknown platform" 
#if TARGET_IPHONE_SIMULATOR // iOS Simulator 
#error "Unknown platform" 
#elif TARGET_OS_IPHONE // iOS device 
#error "Unknown platform" 
#elif TARGET_OS_MAC // Other kinds of Mac OS 
#error "Unknown platform" 
#else 
#error "Unknown Apple platform" 
#endif 
#elif __ANDROID__ // android 
		template<class _Rp, class _Fp, class ..._BoundArgs>
		using binder_type = std::__bind_r<_Rp, _Fp, _BoundArgs...>;

		template<class _Fp, class ..._BoundArgs>
		using binder2_type = std::__bind<_Fp, _BoundArgs...>;

		//std::bind()
		template<class _Fx, class... _Types>
		struct invoke_traits<binder2_type<_Fx, _Types...> > : public invoke_traits<typename std::remove_reference<_Fx>::type>
		{
/*
			using bind_type = binder2_type<_Fx, _Types...>;

			using result_type = typename bind_type::result_type;
			using type = result_type(_Types...);
			using args_tuple_type = std::tuple<_Types...>;		//将函数参作包装成tuple的类型
			using std_function_type = std::function<type>;		//对应的std::function<>类

			using funptr_type = std::false_type;				//不是函数指针
			using memfun_type = std::false_type;				//不是成员函数
			using functor_type = std::true_type;				//是仿函数

			using callee_type = bind_type;
			using this_args_type = bind_type;

			enum { args_size = sizeof...(_Types) };				//函数参数个数

																//通过指定的索引获取函数参数的类型
			template<size_t _Idx>
			struct args_element
			{
				static_assert(_Idx < args_size, "index is out of range, index must less than sizeof _Args");
				using type = typename std::tuple_element<_Idx, args_tuple_type>::type;
			};

			//调用此类函数的辅助方法
			template<class _Fx2, class... _Rest>
			static inline  decltype(auto) Invoke_(const _Fx2 & f, _Types&&... args, _Rest...)
			{
				return f(std::forward<_Types>(args)...);
			}
*/
		};
		template<class _Fx, class... _Types>
		struct invoke_traits<binder2_type<_Fx, _Types...> &> : public invoke_traits<binder2_type<_Fx, _Types...> >
		{
		};
		template<class _Fx, class... _Types>
		struct invoke_traits<binder2_type<_Fx, _Types...> &&> : public invoke_traits<binder2_type<_Fx, _Types...> >
		{
		};

#elif __linux__ // linux 
#error "Unknown platform" 
#elif __unix__ // all unices not caught above // Unix 
#error "Unknown platform" 
#elif defined(_POSIX_VERSION) // POSIX 
#error "Unknown platform" 
#else 
#error "Unknown platform" 
#endif 

		//std::bind()
		template<class _Ret, class _Fx, class... _Types>
		struct invoke_traits<binder_type<_Ret, _Fx, _Types...> > : public invoke_traits<typename std::remove_reference<_Fx>::type>
		{
/*
			using bind_type = binder_type<_Ret, _Fx, _Types...>;

			using result_type = typename bind_type::result_type;
			using type = result_type(_Types...);
			using args_tuple_type = std::tuple<_Types...>;		//将函数参作包装成tuple的类型
			using std_function_type = std::function<type>;		//对应的std::function<>类

			using funptr_type = std::false_type;				//不是函数指针
			using memfun_type = std::false_type;				//不是成员函数
			using functor_type = std::true_type;				//是仿函数

			using callee_type = bind_type;
			using this_args_type = bind_type;

			enum { args_size = sizeof...(_Types) };				//函数参数个数
			
			//通过指定的索引获取函数参数的类型
			template<size_t _Idx>
			struct args_element
			{
				static_assert(_Idx < args_size, "index is out of range, index must less than sizeof _Args");
				using type = typename std::tuple_element<_Idx, args_tuple_type>::type;
			};

			//调用此类函数的辅助方法
			template<class _Fx2, class... _Rest>
			static inline  decltype(auto) Invoke_(const _Fx2 & f, _Types&&... args, _Rest...)
			{
				return f(std::forward<_Types>(args)...);
			}
*/
		};
		template<class _Ret, class _Fx, class... _Types>
		struct invoke_traits<binder_type<_Ret, _Fx, _Types...> &> : public invoke_traits<binder_type<_Ret, _Fx, _Types...> >
		{
		};
		template<class _Ret, class _Fx, class... _Types>
		struct invoke_traits<binder_type<_Ret, _Fx, _Types...> &&> : public invoke_traits<binder_type<_Ret, _Fx, _Types...> >
		{
		};
	}
	
}
