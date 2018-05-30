#pragma once

#if !_HAS_CXX17
#include <utility> 

namespace std
{
#pragma warning(push)
#pragma warning(disable: 4100)	// TRANSITION, VSO#181496, unreferenced formal parameter
	template<class _Callable, class _Tuple, size_t... _Indices>
	constexpr decltype(auto) _Apply_impl(_Callable&& _Obj, _Tuple&& _Tpl, index_sequence<_Indices...>) 
	{	// invoke _Obj with the elements of _Tpl
		return (invoke(forward<_Callable>(_Obj), get<_Indices>(forward<_Tuple>(_Tpl))...));
	}
#pragma warning(pop)

	template<class _Callable, class _Tuple>
	constexpr decltype(auto) apply(_Callable&& _Obj, _Tuple&& _Tpl)
	{	// invoke _Obj with the elements of _Tpl
		return (_Apply_impl(forward<_Callable>(_Obj), forward<_Tuple>(_Tpl), make_index_sequence<tuple_size<decay_t<_Tuple>>::value>{}));
	}
}
#endif

namespace std
{
	template<typename _Tp, typename _Fx, size_t... Idx>
	inline void tuple_for_each_(_Tp && t, const _Fx& f, index_sequence<Idx...>)
	{
		(void)initializer_list<int>{ (f(get<Idx>(t)), 0)...};
	}
	//遍历tuple里的所有元素
	template<typename _Tp, typename _Fx>
	inline void for_each(_Tp && t, const _Fx& f)
	{
		using tuple_noref = remove_reference_t<_Tp>;
		using tuple_type = remove_cv_t<tuple_noref>;

		tuple_for_each_(forward<_Tp>(t), f,
			make_index_sequence<tuple_size<tuple_type>::value>{});
	}
}

template<class _Ty>
struct DEBUG_TYPE;