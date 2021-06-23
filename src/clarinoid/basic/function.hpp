
#pragma once

#include <functional>
#include <type_traits>

namespace clarinoid
{

namespace cc
{
// use just to generate a function ptr type that's similar to the syntax you'd use for std::function.
template <typename Tsignature>
struct function
{
    using ptr_t = Tsignature *;

  private:
    function() = delete;
};

} // namespace cc

} // namespace clarinoid
