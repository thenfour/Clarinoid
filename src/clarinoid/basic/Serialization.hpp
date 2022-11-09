
#pragma once

namespace clarinoid
{

struct ClarinoidJsonDocument : public JsonDocument {
  ClarinoidJsonDocument() : JsonDocument((char *)gClarinoidDmaMem.gJSONBuffer, SizeofStaticArray(gClarinoidDmaMem.gJSONBuffer)) {}

  ClarinoidJsonDocument(const ClarinoidJsonDocument& src)
      : JsonDocument((char *)gClarinoidDmaMem.gJSONBuffer, SizeofStaticArray(gClarinoidDmaMem.gJSONBuffer)) {
    set(src);
  }

  ClarinoidJsonDocument& operator=(const ClarinoidJsonDocument& src) {
    set(src);
    return *this;
  }

  template <typename T>
  ClarinoidJsonDocument& operator=(const T& src) {
    set(src);
    return *this;
  }

  void garbageCollect() {
    ClarinoidJsonDocument tmp(*this);
    set(tmp);
  }
};

struct SerializableObject
{
  protected:
    SerializableObject(const char *fieldName) : mSerializableFieldName(fieldName)
    {
    }

  public:
    const char *mSerializableFieldName;

    virtual String SerializableObject_ToString()
    {
        CCASSERT(!!mSerializableFieldName);
        ClarinoidJsonDocument doc; 
        SerializableObject_ToJSON(doc);
        // String ret;
        serializeJson(doc, Serial); // minified
        // serializeJsonPretty(doc, ret); // pretty
        return "";
    }

    virtual String SerializableObject_GetFieldName()
    {
        return mSerializableFieldName;
    }

    virtual void SerializableObject_FromString(const String &str)
    {
        // DynamicJsonDocument doc(1024);
        // char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

        // // deserializeJson(doc, json);
        // auto x = doc[""];
        // Serial.println(doc["sensor"].as<String>());
    }

    virtual bool SerializableObject_ToJSON(JsonVariant obj) = 0;
    virtual bool SerializableObject_FromJSON(JsonVariant rhs) = 0;
    // virtual void SerializableObject_Serialize(JsonObject parent) {
    //     CCASSERT(!!mSerializableFieldName);
    //     JsonVariant v = parent.getOrAddMember(mSerializableFieldName);
    //     SerializableField_SerializeValue(v);
    // }

    // no key name (parent has sorted that out)
    // virtual void SerializableObject_Serialize(JsonArray obj) = 0;
};

// this allows conversion of our objects for native ArduinoJson use.
// https://arduinojson.org/v6/api/jsonarray/add/
bool convertToJson(SerializableObject *src, JsonVariant dst)
{
    return src->SerializableObject_ToJSON(dst);
}

static constexpr auto aichp = sizeof(SerializableObject);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serialization of fundamental types / structures

// represents a container object with N child values. a dictionary.
struct SerializableDictionary : SerializableObject
{
    array_view<SerializableObject *> mSerializableChildObjects;
    // const int mMySerializableIndex; // -1 = don't use index.

    // the reason we use an index is to avoid dynamically-creating strings at initialization.
    // storing a const char * is much smaller than a String().
    SerializableDictionary(const char *fieldName, array_view<SerializableObject *> childObjects)
        : SerializableObject(fieldName), mSerializableChildObjects(childObjects) //, mMySerializableIndex(myIndex)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant dest) override
    {
        dest.to<JsonObject>();
        for (size_t i = 0; i < mSerializableChildObjects.size(); ++i)
        {
            // Serial.println(String("serializing child : ") + mSerializableChildObjects[i]->mSerializableFieldName);
            JsonObject childObj =
                dest.createNestedObject(mSerializableChildObjects[i]->SerializableObject_GetFieldName());
            if (!mSerializableChildObjects[i]->SerializableObject_ToJSON(childObj))
            {
                return false;
            }
        }
        return true;
    }

    virtual bool SerializableObject_FromJSON(JsonVariant dest) override
    {
        // TODO
        return true;
    }
};

// attach to a std::array to create a serializer.
template <typename T, size_t N>
struct ArraySerializer : SerializableObject
{
    std::array<T, N> &mArray;

    // the reason we use an index is to avoid dynamically-creating strings at initialization.
    // storing a const char * is much smaller than a String().
    ArraySerializer(const char *fieldName, std::array<T, N> &obj)
        : SerializableObject(fieldName), //
          mArray(obj)
    {
    }

    virtual bool SerializableObject_ToJSON(JsonVariant dest) override
    {
        dest.to<JsonArray>();
        for (auto &x : mArray)
        {
            auto ch = dest.createNestedObject();
            if (!x.SerializableObject_ToJSON(ch))
            {
                return false;
            }
        }
        return true;
    }

    virtual bool SerializableObject_FromJSON(JsonVariant dest) override
    {
        // TODO
        return true;
    }
};

} // namespace clarinoid
