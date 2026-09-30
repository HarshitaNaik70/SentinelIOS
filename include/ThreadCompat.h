#ifndef THREAD_COMPAT_H
#define THREAD_COMPAT_H

#include <chrono>

#if defined(_WIN32) && !defined(_GLIBCXX_HAS_GTHREADS)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <functional>

#ifdef ERROR
#undef ERROR
#endif

namespace std {
    // Lightweight Mutex for MinGW Windows fallback
    class mutex {
    private:
        CRITICAL_SECTION cs;
    public:
        mutex() { InitializeCriticalSection(&cs); }
        ~mutex() { DeleteCriticalSection(&cs); }
        mutex(const mutex&) = delete;
        mutex& operator=(const mutex&) = delete;
        void lock() { EnterCriticalSection(&cs); }
        void unlock() { LeaveCriticalSection(&cs); }
        bool try_lock() { return TryEnterCriticalSection(&cs) != 0; }
    };

    template<typename T>
    class lock_guard {
    private:
        T& m_mutex;
    public:
        explicit lock_guard(T& m) : m_mutex(m) { m_mutex.lock(); }
        ~lock_guard() { m_mutex.unlock(); }
        lock_guard(const lock_guard&) = delete;
        lock_guard& operator=(const lock_guard&) = delete;
    };

    class thread {
    private:
        HANDLE m_handle{NULL};
        static unsigned __stdcall runner(void* arg) {
            auto* func = static_cast<std::function<void()>*>(arg);
            if (func) {
                (*func)();
                delete func;
            }
            return 0;
        }
    public:
        thread() = default;
        template<typename Function, typename... Args>
        explicit thread(Function&& f, Args&&... args) {
            auto bound = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
            auto* func_ptr = new std::function<void()>(bound);
            uintptr_t handle = _beginthreadex(NULL, 0, runner, func_ptr, 0, NULL);
            m_handle = reinterpret_cast<HANDLE>(handle);
        }
        ~thread() {
            if (joinable()) {
                CloseHandle(m_handle);
            }
        }
        thread(thread&& other) noexcept : m_handle(other.m_handle) {
            other.m_handle = NULL;
        }
        thread& operator=(thread&& other) noexcept {
            if (this != &other) {
                if (joinable()) CloseHandle(m_handle);
                m_handle = other.m_handle;
                other.m_handle = NULL;
            }
            return *this;
        }
        thread(const thread&) = delete;
        thread& operator=(const thread&) = delete;

        bool joinable() const { return m_handle != NULL; }
        void join() {
            if (joinable()) {
                WaitForSingleObject(m_handle, INFINITE);
                CloseHandle(m_handle);
                m_handle = NULL;
            }
        }
        void detach() {
            if (joinable()) {
                CloseHandle(m_handle);
                m_handle = NULL;
            }
        }
    };

    namespace this_thread {
        inline void sleep_for(const std::chrono::milliseconds& ms) {
            Sleep(static_cast<DWORD>(ms.count()));
        }
        template<typename Rep, typename Period>
        inline void sleep_for(const std::chrono::duration<Rep, Period>& d) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(d);
            Sleep(static_cast<DWORD>(ms.count()));
        }
    }
}
#else
#include <mutex>
#include <thread>
#endif

#endif // THREAD_COMPAT_H
