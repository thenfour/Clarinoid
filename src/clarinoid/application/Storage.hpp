

#pragma once

#include <clarinoid/basic/Basic.hpp>

namespace clarinoid
{

// it's hard to design this interface having no clue where this feature will evolve.

struct IStorage
{
    virtual Result SaveSettings(StorageChannel ch, AppSettings& val) = 0;
    virtual Result LoadSettings(StorageChannel ch, AppSettings&doc) = 0;
    virtual void Dir(StorageChannel ch) = 0;
};


} // namespace clarinoid

