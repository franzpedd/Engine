#pragma once

#include <functional>

namespace Cosmos
{
    // replace implementation, changes any old value with the newer one 
    template<class ForwardIt, class T>
    void replace(ForwardIt first, ForwardIt last, const T& old_value, const T& new_value)
    {
        for (; first != last; ++first)
            if (*first == old_value)
                *first = new_value;
    }
}