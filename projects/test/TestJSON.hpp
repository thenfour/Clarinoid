#pragma once

namespace clarinoid
{

struct DebugStream : IStream
{
    std::string mStr;
    int mCursor = 0; // cursor points to the next byte to read.

    virtual size_t write(const uint8_t *buf, size_t bytes) override
    {
        mStr.append((const char *)buf, bytes);
        std::cout << std::string{(const char *)buf, bytes};
        mCursor += (int)bytes;
        return bytes;
    }

    virtual size_t read(uint8_t *buf, size_t bytes) override
    {
        if (bytes == 0)
            return 0; // knowing we're reading at least 1 byte helps simplify
        int newCursor = std::min(mCursor + (int)bytes, (int)mStr.length()); // maximum cursor is end().
        size_t ret = newCursor - mCursor;
        memcpy(buf, mStr.data() + mCursor, ret);
        mCursor = newCursor;
        return ret;
    }

    virtual int readByte() override
    {
        uint8_t ret;
        return !!read(&ret, 1) ? (int)ret : (int)-1;
    }
    virtual size_t flushWrite() override
    {
        return 0;
    }
};


void TestJSON()
{
    DebugStream outputStream;
    BufferedStream buffered{outputStream};
    TextStream textStream{buffered};
    JsonVariantWriter doc{textStream};

    HarmVoiceSettings voice;
    AppSettings s;
    // TODO: fuzz s
    Serialize(doc, s);

    doc.FinishWriting();
    textStream.DumpStats();

    DebugStream outputStream2;
    outputStream2.mCursor = 0;
    outputStream2.mStr = outputStream.mStr;

    BufferedStream buffered2{outputStream2};
    TextStream textStream2{buffered2};

    AppSettings s2;
    JsonVariantReader doc2(textStream2);
    auto dr = Deserialize(doc2, s2);

    TestEquals(s, s2);
}



} // namespace clarinoid
