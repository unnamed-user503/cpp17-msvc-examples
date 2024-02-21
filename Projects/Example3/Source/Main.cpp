#include <iostream>
#include <list>
#include <functional>

template<typename... Args> class Delegate
{

    using Concrete = std::function<void(Args...)>;

public:

    void Bind(Concrete const& concrete)
    {
        m_Concrete = concrete;
    }

    void Unbind()
    {
        m_Concrete = nullptr;
    }

    void Execute(Args... args)
    {
        if (m_Concrete)
        {
            m_Concrete(std::forward<Args>(args)...);
        }
    }

private:

    Concrete m_Concrete{ nullptr };

};

int main()
{
    Delegate<std::string_view, int> delegate;

    // 処理を委譲する
    delegate.Bind([](std::string_view message, int value)
    {
        std::cout << message << ", Value=" << value << std::endl;
    });

    // 委譲された処理を実行する
    delegate.Execute("Hello", 100);

    // Unbindすると以降の実行は無効になる
    delegate.Unbind();
    delegate.Execute("World", -100);

    return 0;
}
