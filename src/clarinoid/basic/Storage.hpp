

#pragma once

namespace clarinoid
{

// it's hard to design this interface having no clue where this feature will evolve.

struct IStorage
{
    virtual Result SaveDocument(StorageChannel ch, ClarinoidJsonDocument& doc) = 0;
    virtual DeserializationError LoadDocument(StorageChannel ch, ClarinoidJsonDocument &doc) = 0;
    virtual void Dir(StorageChannel ch) = 0;
};


} // namespace clarinoid
