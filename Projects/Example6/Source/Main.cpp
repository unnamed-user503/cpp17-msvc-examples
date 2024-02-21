#include <iostream>
#include <thread>
#include <mutex>
#include <Windows.h>

int main()
{
    std::mutex ConsoleWriteLockMutex;

    HANDLE waitHandle = ::CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);

    auto thread1 = std::thread([&]()
    {
        {
            std::lock_guard guard(ConsoleWriteLockMutex);
            std::cout << "Thread1 started." << std::endl;
        }

        ::WaitForSingleObject(waitHandle, INFINITE);

        {
            std::lock_guard guard(ConsoleWriteLockMutex);
            std::cout << "Thread1 stopped." << std::endl;
        }
    });

    auto thread2 = std::thread([&]()
    {
        {
            std::lock_guard guard(ConsoleWriteLockMutex);
            std::cout << "Thread1 started." << std::endl;
        }

        ::WaitForSingleObject(waitHandle, INFINITE);

        {
            std::lock_guard guard(ConsoleWriteLockMutex);
            std::cout << "Thread1 stopped." << std::endl;
        }
    });

    auto thread3 = std::thread([&]()
    {
        {
            std::lock_guard guard(ConsoleWriteLockMutex);
            std::cout << "Thread1 started." << std::endl;
        }

        ::WaitForSingleObject(waitHandle, INFINITE);

        {
            std::lock_guard guard(ConsoleWriteLockMutex);
            std::cout << "Thread1 stopped." << std::endl;
        }
    });

    ::Sleep(1000);

    {
        std::lock_guard guard(ConsoleWriteLockMutex);
        std::cout << "SetEvent before" << std::endl;
    }
    ::SetEvent(waitHandle);
    {
        std::lock_guard guard(ConsoleWriteLockMutex);
        std::cout << "SetEvent after" << std::endl;
    }

    thread3.join();
    thread2.join();
    thread1.join();

    ::CloseHandle(waitHandle);
}
