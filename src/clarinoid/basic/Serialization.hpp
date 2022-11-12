
#pragma once

namespace clarinoid
{
// static String PointerToString(const void *p)
// {
//     return {uintptr_t(p), 16};
// }

struct ClarinoidJsonDocument : public JsonDocument
{
    static ClarinoidJsonDocument *gInstance;
    ClarinoidJsonDocument()
        : JsonDocument((char *)gClarinoidDmaMem.gJSONBuffer, SizeofStaticArray(gClarinoidDmaMem.gJSONBuffer))
    {
        // Serial.println(String("ClarinoidJsonDocument::ClarinoidJsonDocument() ->gInstance=") +
        //                String((uintptr_t)gInstance, 16) + "; pThis=" + String((uintptr_t)this, 16));

        // singleton because we use a single static heap
        CCASSERT(!gInstance);
        gInstance = this;
    }
    virtual ~ClarinoidJsonDocument()
    {
        gInstance = nullptr;
    }
};

ClarinoidJsonDocument *ClarinoidJsonDocument::gInstance = nullptr;

template <typename T, size_t N>
bool SerializeArrayToJSON(JsonVariant parent, const std::array<T, N> &arr)
{
    bool ret = true;
    for (auto &ch : arr)
    {
        ret = ret && ch.SerializableObject_ToJSON(parent.addElement());
        if (!ret)
        {
            break;
        }
    }
    return ret;
}

template <typename T, size_t N>
Result DeserializeArray(JsonVariant parent, std::array<T, N> &outp)
{
    if (!parent.is<JsonArray>())
    {
        return Result::Failure("Expected array");
    }
    Result ret = Result::Success();
    JsonArray parentArray = parent.as<JsonArray>();
    size_t iout = 0;
    for (JsonVariant item : parentArray)
    {
        if (iout >= N)
        {
            ret.AddWarning(String("Warn:items were skipped (") + parentArray.size() + ">" + N + ")");
            break;
        }
        ret.AndRequires(outp[iout].SerializableObject_Deserialize(item), String("item#") + iout);
        ++iout;
    }
    if (iout != N)
    {
        ret.AddWarning(String("Warn:fewer input items than expected (") + parentArray.size() + "<" + N + ")");
    }
    return ret;
}

} // namespace clarinoid
