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
}
#endif
