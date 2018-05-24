#pragma once

#include <assert.h>
#include <chrono>

#if defined(_WIN32)
#include <windows.h>

#elif defined(ANDROID)
#include <semaphore.h>

#elif defined(__MACH__)
#include <mach/mach.h>

#elif defined(__unix__)
#include <semaphore.h>

#endif

namespace std
{

#if defined(_WIN32)

	class shared_semaphore
	{
	protected:
		void* m_hSema;
	public:
		operator bool() const
		{
			return m_hSema != nullptr;
		}

		bool wait()
		{
			return WaitForSingleObject(m_hSema, INFINITE) == WAIT_OBJECT_0;
		}

		bool try_wait()
		{
			return WaitForSingleObject(m_hSema, 0) == WAIT_OBJECT_0;
		}

		bool timed_wait(std::chrono::microseconds dt)
		{
			return WaitForSingleObject(m_hSema, (unsigned long)(dt.count() / 1000)) == WAIT_OBJECT_0;
		}

		void signal(int count = 1)
		{
			ReleaseSemaphore(m_hSema, count, nullptr);
		}

		void * native_handle() const
		{
			return m_hSema;
		}
	};

	class semaphore : public shared_semaphore
	{
	private:
		semaphore(const semaphore& other) = delete;
		semaphore& operator=(const semaphore& other) = delete;
	public:
		semaphore(int initialCount = 0, int maxLong = (std::numeric_limits<int>::max)())
		{
			assert(initialCount >= 0);
			//const long maxLong = 0x7fffffff;
			m_hSema = CreateSemaphoreW(nullptr, initialCount, maxLong, nullptr);
		}
		semaphore(semaphore && _Right)
		{
			m_hSema = _Right.m_hSema;
			_Right.m_hSema = nullptr;
		}
		semaphore & operator = (semaphore && _Right)
		{
			if (this != &_Right)
			{
				m_hSema = _Right.m_hSema;
				_Right.m_hSema = nullptr;
			}
			return *this;
		}
		void swap(semaphore & _Right)
		{
			void* hSema = m_hSema;
			m_hSema = _Right.m_hSema;
			_Right.m_hSema = hSema;
		}

		~semaphore()
		{
			if (m_hSema)
				CloseHandle(m_hSema);
		}
	};

#elif defined(__MACH__)

	//---------------------------------------------------------
	// Semaphore (Apple iOS and OSX)
	// Can't use POSIX semaphores due to http://lists.apple.com/archives/darwin-kernel/2009/Apr/msg00010.html
	//---------------------------------------------------------
	class shared_semaphore
	{
	protected:
		semaphore_t m_hSema;
	public:
		operator bool() const
		{
			return m_hSema != nullptr;
		}

		void wait()
		{
			semaphore_wait(m_hSema);
		}

		bool try_wait()
		{
			return timed_wait(0);
		}

		bool timed_wait(std::chrono::microseconds dt)
		{
			mach_timespec_t ts;

			ts.tv_sec = static_cast<unsigned int>(dt.count() / 1000000);
			ts.tv_nsec = (dt.count() % 1000000) * 1000;

			// added in OSX 10.10: https://developer.apple.com/library/prerelease/mac/documentation/General/Reference/APIDiffsMacOSX10_10SeedDiff/modules/Darwin.html
			kern_return_t rc = semaphore_timedwait(m_hSema, ts);
			return rc != KERN_OPERATION_TIMED_OUT && rc != KERN_ABORTED;
		}

		void signal()
		{
			semaphore_signal(m_hSema);
		}

		void signal(int count)
		{
			while (count-- > 0)
			{
				semaphore_signal(m_hSema);
			}
		}

		semaphore_t native_handle() const
		{
			return m_hSema;
		}
	};

	class semaphore : public shared_semaphore
	{
	private:
		semaphore(const semaphore& other) = delete;
		semaphore& operator=(const semaphore& other) = delete;
	public:
		semaphore(int initialCount = 0)
		{
			assert(initialCount >= 0);
			semaphore_create(mach_task_self(), &m_hSema, SYNC_POLICY_FIFO, initialCount);
		}
		semaphore(semaphore && _Right)
			: m_hSema(_Right.m_hSema)
		{
			_Right.m_hSema = nullptr;
		}
		semaphore & operator = (semaphore && _Right)
		{
			if (this != &_Right)
			{
				m_hSema = _Right.m_hSema;
				_Right.m_hSema = nullptr;
			}
			return *this;
		}
		void swap(semaphore & _Right)
		{
			semaphore_t* hSema = m_hSema;
			m_hSema = _Right.m_hSema;
			_Right.m_hSema = hSema;
		}

		~semaphore()
		{
			semaphore_destroy(mach_task_self(), m_hSema);
		}
	};

#elif defined(__unix__) || defined(ANDROID)

	//---------------------------------------------------------
	// Semaphore (POSIX, Linux)
	//---------------------------------------------------------

	class shared_semaphore
	{
	protected:
		sem_t m_hSema;
	public:
		operator bool() const
		{
			return m_hSema != nullptr;
		}

		void wait()
		{
			// http://stackoverflow.com/questions/2013181/gdb-causes-sem-wait-to-fail-with-eintr-error
			int rc;
			do {
				rc = sem_wait(&m_hSema);
			} while (rc == -1 && errno == EINTR);
		}

		bool try_wait()
		{
			int rc;
			do {
				rc = sem_trywait(&m_hSema);
			} while (rc == -1 && errno == EINTR);
			return !(rc == -1 && errno == EAGAIN);
		}

		bool timed_wait(std::chrono::microseconds dt)
		{
			struct timespec ts;
			const int usecs_in_1_sec = 1000000;
			const int nsecs_in_1_sec = 1000000000;
			clock_gettime(CLOCK_REALTIME, &ts);

			ts.tv_sec += dt.count() / usecs_in_1_sec;
			ts.tv_nsec += (dt.count() % usecs_in_1_sec) * 1000;

			// sem_timedwait bombs if you have more than 1e9 in tv_nsec
			// so we have to clean things up before passing it in
			if (ts.tv_nsec >= nsecs_in_1_sec)
			{
				ts.tv_nsec -= nsecs_in_1_sec;
				++ts.tv_sec;
			}

			int rc;
			do
			{
				rc = sem_timedwait(&m_hSema, &ts);
			} while (rc == -1 && errno == EINTR);
			return !(rc == -1 && errno == ETIMEDOUT);
		}

		void signal()
		{
			sem_post(&m_hSema);
		}

		void signal(int count)
		{
			while (count-- > 0)
			{
				sem_post(&m_hSema);
			}
		}

		sem_t native_handle() const
		{
			return m_hSema;
		}
	};

	class semaphore : public shared_semaphore
	{
	private:
		semaphore(const semaphore& other) = delete;
		semaphore& operator=(const semaphore& other) = delete;
	public:
		semaphore(int initialCount = 0)
		{
			assert(initialCount >= 0);
			sem_init(&m_hSema, 0, initialCount);
		}
		semaphore(semaphore && _Right)
			: m_hSema(_Right.m_hSema)
		{
			_Right.m_hSema = nullptr;
		}
		semaphore & operator = (semaphore && _Right)
		{
			if (this != &_Right)
			{
				m_hSema = _Right.m_hSema;
				_Right.m_hSema = nullptr;
			}
			return *this;
		}
		void swap(semaphore & _Right)
		{
			sem_t hSema = m_hSema;
			m_hSema = _Right.m_hSema;
			_Right.m_hSema = hSema;
		}

		~semaphore()
		{
			sem_destroy(&m_hSema);
		}
	};

#else
#error Unsupported platform! (No semaphore wrapper available)
#endif

	inline void swap(semaphore & _Left, semaphore & _Right)
	{
		_Left.swap(_Right);
	}
}
