
#pragma once

namespace lib_shark_task
{
	struct executor
	{
		virtual ~executor() {}

		virtual void run_once() = 0;
	};
	using executor_sptr = std::shared_ptr<executor>;

	template<class _St>
	struct task_executor : public executor
	{
		using node_type = _St;
		using result_type = typename node_type::result_type;
		using args_tuple_type = typename node_type::args_tuple_type;

	private:
		std::shared_ptr<node_type>	_State;
	public:
		args_tuple_type				_Parames;

		template<class... _Rest>
		task_executor(const std::shared_ptr<node_type> & st, _Rest&&... args)
			: _State(st)
			, _Parames{std::forward<_Rest>(args)...}
		{
			assert(_State != nullptr);
		}

		task_executor(const task_executor & st) = default;
		task_executor(task_executor && st) = default;
		task_executor & operator = (const task_executor & st) = default;
		task_executor & operator = (task_executor && st) = default;

		virtual void run_once() override
		{
			assert(_State != nullptr);

			if (_State->invoke_thiz_tuple(std::move(_Parames)))
				_State->invoke_then_if();
		}
	};

	template<class _Ft>
	struct method_executor : public executor
	{
		using method_type = _Ft;
		using result_type = detail::result_of_t<method_type>;
		using args_tuple_type = detail::args_tuple_t<method_type>;

	private:
		bool						_Is_running = false;
		method_type					_Method;
	public:
		args_tuple_type				_Parames;

		template<class... _Rest>
		method_executor(const method_type & st, _Rest&&... args)
			: _Method(st)
			, _Parames{ std::forward<_Rest>(args)... }
		{
		}

		method_executor(const method_executor & st) = default;
		method_executor(method_executor && st) = default;
		method_executor & operator = (const method_executor & st) = default;
		method_executor & operator = (method_executor && st) = default;

		virtual void run_once() override
		{
			if (!_Is_running)
			{
				_Is_running = true;
				std::apply(_Method, std::move(_Parames));
			}
		}
	};


	namespace detail
	{
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


		template<class _Context, class _Ftype>
		struct _Set_method_ctx_helper
		{
			using method_type = _Ftype;
			using executor_type = method_executor<method_type>;
			using executor_type_ptr = std::shared_ptr<executor_type>;

			_Context *			_Ctx;
			executor_type_ptr	_Next;

			template<class... _Rest>
			_Set_method_ctx_helper(_Context * c, method_type && st, _Rest&&... args)
				: _Ctx(c)
				, _Next(std::make_shared<>(std::forward<method_type>(st), std::forward<_Rest>(args)...))
			{
			}

			template<class... _Args>
			auto operator ()(_Args&&... args) const
			{
				_Ctx->add(_Next);
			}
		};
	}
}
