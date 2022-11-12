
#pragma once

namespace clarinoid
{
static String PointerToString(const void *p)
{
    return {uintptr_t(p), 16};
}

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

// struct SerializableObject
// {
//   protected:
//     SerializableObject(const char *fieldName) : mSerializableFieldName(fieldName)
//     {
//     }

//   public:
//     const char *mSerializableFieldName;

//     virtual size_t SerializableObject_ToSerial()
//     {
//         CCASSERT(!!mSerializableFieldName);
//         Serial.println(String("SerializableObject::SerializableObject_ToSerial() ->pThis=") +
//                        String((uintptr_t)this, 16));
//         ClarinoidJsonDocument doc;
//         SerializableObject_ToJSON(doc);
//         // String ret;
//         return serializeJson(doc, Serial); // minified
//         // serializeJsonPretty(doc, ret); // pretty
//         // return "";
//     }

//     virtual String SerializableObject_GetFieldName()
//     {
//         return mSerializableFieldName;
//     }

//     virtual void SerializableObject_FromString(const String &str)
//     {
//         // DynamicJsonDocument doc(1024);
//         // char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

//         // // deserializeJson(doc, json);
//         // auto x = doc[""];
//         // Serial.println(doc["sensor"].as<String>());
//     }

//     virtual bool SerializableObject_ToJSON(JsonVariant obj) = 0;
//     virtual bool SerializableObject_FromJSON(JsonVariant rhs) = 0;
//     // virtual void SerializableObject_Serialize(JsonObject parent) {
//     //     CCASSERT(!!mSerializableFieldName);
//     //     JsonVariant v = parent.getOrAddMember(mSerializableFieldName);
//     //     SerializableField_SerializeValue(v);
//     // }

//     // no key name (parent has sorted that out)
//     // virtual void SerializableObject_Serialize(JsonArray obj) = 0;
// };

// // this allows conversion of our objects for native ArduinoJson use.
// // https://arduinojson.org/v6/api/jsonarray/add/
// bool convertToJson(SerializableObject *src, JsonVariant dst)
// {
//     return src->SerializableObject_ToJSON(dst);
// }

// static constexpr auto aichp = sizeof(SerializableObject);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serialization of fundamental types / structures

// // represents a container object with N child values. a dictionary.
// struct SerializableDictionary //: SerializableObject
// {
//     array_view<SerializableObject *> mSerializableChildObjects;
//     // const int mMySerializableIndex; // -1 = don't use index.

//     // the reason we use an index is to avoid dynamically-creating strings at initialization.
//     // storing a const char * is much smaller than a String().
//     SerializableDictionary(const char *fieldName, array_view<SerializableObject *> childObjects)
//         : SerializableObject(fieldName), mSerializableChildObjects(childObjects) //, mMySerializableIndex(myIndex)
//     {
//     }

//     virtual bool SerializableObject_ToJSON(JsonVariant dest) override
//     {
//         dest.to<JsonObject>();
//         for (size_t i = 0; i < mSerializableChildObjects.size(); ++i)
//         {
//             // Serial.println(String("serializing child : ") + mSerializableChildObjects[i]->mSerializableFieldName);
//             JsonObject childObj =
//                 dest.createNestedObject(mSerializableChildObjects[i]->SerializableObject_GetFieldName());
//             if (!mSerializableChildObjects[i]->SerializableObject_ToJSON(childObj))
//             {
//                 return false;
//             }
//         }
//         return true;
//     }

//     virtual bool SerializableObject_FromJSON(JsonVariant dest) override
//     {
//         // TODO
//         return true;
//     }
// };

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

// // attach to a std::array to create a serializer.
// template <typename T, size_t N>
// struct ArraySerializer : SerializableObject
// {
//     std::array<T, N> &mArray;

//     // the reason we use an index is to avoid dynamically-creating strings at initialization.
//     // storing a const char * is much smaller than a String().
//     ArraySerializer(const char *fieldName, std::array<T, N> &obj)
//         : SerializableObject(fieldName), //
//           mArray(obj)
//     {
//     }

//     virtual bool SerializableObject_ToJSON(JsonVariant dest) override
//     {
//         dest.to<JsonArray>();
//         for (auto &x : mArray)
//         {
//             auto ch = dest.createNestedObject();
//             if (!x.SerializableObject_ToJSON(ch))
//             {
//                 return false;
//             }
//         }
//         return true;
//     }

//     virtual bool SerializableObject_FromJSON(JsonVariant dest) override
//     {
//         // TODO
//         return true;
//     }
// };

} // namespace clarinoid
