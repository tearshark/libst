#pragma once

#if !_HAS_CXX17

namespace std
{
#pragma warning(push)
#pragma warning(disable: 4100)	// TRANSITION, VSO#181496, unreferenced formal parameter
	template<class _Callable, class _Tuple, size_t... _Indices>
	constexpr decltype(auto) _Apply_impl(_Callable&& _Obj, _Tuple&& _Tpl, index_sequence<_Indices...>)
	{	// invoke _Obj with the elements of _Tpl
		return (_STD invoke(_STD forward<_Callable>(_Obj), _STD get<_Indices>(_STD forward<_Tuple>(_Tpl))...));
	}
#pragma warning(pop)

	template<class _Callable, class _Tuple>
	constexpr decltype(auto) apply(_Callable&& _Obj, _Tuple&& _Tpl)
	{	// invoke _Obj with the elements of _Tpl
		return (_Apply_impl(_STD forward<_Callable>(_Obj), _STD forward<_Tuple>(_Tpl),
			make_index_sequence<tuple_size_v<decay_t<_Tuple>>>{}));
	}

	template<typename _Tp, typename _Fx, size_t... Idx>
	inline void tuple_for_each_(_Tp && t, const _Fx& f, std::index_sequence<Idx...>)
	{
#if _MSC_FULL_VER < 191225830
		(void)std::initializer_list<int>{ (f(std::get<Idx>(t)), 0)...};
#else
		(f(std::get<Idx>(t)), ...);	//need vs2017 ver 15.5.0
#endif
	}
	//遍历tuple里的所有元素
	template<typename _Tp, typename _Fx>
	inline void for_each(_Tp && t, const _Fx& f)
	{
		using tuple_noref = std::remove_reference_t<_Tp>;
		using tuple_type = std::remove_cv_t<tuple_noref>;

		tuple_for_each_(std::forward<_Tp>(t), f,
			std::make_index_sequence<std::tuple_size<tuple_type>::value>{});
	}
}
#endif

template<class _Ty>
struct DEBUG_TYPE;