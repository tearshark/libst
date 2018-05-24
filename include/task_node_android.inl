
#pragma once

namespace lib_shark_task
{
#ifdef ANDROID
	template<class _Rtype>
	struct __assoc_state_hack : public std::__assoc_state<_Rtype>
	{
		std::mutex & _Mtx()
		{
			return this->__mut_;
		}
		_Rtype _Get_value()
		{
			std::unique_lock<std::mutex> __lk(this->__mut_);

			this->__state_ |= std::__assoc_state<_Rtype>::__future_attached;
			return this->__value_;
		}
		_Rtype & _Peek_value()
		{
			return this->__value_;
		}
	};

	//实现存取任务节点的结果。
	//存取结果值
	//返回对应的future对象
	//提供mutex实例
	template<class _Rtype>
	struct node_result_ : public std::enable_shared_from_this<node_result_<_Rtype>>
							, public task_set_exception
	{
	protected:
		__assoc_state_hack<_Rtype>*		_State;						//通过clang的future内部的__assoc_state来实现future功能。今后考虑自己做future
		task_set_exception_agent_sptr	_Exception;					//传递异常的代理接口
		std::atomic<bool>				_Ready{ false };			//结果是否已经准备好了，线程安全

	public:
		node_result_(const task_set_exception_agent_sptr & exp)
			: _State(new __assoc_state_hack<_Rtype>)
			, _Exception(exp)
		{
		}
		~node_result_()
		{
			if (_State)
				_State->__release_shared();
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
			return std::__make_deferred_assoc_state<_Rtype>([this]()
			{
				return _State;
			});
			//return (std::future<_Rtype>(_State));
		}

		//结果是否已经准备好了，线程安全
		inline bool is_ready() const
		{
			return _Ready;
		}

		//是否已经调用过get_future/_Set_then_if/_Get_value之一
		inline bool is_retrieved() const
		{
			return _State->__has_future_attached();
		}
	private:
		//task_set_exception 接口的实现
		virtual void _Set_exception(std::exception_ptr && val) override
		{
			_State->set_exception(std::forward<std::exception_ptr>(val));
		}
	protected:
		inline std::__assoc_state<_Rtype> * _Ptr() const
		{
			return _State;
		}
		//设定调用过get_future/_Set_then_if/_Get_value之一
		void _Set_retrieved()
		{
			if (_State->__has_future_attached())
				throw std::future_error(std::make_error_code(std::future_errc::no_state));
			_State->__set_future_attached();
		}
		//获取存的值，只能调用一次
		inline _Rtype _Get_value()
		{
			return _State->_Get_value();
		}
		inline _Rtype & _Peek_value()
		{
			return _State->_Peek_value();
		}
		//设置存的值
		template<class _Ty2>
		inline void _Set_value(_Ty2 && val)
		{
			_State->set_value(std::forward<_Ty2>(val));
		}
		//设置异常
		inline void _Set_Agent_exception(std::exception_ptr && val)
		{
			_Exception->_Set_exception(std::forward<std::exception_ptr>(val));
		}
		//对外提供的mutex实例
		inline std::mutex & _Mtx() const
		{
			return _State->_Mtx();
		}
	};

#endif
}
