#include <iostream>
#include <mutex>
#include <Windows.h>

struct UserDefineContext
{
    std::string Name;
};

std::mutex g_ConsoleWriteLock;

void CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE pInstance, PVOID pContext, PTP_WORK pWork)
{
    auto threadId = ::GetCurrentThreadId();

    for (int i = 0; i < 10; i++)
    {
        std::lock_guard guard{ g_ConsoleWriteLock };
        std::cout << "ThreadId: " << threadId << ", i = " << i << std::endl;
    }
}

void CALLBACK CleanupGroupCancelCallback(void* pObjectContext, void* pCleanupContext)
{
    {
        std::lock_guard guard{ g_ConsoleWriteLock };
        std::cout << "pObjectContext=" << reinterpret_cast<std::uint64_t>(pObjectContext) << ", pCleanupContext=" << reinterpret_cast<std::uint64_t>(pCleanupContext) << std::endl;
    }

    UserDefineContext* pContext = reinterpret_cast<UserDefineContext*>(pObjectContext);

    if (pContext)
    {
        std::lock_guard guard{ g_ConsoleWriteLock };
        std::cout << "Name=" << pContext->Name << std::endl;
    }
}

int main()
{
    UserDefineContext value1{ "User#01" };
    UserDefineContext value2{ "User#02" };

    TP_CALLBACK_ENVIRON CallbackEnviron{};

    // コールバック環境を初期化します。
    ::InitializeThreadpoolEnvironment(&CallbackEnviron);

    // アプリケーションが 1 つ以上のスレッド プール コールバックを追跡するために使用できるクリーンアップ グループを作成します。
    PTP_CLEANUP_GROUP pCleanupGroup = ::CreateThreadpoolCleanupGroup();

    // 指定したクリーンアップ グループを指定したコールバック環境に関連付けます。
    ::SetThreadpoolCallbackCleanupGroup(&CallbackEnviron, pCleanupGroup, CleanupGroupCancelCallback);

    // CreateThreadpool 関数は、既定のスレッド プールから、プライベート プール オブジェクトを完全に独立した作成します。
    PTP_POOL pPool = ::CreateThreadpool(nullptr);

    ::SetThreadpoolThreadMinimum(pPool, 1); // 指定したスレッド プールがコールバックの処理に使用できるようにする必要があるスレッドの最小数を設定します。
    ::SetThreadpoolThreadMaximum(pPool, 3); // 指定したスレッド プールがコールバックを処理するために割り当てることができるスレッドの最大数を設定します。

    // コールバックの生成時に使用するスレッド プールを設定します。
    ::SetThreadpoolCallbackPool(&CallbackEnviron, pPool);

    PTP_WORK pWork1 = ::CreateThreadpoolWork(WorkCallback, &value1, &CallbackEnviron);
    PTP_WORK pWork2 = ::CreateThreadpoolWork(WorkCallback, &value2, &CallbackEnviron);

    ::SubmitThreadpoolWork(pWork1);
    ::SubmitThreadpoolWork(pWork2);

    //::WaitForThreadpoolWorkCallbacks(pWork1, FALSE);
    //::WaitForThreadpoolWorkCallbacks(pWork2, FALSE);

    // 指定したクリーンアップ グループのメンバーを解放し、すべてのコールバック関数が完了するまで待機し、必要に応じて未処理のコールバック関数を取り消します。
    ::CloseThreadpoolCleanupGroupMembers(pCleanupGroup, FALSE, nullptr);

    // 指定したクリーンアップ グループを閉じます。
    ::CloseThreadpoolCleanupGroup(pCleanupGroup);

    // 指定したスレッド プールを閉じます。
    ::CloseThreadpool(pPool);

    // 指定したコールバック環境を削除します。 新しいスレッド プール オブジェクトを作成するためにコールバック環境が不要になった場合は、この関数を呼び出します。
    ::DestroyThreadpoolEnvironment(&CallbackEnviron);

    return 0;
}
