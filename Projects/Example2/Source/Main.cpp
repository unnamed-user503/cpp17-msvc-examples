#include "Pch.hpp"
#include <iostream>
#include <list>
#include <vector>

int main()
{
    std::list<int> values;

    auto position1 = values.insert(values.end(), 1);
    auto position2 = values.insert(values.end(), 2);
    auto position3 = values.insert(values.end(), 3);
    auto position4 = values.insert(values.end(), 4);
    auto position5 = values.insert(values.end(), 5);
    auto position6 = values.insert(values.end(), 6);
    auto position7 = values.insert(values.end(), 7);
    auto position8 = values.insert(values.end(), 8);
    auto position9 = values.insert(values.end(), 9);

    values.erase(position5); // std::vectorの場合要素の移動が発生していると古いiteratorは無効な場所を指している

    for (auto& iterator : values)
    {
        std::cout << "value = " << iterator << std::endl; // 5は表示されない
    }

    return 0;
}
