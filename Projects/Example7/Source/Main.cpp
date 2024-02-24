#include <iostream>
#include <thread>
#include <Windows.h>

#pragma comment(lib, "Synchronization.lib")

int main()
{
    std::uint32_t variable  = 0;
    std::uint32_t condition = 0;
    std::uint32_t verify    = 0;

    std::thread thread([&variable]()
    {
        ::Sleep(5000);
        variable = 100;
        ::WakeByAddressAll(&variable);
    });

    while (verify == condition)
    {
        std::cout << "WaitOnAddress start." << std::endl;

        if (::WaitOnAddress(&variable, &condition, sizeof(decltype(variable)), 3000))
        {
            std::cout << "WaitOnAddress wakeup. (variable=" << variable << ")" << std::endl;
            verify = variable;
        }
        else if (::GetLastError() == ERROR_TIMEOUT)
        {
            std::cout << "WaitOnAddress timeout. (variable=" << variable << ")" << std::endl;
            verify = variable;
        }
        else
        {
            break; // Error.
        }

        std::cout << std::endl;
    }

    thread.join();
    return 0;
}
