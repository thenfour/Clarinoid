

#define CLARINOID_PLATFORM_X86
#define CLARINOID_MODULE_TEST // OK this is not a test, but because we pull in all headers, we use test foundation like test timer instead of real timer.

#include <map>

#include <clarinoid/x86/ArduinoEmu.hpp>
#include <clarinoid/basic/Basic.hpp>


// represents a document + serialization context
// that means it's pointing to a VALUE. the document itself is a value.
struct CSFContext
{
  const char *doc;
  const char *cursor;
  uint16_t indent;
  uint16_t line;

  // for all values,
  // cursor points at
  //     name:      1  # comment possible for numeric
  //          ^
  // for strings, no comments are possible, to allow # characetrs to be in the string.
  int AsInteger() const {
  }
  String AsString() const {
  }
  float AsFloat() const {
  }
  bool AsBool() const {
    return AsInteger() == 1;
  }

  // cursor points after the colon after someObject
  //     someObject:
  //       xyz:1
  //       abc:2
  // gets context for a sub-value. assumes 'this' context is an object/dictionary.
  CSFContext operator [](const char * s) const {
    //
  }

  template<typename T, size_t N>
  void DeserializeArray(const T(&arr)[N]) const {
    for (size_t i = 0; i < N; ++i) {
      arr[i].Deserialize(*this);
    }
  }

  template<typename Tdest>
  static void Deserialize(const char *src, Tdest& dest) {
    //
  }
  template<typename Tsrc>
  static String Serialize(const Tsrc& src) {
    //
  }
};

struct CSFWriter {
  //
};


struct Patch {
  void Serialize(CSFWriter& dest) {
  }
  void Deserialize(const CSFContext& src) {
  }
};

struct Settings {
  int intParam1;
  int intParam2;
  float floatParam3;
  Patch patches[10];
  Patch singlePatch;

  void Serialize(CSFWriter& dest) {
    dest.WriteInt("intParam1", intParam1);
    dest.WriteObject("singlePatch", singlePatch);
    dest.WriteArray("patches", patches);
  }

  void Deserialize(const CSFContext& src) {
    intParam1 = src["intParam1"].AsInteger();
    singlePatch.Deserialize(src["singlePatch"]);
    src["patches"].DeserializeArray(patches);
  }
};


int main()
{
  Settings s;
  return 0;
}
