
#pragma once

#include <functional>
#include <type_traits>

namespace cc
{
    template<typename Tsignature>
    struct function
    {
        using ptr_t = Tsignature*;
    private:
        function() = delete;
    };

}

