
#pragma once

namespace lib_shark_task
{
#if defined(_MSC_VER)
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
		inline void _Set_value()
		{
			_Ptr()->_Set_value(false);
		}
		//设置异常
		inline void _Set_Agent_exception(std::exception_ptr && val)
		{
			_Exception->_Set_exception(std::forward<std::exception_ptr>(val));
		}
		//对外提供的mutex实例
		inline std::mutex & _Mtx() const
		{
			return _Ptr()->_Mtx;
		}
	};
#else
	#error "Unknown Compiler on Windows" 
#endif
}
