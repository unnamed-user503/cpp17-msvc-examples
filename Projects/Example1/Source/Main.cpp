#include "Pch.hpp"
#include <iostream>

struct Lambda
{
    Lambda(int value) : m_Value(value)
    {
    }
    /*
    void operator()() const // not mutable lambda
    {
        std::cout << "Lambda1=" << m_Value << std::endl;
        m_Value++; // constなのでコンパイルエラー
    }
    */
    void operator()() // mutable lambda
    {
        std::cout << "Lambda1=" << m_Value << std::endl;
        m_Value++;
    }

    int m_Value;
};

int main()
{
    int value = 100;

    // (1) 関数オブジェクトで表現
    Lambda lambda1(value);

    lambda1();
    lambda1();

    // (2) ラムダ式は関数オブジェクトのSyntax Sugar
    auto lambda2 = [value]() mutable
    {
        std::cout << "Lambda2=" << value << std::endl;
        value++; // mutableがないとvalueは書き換えできない
    };

    lambda2();
    lambda2();

    return 0;
}
