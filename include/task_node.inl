
#pragma once

namespace lib_shark_task
{
	//实现存取任务节点的结果。
	//存取结果值
	//返回对应的future对象
	//提供mutex实例
	template<class _Rtype>
	struct node_result_ : public std::enable_shared_from_this<node_result_<_Rtype>>
							, public task_set_exception
	{
	protected:
		std::_State_manager<_Rtype>		_State;						//通过VS的future内部的_State_manager来实现future功能。今后考虑自己做future
		task_set_exception_agent_sptr	_Exception;					//传递异常的代理接口
		std::atomic<bool>				_Ready = false;				//结果是否已经准备好了，线程安全
		bool							_Future_retrieved = false;	//是否已经调用过get_future()，或者已经调用过_Set_then_if

	public:
		node_result_(const task_set_exception_agent_sptr & exp)
			: _State(new std::_Associated_state<_Rtype>, true)
			, _Exception(exp)
		{
		}
		node_result_(node_result_ && _Right) = default;
		node_result_ & operator = (node_result_ && _Right) = default;
		node_result_(const node_result_ & _Right) = delete;
		node_result_ & operator = (const node_result_ & _Right) = delete;
		
		//此函数只应该被调用一次。
		//并且外部没有调用过 _Set_then_if()
		//内部不能调用过_Get_value()
		inline std::future<_Rtype> get_future()
		{
			assert(!is_retrieved());

			_Set_retrieved();
			return (std::future<_Rtype>(_State, std::_Nil()));
		}

		//结果是否已经准备好了，线程安全
		inline bool is_ready() const
		{
			return _Ready;
		}

		//是否已经调用过get_future/_Set_then_if/_Get_value之一
		inline bool is_retrieved() const
		{
			std::unique_lock<std::mutex> _Lock(_Mtx());

			return _Ptr()->_Already_retrieved() || _Future_retrieved;
		}
	private:
		//task_set_exception 接口的实现
		virtual void _Set_exception(std::exception_ptr && val) override
		{
			_Ptr()->_Set_exception(std::forward<std::exception_ptr>(val), false);
		}
	protected:
		inline std::_Associated_state<_Rtype> * _Ptr() const
		{
			return _State._Ptr();
		}
		//设定调用过get_future/_Set_then_if/_Get_value之一
		void _Set_retrieved()
		{
			std::unique_lock<std::mutex> _Lock(_Mtx());

			if (!_State.valid())
				std::_Throw_future_error(
					std::make_error_code(std::future_errc::no_state));
			if (_Future_retrieved)
				std::_Throw_future_error(
					std::make_error_code(std::future_errc::future_already_retrieved));
			_Future_retrieved = true;
		}
		//获取存的值，只能调用一次
		inline _Rtype && _Get_value()
		{
			return std::move(_Ptr()->_Get_value(true));
		}
		inline _Rtype & _Peek_value()
		{
			return _Ptr()->_Result;
		}
		//设置存的值
		template<class _Ty2>
		inline void _Set_value(_Ty2 && val)
		{
			_Ptr()->_Set_value(std::forward<_Ty2>(val), false);
		}
		//设置异常
		inline void _Set_Agent_exception(std::exception_ptr && val)
		{
			_Exception->_Set_exception(std::current_exception());
		}
		//对外提供的mutex实例
		inline std::mutex & _Mtx() const
		{
			return _Ptr()->_Mtx;
		}
	};

	//在task_state_result基础上，提供操作执行当前任务节点，执行下一个任务节点的函数
	template<class _Rtype, class _Taskf, class _Thenf = std::function<void()>>
	struct node_impl : public node_result_<_Rtype>
	{
		using task_function = std::remove_reference_t<_Taskf>;
		using then_function = std::remove_reference_t<_Thenf>;
	protected:
		task_function			_Thiz;			//执行当前任务节点
		then_function			_Then;			//执行下一个任务节点
	public:
		node_impl(task_function && fn, const task_set_exception_agent_sptr & exp)
			: node_result_(exp)
			, _Thiz(std::forward<task_function>(fn))
		{
		}
		node_impl(const task_function & fn, const task_set_exception_agent_sptr & exp)
			: node_result_(exp)
			: _Thiz(fn)
		{
		}
		node_impl(const task_set_exception_agent_sptr & exp)
			: node_result_(exp)
		{
		}

		node_impl(node_impl && _Right) = default;
		node_impl & operator = (node_impl && _Right) = default;
		node_impl(const node_impl & _Right) = delete;
		node_impl & operator = (const node_impl & _Right) = delete;
	protected:
		//取执行当前任务节点的函数，只能取一次。线程安全
		inline task_function _Move_thiz()
		{
			std::unique_lock<std::mutex> _Lock(_Mtx());
			return std::move(_Thiz);			//强迫只能调用一次
		}
		//取执行下一个任务节点的函数，只能取一次。线程安全
		inline then_function _Move_then()
		{
			std::unique_lock<std::mutex> _Lock(_Mtx());
			return std::move(_Then);			//强迫只能调用一次
		}
	};

	//任务节点
	//_Rtype是本节点的返回值类型
	//_PrevArgs...是上一个任务节点的返回值(如果上一个节点返回值是std::tuple<>，则_PrevArgs是将tuple接包后的参数列表)
	template<class _Rtype, class... _PrevArgs>
	struct task_node : public node_impl<std::remove_reference_t<_Rtype>,
						std::function<std::remove_reference_t<_Rtype>(_PrevArgs...)>, 
						detail::unpack_tuple_fn_t<void, std::remove_reference_t<_Rtype>> >
	{
		using result_type = std::remove_reference_t<_Rtype>;			//本节点的结果的类型
		using result_tuple = detail::package_tuple_t<result_type>;		//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<_PrevArgs...>;				//本节点的入参打包成tuple<>后的类型

		using node_impl::node_impl;

		template<class... _PrevArgs2>
		bool invoke_thiz(_PrevArgs2&&... args)
		{
			static_assert(sizeof...(_PrevArgs2) >= typename std::tuple_size<args_tuple_type>::value, "");

			try
			{
				task_function fn = _Move_thiz();
				_Set_value(detail::_Apply_then(fn, std::forward<_PrevArgs2>(args)...));
				//_Set_value(fn(std::forward<_PrevArgs2>(args)...));
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
				task_function fn = _Move_thiz();
				_Set_value(detail::_Apply_then(fn, std::forward<_PrevTuple>(args)));
				//_Set_value(std::apply(fn, std::forward<_PrevTuple>(args)));
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

	template<class... _PrevArgs>
	struct task_node<void, _PrevArgs...> : public node_impl<int, std::function<void(_PrevArgs...)>>
	{
		using result_type = void;									//本节点的结果的类型
		using result_tuple = std::tuple<>;							//本节点的结果打包成tuple<>后的类型
		using args_tuple_type = std::tuple<_PrevArgs...>;			//本节点的入参打包成tuple<>后的类型

		using node_impl::node_impl;

		template<class... _PrevArgs2>
		bool invoke_thiz(_PrevArgs2&&... args)
		{
			static_assert(sizeof...(_PrevArgs2) >= typename std::tuple_size<args_tuple_type>::value, "");

			try
			{
				task_function fn = _Move_thiz();
				detail::_Apply_then(fn, std::forward<_PrevArgs2>(args)...);
				//fn(std::forward<_PrevArgs2>(args)...);
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
				task_function fn = _Move_thiz();
				detail::_Apply_then(fn, std::forward<_PrevTuple>(args));
				//std::apply(fn, std::forward<_PrevTuple>(args));
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
			if (!_Ready)
				return;

			then_function fn = _Move_then();
			if (!fn)
				return;

			try
			{
				fn();
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
					fn();
				}
				catch (...)
				{
					_Set_Agent_exception(std::current_exception());
				}
			}
			else
			{
				std::unique_lock<std::mutex> _Lock(_Mtx());
				_Then = std::forward<_NextFx>(fn);
			}
		}
	};

	template<class _Rtype, class... _PrevArgs>
	struct task_node<_Rtype, std::tuple<_PrevArgs...>> : public task_node<_Rtype, _PrevArgs...>
	{
		using task_node<_Rtype, _PrevArgs...>::task_node;
	};
}
