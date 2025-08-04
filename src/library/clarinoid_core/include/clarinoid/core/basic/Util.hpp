#pragma once

#include <limits>

#include "Memory.hpp"
#include "Property.hpp"

namespace clarinoid
{

// use as a global var to run init code
struct StaticInit
{
    template <typename T>
    StaticInit(T &&x)
    {
        x();
    }
};


struct NoInterrupts
{
    static int gNoInterruptRefs;
    NoInterrupts()
    {
#ifndef CLARINOID_PLATFORM_X86
        if (0 == gNoInterruptRefs)
        {
            // __disable_irq(); // not sure which one to use honestly...
            NVIC_DISABLE_IRQ(IRQ_SOFTWARE);
        }
#endif
        gNoInterruptRefs++;
    }
    ~NoInterrupts()
    {
        gNoInterruptRefs--;
#ifndef CLARINOID_PLATFORM_X86
        if (0 == gNoInterruptRefs)
        {
            // __enable_irq(); // not sure which one to use honestly...
            NVIC_ENABLE_IRQ(IRQ_SOFTWARE);
        }
#endif
    }
};

int NoInterrupts::gNoInterruptRefs = 0;

} // namespace clarinoid
