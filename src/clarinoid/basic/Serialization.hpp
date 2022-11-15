// TODO: maximum string size and possibly other sanity checks

#pragma once

#include <type_traits>

namespace clarinoid
{

struct IStream
{
    virtual size_t write(const uint8_t *buf, size_t bytes) = 0;
    virtual size_t read(uint8_t *buf, size_t bytes) = 0;
    virtual int readByte() = 0;
    virtual size_t flushWrite() = 0;
};

struct StreamStream : IStream
{
    ::Stream &mStream;
    StreamStream(::Stream &s) : mStream(s)
    {
    }
    virtual size_t write(const uint8_t *buf, size_t bytes) override
    {
        return mStream.write(buf, bytes);
    }

    virtual size_t read(uint8_t *buf, size_t bytes) override
    {
        return mStream.readBytes((char *)buf, bytes);
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

struct IPeekableStream : IStream
{
    // return <0 if failure
    virtual int peekByte() = 0;
};

struct BufferedStream : IPeekableStream
{
    static constexpr size_t BufferSize = gBufferedStreamBufferSize;
    IStream &mStream;
    uint8_t mBuffer[BufferSize]{0};
    size_t mBufferUsed = 0;
    size_t mReadBufferCursor = 0;

    BufferedStream(IStream &stream) : mStream(stream)
    {
    }

    virtual size_t write(const uint8_t *buf, size_t bytes) override
    {
        size_t ret = 0;
        while (bytes > 0)
        {
            size_t toWriteToBuffer = std::min(bytes, BufferSize - mBufferUsed);
            memcpy(mBuffer + mBufferUsed, buf, toWriteToBuffer);
            mBufferUsed += toWriteToBuffer;
            ret += toWriteToBuffer;
            if (mBufferUsed >= BufferSize)
            {
                size_t written = flushWrite();
                if (written != BufferSize)
                {
                    // something went wrong with underlying stream. likely unusable past this state.
                    int bytesFailed = (int)BufferSize - written;
                    return ret - bytesFailed;
                }
            }
            buf += toWriteToBuffer;
            bytes -= toWriteToBuffer;
        }
        return ret;
    }

    virtual int readByte() override
    {
        uint8_t ret;
        return !!read(&ret, 1) ? (int)ret : (int)-1;
    }

    virtual size_t flushWrite() override
    {
        size_t ret = mStream.write(mBuffer, mBufferUsed);
        mBufferUsed = 0;
        return ret;
    }

    virtual size_t read(uint8_t *buf, size_t bytes) override
    {
        size_t ret = 0;
        while (bytes && EnsureBufferHasUnreadContent())
        {
            size_t unreadBufferBytes = mBufferUsed - mReadBufferCursor;
            size_t bufferBytesToRead = std::min(bytes, unreadBufferBytes);
            memcpy(buf, mBuffer + mReadBufferCursor, bufferBytesToRead);
            mReadBufferCursor += bufferBytesToRead;
            ret += bufferBytesToRead;
            buf += bufferBytesToRead;
            bytes -= bufferBytesToRead;
        }
        return ret;
    }

    virtual int peekByte() override
    {
        if (!EnsureBufferHasUnreadContent())
        {
            return -1;
        }
        return mBuffer[mReadBufferCursor];
    }

    // tries to make sure there's at least 1 unbyte in the buffer.
    // if unable to get even 1 byte, returns false.
    bool EnsureBufferHasUnreadContent()
    {
        // if there's no more buffer to read
        if (mReadBufferCursor >= mBufferUsed)
        {
            // get a fresh buffer.
            mBufferUsed = mStream.read(mBuffer, BufferSize);
            mReadBufferCursor = 0;
            return mBufferUsed > 0;
        }
        return mBufferUsed != 0;
    }
};

struct TextStream : IPeekableStream
{
    IPeekableStream &mStream;
    // 1-based per convention
    uint32_t mLine = 1;
    uint32_t mColumn = 1;

    TextStream(IPeekableStream &stream) : mStream(stream)
    {
    }

    virtual size_t write(const uint8_t *buf, size_t bytes) override
    {
        return mStream.write(buf, bytes);
    }

    virtual size_t read(uint8_t *buf, size_t bytes) override
    {
        return mStream.read(buf, bytes);
    }

    size_t write(const String &s)
    {
        size_t ret = mStream.write((const uint8_t *)s.c_str(), s.length());
        for (size_t i = 0; i < ret; ++i)
        {
            interpretChar(s[i]);
        }
        return ret;
    }

    // Reads one byte, or returns -1
    virtual int readByte() override
    {
        int ch = mStream.readByte();
        if (ch <= 0)
            return ch;
        interpretChar(ch);
        return ch;
    }

    virtual int peekByte() override
    {
        return mStream.peekByte();
    }

    virtual size_t flushWrite() override
    {
        return mStream.flushWrite();
    }

  private:
    void interpretChar(char ch)
    {
        if (ch == '\n')
        {
            mLine++;
            mColumn = 1;
            return;
        }
        mColumn++;
    }
};

namespace CCJSON
{
template <typename T, typename F>
T alias_cast(F raw_data)
{
    union {
        F raw;
        T data;
    } z;
    z.raw = raw_data;
    return z.data;
}

template <bool Condition, class TrueType, class FalseType>
struct conditional
{
    typedef TrueType type;
};

template <class TrueType, class FalseType>
struct conditional<false, TrueType, FalseType>
{
    typedef FalseType type;
};

template <typename A, typename B>
struct choose_largest : conditional<(sizeof(A) > sizeof(B)), A, B>
{
};

typedef double Float;
typedef int64_t Integer;
typedef uint64_t UInt;

enum class NumberType : uint8_t
{
    Undefined,
    Float,
    Integer,
    UInteger,
};

struct EscapeSequence
{
    // returns the json char after escape slash, for the given string character.
    // returns 0 if this char does not need escaping.
    static char escapeChar(char c)
    {
        const char *p = escapeTable(true);
        while (p[0] && p[1] != c)
        {
            p += 2;
        }
        return p[0];
    }
    // accepts a json char after slash, and returns the actual string char.
    // returns 0 if this is not a recognized escape sequence.
    static char unescapeChar(char c)
    {
        const char *p = escapeTable(false);
        for (;;)
        {
            if (p[0] == '\0')
                return 0;
            if (p[0] == c)
                return p[1];
            p += 2;
        }
    }

  private:
    static const char *escapeTable(bool excludeSolidus)
    {
        // groups of 2 chars; 1st char is the incoming char after an escape char ('\')
        // 2nd char is the actual char to output.
        return &"//\"\"\\\\b\bf\fn\nr\rt\t"[excludeSolidus ? 2 : 0];
    }
};

template <typename T, size_t = sizeof(T)>
struct FloatTraits
{
};

template <typename T>
struct FloatTraits<T, sizeof(double)>
{
    typedef uint64_t mantissa_type;
    static const short mantissa_bits = 52;
    static const mantissa_type mantissa_max = (mantissa_type(1) << mantissa_bits) - 1;

    typedef int16_t exponent_type;
    static const exponent_type exponent_max = 308;

    template <typename TExponent>
    static T make_float(T m, TExponent e)
    {
        if (e > 0)
        {
            for (uint8_t index = 0; e != 0; index++)
            {
                if (e & 1)
                    m *= positiveBinaryPowerOfTen(index);
                e >>= 1;
            }
        }
        else
        {
            e = TExponent(-e);
            for (uint8_t index = 0; e != 0; index++)
            {
                if (e & 1)
                    m *= negativeBinaryPowerOfTen(index);
                e >>= 1;
            }
        }
        return m;
    }

    static T positiveBinaryPowerOfTen(int index)
    {
        static uint32_t const factors[] PROGMEM = {
            0x40240000,
            0x00000000, // 1e1
            0x40590000,
            0x00000000, // 1e2
            0x40C38800,
            0x00000000, // 1e4
            0x4197D784,
            0x00000000, // 1e8
            0x4341C379,
            0x37E08000, // 1e16
            0x4693B8B5,
            0xB5056E17, // 1e32
            0x4D384F03,
            0xE93FF9F5, // 1e64
            0x5A827748,
            0xF9301D32, // 1e128
            0x75154FDD,
            0x7F73BF3C // 1e256
        };
        return forge(factors[2 * index], factors[2 * index + 1]);
    }

    static T negativeBinaryPowerOfTen(int index)
    {
        static uint32_t const factors[] PROGMEM = {
            0x3FB99999,
            0x9999999A, // 1e-1
            0x3F847AE1,
            0x47AE147B, // 1e-2
            0x3F1A36E2,
            0xEB1C432D, // 1e-4
            0x3E45798E,
            0xE2308C3A, // 1e-8
            0x3C9CD2B2,
            0x97D889BC, // 1e-16
            0x3949F623,
            0xD5A8A733, // 1e-32
            0x32A50FFD,
            0x44F4A73D, // 1e-64
            0x255BBA08,
            0xCF8C979D, // 1e-128
            0x0AC80628,
            0x64AC6F43 // 1e-256
        };
        return forge(factors[2 * index], factors[2 * index + 1]);
    }

    static T negativeBinaryPowerOfTenPlusOne(int index)
    {
        static uint32_t const factors[] PROGMEM = {
            0x3FF00000,
            0x00000000, // 1e0
            0x3FB99999,
            0x9999999A, // 1e-1
            0x3F50624D,
            0xD2F1A9FC, // 1e-3
            0x3E7AD7F2,
            0x9ABCAF48, // 1e-7
            0x3CD203AF,
            0x9EE75616, // 1e-15
            0x398039D6,
            0x65896880, // 1e-31
            0x32DA53FC,
            0x9631D10D, // 1e-63
            0x25915445,
            0x81B7DEC2, // 1e-127
            0x0AFE07B2,
            0x7DD78B14 // 1e-255
        };
        return forge(factors[2 * index], factors[2 * index + 1]);
    }

    static T nan()
    {
        return forge(0x7ff80000, 0x00000000);
    }

    static T inf()
    {
        return forge(0x7ff00000, 0x00000000);
    }

    static T highest()
    {
        return forge(0x7FEFFFFF, 0xFFFFFFFF);
    }

    static T lowest()
    {
        return forge(0xFFEFFFFF, 0xFFFFFFFF);
    }

    // constructs a double floating point values from its binary representation
    // we use this function to workaround platforms with single precision literals
    // (for example, when -fsingle-precision-constant is passed to GCC)
    static T forge(uint32_t msb, uint32_t lsb)
    {
        return alias_cast<T>((uint64_t(msb) << 32) | lsb);
    }
};

// specialization for 32-bit floats
template <typename T>
struct FloatTraits<T, sizeof(float)>
{
    typedef uint32_t mantissa_type;
    static const short mantissa_bits = 23;
    static const mantissa_type mantissa_max = (mantissa_type(1) << mantissa_bits) - 1;

    typedef int8_t exponent_type;
    static const exponent_type exponent_max = 38;

    template <typename TExponent>
    static T make_float(T m, TExponent e)
    {
        if (e > 0)
        {
            for (uint8_t index = 0; e != 0; index++)
            {
                if (e & 1)
                    m *= positiveBinaryPowerOfTen(index);
                e >>= 1;
            }
        }
        else
        {
            e = -e;
            for (uint8_t index = 0; e != 0; index++)
            {
                if (e & 1)
                    m *= negativeBinaryPowerOfTen(index);
                e >>= 1;
            }
        }
        return m;
    }

    static T positiveBinaryPowerOfTen(int index)
    {
        static uint32_t const factors[] PROGMEM = {
            0x41200000, // 1e1f
            0x42c80000, // 1e2f
            0x461c4000, // 1e4f
            0x4cbebc20, // 1e8f
            0x5a0e1bca, // 1e16f
            0x749dc5ae  // 1e32f
        };
        return forge(factors[index]);
    }

    static T negativeBinaryPowerOfTen(int index)
    {
        static uint32_t const factors[] PROGMEM = {
            0x3dcccccd, // 1e-1f
            0x3c23d70a, // 1e-2f
            0x38d1b717, // 1e-4f
            0x322bcc77, // 1e-8f
            0x24e69595, // 1e-16f
            0x0a4fb11f  // 1e-32f
        };
        return forge(factors[index]);
    }

    static T negativeBinaryPowerOfTenPlusOne(int index)
    {
        static uint32_t const factors[] PROGMEM = {
            0x3f800000, // 1e0f
            0x3dcccccd, // 1e-1f
            0x3a83126f, // 1e-3f
            0x33d6bf95, // 1e-7f
            0x26901d7d, // 1e-15f
            0x0c01ceb3  // 1e-31f
        };
        return forge(factors[index]);
    }

    static T forge(uint32_t bits)
    {
        return alias_cast<T>(bits);
    }

    static T nan()
    {
        return forge(0x7fc00000);
    }

    static T inf()
    {
        return forge(0x7f800000);
    }

    static T highest()
    {
        return forge(0x7f7fffff);
    }

    static T lowest()
    {
        return forge(0xFf7fffff);
    }
};

template <typename TFloat>
struct FloatParts
{
    uint32_t integral;
    uint32_t decimal;
    int16_t exponent;
    int8_t decimalPlaces;

    // Control the exponentiation threshold for big numbers
    // CAUTION: cannot be more that 1e9 !!!!
    static constexpr TFloat POSITIVE_EXPONENTIATION_THRESHOLD = TFloat(1e7);
    static constexpr TFloat NEGATIVE_EXPONENTIATION_THRESHOLD = TFloat(1e-5);

    FloatParts(TFloat value)
    {
        uint32_t maxDecimalPart = sizeof(TFloat) >= 8 ? 1000000000 : 1000000;
        decimalPlaces = sizeof(TFloat) >= 8 ? 9 : 6;

        exponent = normalize(value);

        integral = uint32_t(value);
        // reduce number of decimal places by the number of integral places
        for (uint32_t tmp = integral; tmp >= 10; tmp /= 10)
        {
            maxDecimalPart /= 10;
            decimalPlaces--;
        }

        TFloat remainder = (value - TFloat(integral)) * TFloat(maxDecimalPart);

        decimal = uint32_t(remainder);
        remainder = remainder - TFloat(decimal);

        // rounding:
        // increment by 1 if remainder >= 0.5
        decimal += uint32_t(remainder * 2);
        if (decimal >= maxDecimalPart)
        {
            decimal = 0;
            integral++;
            if (exponent && integral >= 10)
            {
                exponent++;
                integral = 1;
            }
        }

        // remove trailing zeros
        while (decimal % 10 == 0 && decimalPlaces > 0)
        {
            decimal /= 10;
            decimalPlaces--;
        }
    }

    static int16_t normalize(TFloat &value)
    {
        typedef FloatTraits<TFloat> traits;
        int16_t powersOf10 = 0;

        int8_t index = sizeof(TFloat) == 8 ? 8 : 5;
        int bit = 1 << index;

        if (value >= POSITIVE_EXPONENTIATION_THRESHOLD)
        {
            for (; index >= 0; index--)
            {
                if (value >= traits::positiveBinaryPowerOfTen(index))
                {
                    value *= traits::negativeBinaryPowerOfTen(index);
                    powersOf10 = int16_t(powersOf10 + bit);
                }
                bit >>= 1;
            }
        }

        if (value > 0 && value <= NEGATIVE_EXPONENTIATION_THRESHOLD)
        {
            for (; index >= 0; index--)
            {
                if (value < traits::negativeBinaryPowerOfTenPlusOne(index))
                {
                    value *= traits::positiveBinaryPowerOfTen(index);
                    powersOf10 = int16_t(powersOf10 - bit);
                }
                bit >>= 1;
            }
        }

        return powersOf10;
    }
};

String UnsignedIntegerToStringWithOptionalPrefix(UInt value, char prefixChar = 0)
{
    char buffer[24] = {0};
    char *end = buffer + sizeof(buffer) - 1;
    char *begin = end;

    // write the string in reverse order
    do
    {
        *--begin = char(value % 10 + '0');
        value = UInt(value / 10);
    } while (value);

    if (prefixChar)
    {
        *--begin = prefixChar;
    }

    return {begin};
}

String UnsignedIntegerToString(UInt value)
{
    return UnsignedIntegerToStringWithOptionalPrefix(value, 0);
}

String SignedIntegerToString(Integer value)
{
    String ret;
    if (value < 0)
    {
        UInt unsigned_value = UInt(UInt(~value) + 1);
        return ret + UnsignedIntegerToStringWithOptionalPrefix(unsigned_value, '-');
    }
    return UnsignedIntegerToStringWithOptionalPrefix(UInt(value), 0);
}

struct NumberData
{
    NumberType mType = NumberType::Undefined;
    Float mFloatValue = 0;
    Integer mIntValue = 0;
    UInt mUIntValue = 0;

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    void Set(T param)
    {
        mFloatValue = param;
        mType = NumberType::Float;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, int> = 0>
    void Set(T param)
    {
        mIntValue = param;
        mType = NumberType::Integer;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, int> = 0>
    void Set(T param)
    {
        mUIntValue = param;
        mType = NumberType::UInteger;
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value || std::is_integral<T>::value, int> = 0>
    T Get() const
    {
        switch (mType)
        {
        case NumberType::Undefined:
        default:
            return 0;
        case NumberType::Float:
            return T(mFloatValue);
        case NumberType::Integer:
            return T(mIntValue);
        case NumberType::UInteger:
            return T(mUIntValue);
        }
    }

    String ToString() const
    {
        switch (mType)
        {
        default:
            return "<invalid;fatal error>";
        case NumberType::Undefined:
            return "<undefined>";
        case NumberType::Float:
            return String("<float>:") + String(mFloatValue);
        case NumberType::Integer:
            return String("<signed>:") + SignedIntegerToString(mIntValue);
        case NumberType::UInteger:
            return String("<unsigned>:") + UnsignedIntegerToString(mUIntValue);
        }
    }
};

// this could be adapted to read from stream, char by char, but it's just simpler to go like this.
inline bool parseNumber(const char *s, NumberData &result)
{
    typedef FloatTraits<Float> traits;
    typedef choose_largest<traits::mantissa_type, UInt>::type mantissa_t;
    typedef traits::exponent_type exponent_t;

    CCASSERT(s != 0);

    bool is_negative = false;
    switch (*s)
    {
    case '-':
        is_negative = true;
        s++;
        break;
    case '+':
        s++;
        break;
    }

    if (!isdigit(*s) && *s != '.')
        return false;

    mantissa_t mantissa = 0;
    exponent_t exponent_offset = 0;
    const mantissa_t maxUint = UInt(-1);

    while (isdigit(*s))
    {
        uint8_t digit = uint8_t(*s - '0');
        if (mantissa > maxUint / 10)
            break;
        mantissa *= 10;
        if (mantissa > maxUint - digit)
            break;
        mantissa += digit;
        s++;
    }

    if (*s == '\0')
    {
        if (is_negative)
        {
            const mantissa_t sintMantissaMax = mantissa_t(1) << (sizeof(Integer) * 8 - 1);
            if (mantissa <= sintMantissaMax)
            {
                result.Set(Integer(~mantissa + 1));
                return true;
            }
        }
        else
        {
            result.Set(UInt(mantissa));
            return true;
        }
    }

    // avoid mantissa overflow
    while (mantissa > traits::mantissa_max)
    {
        mantissa /= 10;
        exponent_offset++;
    }

    // remaing digits can't fit in the mantissa
    while (isdigit(*s))
    {
        exponent_offset++;
        s++;
    }

    if (*s == '.')
    {
        s++;
        while (isdigit(*s))
        {
            if (mantissa < traits::mantissa_max / 10)
            {
                mantissa = mantissa * 10 + uint8_t(*s - '0');
                exponent_offset--;
            }
            s++;
        }
    }

    int exponent = 0;
    if (*s == 'e' || *s == 'E')
    {
        s++;
        bool negative_exponent = false;
        if (*s == '-')
        {
            negative_exponent = true;
            s++;
        }
        else if (*s == '+')
        {
            s++;
        }

        while (isdigit(*s))
        {
            exponent = exponent * 10 + (*s - '0');
            if (exponent + exponent_offset > traits::exponent_max)
            {
                if (negative_exponent)
                    result.Set(is_negative ? -0.0f : 0.0f);
                else
                    result.Set(is_negative ? -traits::inf() : traits::inf());
                return true;
            }
            s++;
        }
        if (negative_exponent)
            exponent = -exponent;
    }
    exponent += exponent_offset;

    // we should be at the end of the string, otherwise it's an error
    if (*s != '\0')
        return false;

    Float final_result = traits::make_float(static_cast<Float>(mantissa), exponent);

    result.Set(is_negative ? -final_result : final_result);

    return true;
}
} // namespace CCJSON

enum class JsonDataType : uint8_t
{
    EndOfList,
    Undefined,
    Null,
    Number,
    String,
    Boolean,
    Array,
    Object,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// requirements of implementer:
// - non-copyable (delete copy ctor & operator =())
// - when creating child objects, call ExpectChildStreamAccess(), then construct with parent. default ctor should use
// nullptr
// - when accessing the stream, call OnStreamAccess() to perform checks & notify parent who's the currently active child
// - destructor call OnDestructor();
struct JsonParentChildHelper
{
  protected:
    bool mIsLocked = false;

  private:
    JsonParentChildHelper *mParent = nullptr;
    JsonParentChildHelper *mCurrentChild = nullptr;
    bool mExpectingChild = false;

    // called from a child when it's being destructed.
    void OnChildDestroy(JsonParentChildHelper *child)
    {
        // if a child is destructing but not the one we're tracking, then we don't really care; it's some intermediate
        // object
        if (mCurrentChild == child)
        {
            child->EnsureClosed();
            mCurrentChild = nullptr;
        }
    }

    void OnChildCreate(JsonParentChildHelper *child)
    {
        if (mCurrentChild)
        {
            mCurrentChild->EnsureClosed();
        }
        mCurrentChild = child;
    }

  protected:
    // no duplicates allowed because parents keep track of children.
    JsonParentChildHelper(const JsonParentChildHelper &s) = delete;
    JsonParentChildHelper &operator=(const JsonParentChildHelper &s) = delete;

    // but moves are OK;
    explicit JsonParentChildHelper(JsonParentChildHelper &&rhs) = default;

    // parent may be nullptr
    explicit JsonParentChildHelper(JsonParentChildHelper *parent) : mParent(parent) //, mNodeID(CCJSON::GetNextID())
    {
    }

    virtual ~JsonParentChildHelper()
    {
        if (this->mCurrentChild)
        {
            // this would leave a child with an invalid parent pointer
            CCASSERT(false);
        }
    }

    virtual void EnsureClosed() = 0;

    void OnDestructor()
    {
        if (mParent)
        {
            mParent->OnChildDestroy(this);
        }
    }

    // return true if we already know we're closed (because we're locked).
    bool EnsureChildrenClosed()
    {
        if (mIsLocked)
            return true;
        if (mCurrentChild)
        {
            mCurrentChild->EnsureClosed();
        }
        return false;
    }

    void OnStreamAccess()
    {
        if (mParent)
        {
            mParent->SetCurrentlyAccessingChild(this);
        }
    }

    void SetCurrentlyAccessingChild(JsonParentChildHelper *child)
    {
        CCASSERT(child != this);
        // there are a number of assertions we could do here to ensure order of certain operations, but it's precarious
        // and complex so count on other symptoms of errors.

        if (child != mCurrentChild && mCurrentChild)
        {
            CCASSERT(mCurrentChild->mIsLocked); // changing children; make sure that the previous children finished.
        }

        mCurrentChild = child;
        mExpectingChild = false;
    }

    // call when you are done with current child and will move to a next (maybe)
    void ExpectChildStreamAccess()
    {
        // if this is true, it means you were already expecting child activity, before expecting another one.
        // it means you created multiple child values without using them. probably coding something out of order.
        CCASSERT(!mExpectingChild);
        if (mCurrentChild)
        {
            mCurrentChild->mIsLocked = true;
        }
        mCurrentChild = nullptr;
        mExpectingChild = true;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct JsonVariantWriter : JsonParentChildHelper
{
    TextStream &mStream;
    const size_t mNestLevel = 0;
    int mIndentLevel = 0;
    size_t mElementCount = 0;
    JsonDataType mType = JsonDataType::Undefined;

    // no duplicates allowed because parents keep track of children.
    JsonVariantWriter(const JsonVariantWriter &s) = delete;
    JsonVariantWriter &operator=(const JsonVariantWriter &s) = delete;

    // but moves are OK; parents eventually get notified of the change.
    JsonVariantWriter(JsonVariantWriter &&rhs) = default;

    // create child
    JsonVariantWriter(JsonVariantWriter *parent, int indentLevel)
        : JsonParentChildHelper(parent),      //
          mStream(parent->mStream),           //
          mNestLevel(parent->mNestLevel + 1), //
          mIndentLevel(indentLevel)           //
    {
    }

    // create new blank standalone object (document)
    explicit JsonVariantWriter(TextStream &stream)
        : JsonParentChildHelper(nullptr), mStream(stream) //, mNodeID(CCJSON::GetNextID())
    {
    }

    virtual ~JsonVariantWriter()
    {
        OnDestructor();
    }

    void FinishWriting()
    {
        EnsureClosed();
        // trailing new line is typical text file convention
        if (mStream.mColumn > 1)
        {
            WriteRaw("\n");
        }
        mStream.flushWrite();
    }

    // create a key, and return the value context
    JsonVariantWriter Object_MakeKey(const String &s)
    {
        CCASSERT(mType == JsonDataType::Undefined || mType == JsonDataType::Object);
        CCASSERT(!mIsLocked);
        this->OnStreamAccess();
        mType = JsonDataType::Object;
        if (mElementCount > 0)
        {
            EnsureChildrenClosed();
            mStream.write(",");
            WriteNewLine();
        }
        else
        {
            mStream.write("{");
            mIndentLevel++;
            WriteNewLine();
        }
        mElementCount++;
        Result ret = RawWriteQuotedString(s.c_str());
        ret.AndRequires(this->WriteRaw(":"), "key colon");
        ExpectChildStreamAccess();
        return {this, mIndentLevel};
    }

    // create a key, and return the value context
    JsonVariantWriter Array_MakeValue()
    {
        CCASSERT(mType == JsonDataType::Undefined || mType == JsonDataType::Array);
        CCASSERT(!mIsLocked);
        this->OnStreamAccess();
        mType = JsonDataType::Array;
        if (mElementCount > 0)
        {
            EnsureChildrenClosed();
            mStream.write(",");
            WriteNewLine();
        }
        else
        {
            mStream.write("[");
            mIndentLevel++;
            WriteNewLine();
        }
        mElementCount++;
        ExpectChildStreamAccess();
        return {this, mIndentLevel};
    }

    Result WriteStringValue(const String &s)
    {
        if ((mType != JsonDataType::Undefined) || mIsLocked)
            return Result::Failure("jsonvariant can only be used once");
        this->OnStreamAccess();
        mType = JsonDataType::String;
        auto ret = RawWriteQuotedString(s.c_str());
        mIsLocked = true;
        return ret;
    }

    Result WriteNull()
    {
        if ((mType != JsonDataType::Undefined) || mIsLocked)
            return Result::Failure("jsonvariant can only be used once");
        this->OnStreamAccess();
        mType = JsonDataType::Null;
        mStream.write((const uint8_t *)"null", 4);
        mIsLocked = true;
        return Result::Success();
    }

    Result WriteBoolean(bool b)
    {
        if ((mType != JsonDataType::Undefined) || mIsLocked)
            return Result::Failure("jsonvariant can only be used once");
        this->OnStreamAccess();
        mType = JsonDataType::Boolean;
        if (b)
        {
            mStream.write((const uint8_t *)"true", 4);
        }
        else
        {
            mStream.write((const uint8_t *)"false", 5);
        }
        mIsLocked = true;
        return Result::Success();
    }

    virtual void EnsureClosed() override
    {
        if (JsonParentChildHelper::EnsureChildrenClosed())
            return;
        if (mType == JsonDataType::Object)
        {
            Object_Close();
            return;
        }
        if (mType == JsonDataType::Array)
        {
            Array_Close();
            return;
        }

        // other single-value datatypes have already been written and should be locked.
        CCASSERT(false);
    }

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    Result WriteNumberValue(T s)
    {
        if ((mType != JsonDataType::Undefined) || mIsLocked)
            return Result::Failure("jsonvariant can only be used once");
        this->OnStreamAccess();
        mType = JsonDataType::Number;
        auto ret = RawWriteFloat(s);
        mIsLocked = true;
        return ret;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, int> = 0>
    Result WriteNumberValue(T s)
    {
        if ((mType != JsonDataType::Undefined) || mIsLocked)
            return Result::Failure("jsonvariant can only be used once");
        this->OnStreamAccess();
        mType = JsonDataType::Number;
        String str = CCJSON::SignedIntegerToString(s);
        auto ret = WriteRaw(str.c_str());
        mIsLocked = true;
        return ret;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, int> = 0>
    Result WriteNumberValue(T s)
    {
        if ((mType != JsonDataType::Undefined) || mIsLocked)
            return Result::Failure("jsonvariant can only be used once");
        this->OnStreamAccess();
        mType = JsonDataType::Number;
        String str = CCJSON::UnsignedIntegerToString(s);
        auto ret = WriteRaw(str.c_str());
        mIsLocked = true;
        return ret;
    }

    String GetFilePosInfoString() const
    {
        return String("[") + mStream.mLine + ":" + mStream.mColumn + "]";
    }

  private:
    void WriteNewLine()
    {
        mStream.write("\r\n");
        for (int i = 0; i < mIndentLevel; ++i)
        {
            mStream.write("  ");
        }
    }

    Result RawWriteQuotedString(const char *value)
    {
        Result ret = WriteRaw("\"");
        while (*value)
            ret.AndRequires(RawWriteEscapedChar(*value++));
        ret.AndRequires(WriteRaw("\""));
        return ret;
    }

    Result RawWriteEscapedChar(char c)
    {
        char specialChar = CCJSON::EscapeSequence::escapeChar(c);
        Result ret = Result::Success();
        if (specialChar)
        {
            ret.AndRequires(RawWriteChar('\\'));
            ret.AndRequires(RawWriteChar(specialChar));
            return ret;
        }

        // todo: text encoding.
        if (c)
        {
            ret.AndRequires(RawWriteChar(c));
        }
        else
        {
            ret.AndRequires(this->WriteRaw("\\u0000"));
        }
        return ret;
    }

    Result RawWriteChar(char ch)
    {
        return Result::Requires(1 == mStream.write((const uint8_t *)&ch, 1), "stream error");
    }

    template <typename T>
    Result RawWriteFloat(T value)
    {
        if (std::isnan(value))
            return WriteRaw("null");

        if (std::isinf(value))
            return WriteRaw("null");

        Result ret = Result::Success();

        if (value < 0.0)
        {
            ret.AndRequires(WriteRaw("-"), "writing neg sign");
            value = -value;
        }

        CCJSON::FloatParts<T> parts(value);

        WriteRaw(CCJSON::UnsignedIntegerToString(parts.integral).c_str());

        // ret.AndRequires(RawWriteUnsignedInteger(parts.integral), "writing integral part");
        if (parts.decimalPlaces)
        {
            ret.AndRequires(RawWriteDecimals(parts.decimal, parts.decimalPlaces), "writing decimals");
        }

        if (parts.exponent)
        {
            ret.AndRequires(WriteRaw("e"), "writing exp char");
            WriteRaw(CCJSON::SignedIntegerToString(parts.exponent).c_str());
        }
        return ret;
    }

    Result RawWriteDecimals(uint32_t value, int8_t width)
    {
        // buffer should be big enough for all digits and the dot
        char buffer[16];
        char *end = buffer + sizeof(buffer);
        char *begin = end;

        // write the string in reverse order
        while (width--)
        {
            *--begin = char(value % 10 + '0');
            value /= 10;
        }
        *--begin = '.';

        return WriteRaw(begin, end);
    }

    Result WriteRaw(const char *begin, const char *end)
    {
        CCASSERT(end > begin);
        size_t toWrite = end - begin;
        if (toWrite != mStream.write((const uint8_t *)begin, toWrite))
        {
            return Result::Failure("stream write error");
        }
        return Result::Success();
    }

    Result WriteRaw(const char *str)
    {
        size_t toWrite = StringLength(str);
        if (toWrite != mStream.write((const uint8_t *)str, toWrite))
        {
            return Result::Failure("stream write error");
        }
        return Result::Success();
    }

    Result Object_Close()
    {
        if (mType == JsonDataType::Undefined)
        {
            // empty object
            mStream.write("{}");
            WriteNewLine();
            return Result::Success();
        }
        if (mType != JsonDataType::Object)
            return Result::Failure("Object_Close can only be called on objects");
        mIndentLevel--;
        WriteNewLine();
        mStream.write("}");
        mIsLocked = true;
        return Result::Success();
    }

    Result Array_Close()
    {
        if (mType == JsonDataType::Undefined)
        {
            // unused; no biggie; 0 item array.
            mStream.write("[]");
            WriteNewLine();
            return Result::Success();
        }
        if (mType != JsonDataType::Array)
            return Result::Failure("Array_Close can only be called on arrays");
        mIndentLevel--;
        WriteNewLine();
        mStream.write("]");
        mIsLocked = true;
        return Result::Success();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct JsonVariantReader : JsonParentChildHelper
{
    JsonDataType mType = JsonDataType::Undefined;
    bool mBooleanValue = false;
    bool mHasReadAnElement = false; // tells arrays & objects if they're expecting a comma

    Result mParseResult = Result::Success();
    TextStream *mStream = nullptr;
    String mKeyName;
    int16_t mNestLevel = 0;
    String mStringValue;
    CCJSON::NumberData mNumericValue;

    // no duplicates allowed because parents keep track of children.
    JsonVariantReader(const JsonVariantReader &s) = delete;
    JsonVariantReader &operator=(const JsonVariantReader &s) = delete;

    // but moves are OK; parents get notified of the change.
    JsonVariantReader(JsonVariantReader &&rhs) = default;

    // seed child with parse error.
    JsonVariantReader(JsonVariantReader *parent, Result r)
        : JsonParentChildHelper(parent),     //
          mParseResult(r),                   //
          mStream(parent->mStream),          //
          mNestLevel(parent->mNestLevel + 1) //
    {
    }

    // parse object child
    JsonVariantReader(JsonVariantReader *parent, const String &keyName)
        : JsonParentChildHelper(parent),     //
          mStream(parent->mStream),          //
          mKeyName(keyName),                 //
          mNestLevel(parent->mNestLevel + 1) //
    {
        ConstructParse();
    }

    // parse array child
    JsonVariantReader(JsonVariantReader *parent)
        : JsonParentChildHelper(parent),     //
          mStream(parent->mStream),          //
          mNestLevel(parent->mNestLevel + 1) //
    {
        ConstructParse();
    }

    // create array or object child with type (actually this is only used for EOF.
    // does not do any parsing!
    JsonVariantReader(JsonVariantReader *parent, JsonDataType type)
        : JsonParentChildHelper(parent),     //
          mType(type),                       //
          mStream(parent->mStream),          //
          mNestLevel(parent->mNestLevel + 1) //
    {
    }

    // typical outside caller construction
    explicit JsonVariantReader(TextStream &s) : JsonParentChildHelper(nullptr), mStream(&s)
    {
        ConstructParse();
    }

    virtual ~JsonVariantReader()
    {
        OnDestructor();
    }

    bool IsEOF() const
    {
        return mType == JsonDataType::EndOfList;
    }

    String ToString() const
    {
        String ret;
        for (int i = 0; i < mNestLevel; ++i)
        {
            ret += "  ";
        }
        if (mKeyName.length())
        {
            ret += String("key:") + mKeyName + " ";
        }
        ret += mParseResult.ToString();

        switch (mType)
        {
        case JsonDataType::EndOfList:
            return ret + "<end-of-list>";
        case JsonDataType::Undefined:
            return ret + String("<undefined, invalid>");
        case JsonDataType::Null:
            return ret + "<null>";
        case JsonDataType::Boolean:
            return ret + "<boolean>:" + (mBooleanValue ? "true" : "false");
        case JsonDataType::String:
            return ret + "<string>:" + mStringValue;
        case JsonDataType::Number:
            return ret + String("<number>:") + mNumericValue.ToString();
        case JsonDataType::Array:
            return ret + "<array>";
        case JsonDataType::Object:
            return ret + "<object>";
        default:
            return "Unknown type; fatal error";
        }
    }

    void ConstructParse()
    {
        this->OnStreamAccess();
        // read to understand type (obj, array, val)
        AdvancePastWhitespace();
        int ch = mStream->readByte();
        if (ch < 0)
        {
            mParseResult = Result::Failure("EOF");
            mType = JsonDataType::EndOfList;
            return;
        }
        switch (ch)
        {
        case '[':
            IAmAnArray();
            break;
        case '{':
            IAmAnObject();
            break;
        case '\"':
            IAmADoubleQuotedString();
            break;
        case '+':
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            IAmANumber(ch);
            break;
        case 'n':
            IAmNull();
            break;
        case 't':
            IAmTrue();
            break;
        case 'f':
            IAmFalse();
            break;
        default:
            mParseResult = Result::Failure(String("Unknown char @ ") + GetFilePosInfoString());
        }
    }

    static bool CharExistsInString(char ch, const char *chars)
    {
        for (const char *p = chars; *p != 0; ++p)
        {
            if (ch == *p)
                return true;
        }
        return false;
    }

    String ReadUntilNotOf(const char *chars)
    {
        String ret;
        while (true)
        {
            int ch = mStream->peekByte();
            if (!CharExistsInString(ch, chars))
            {
                return ret;
            }
            ret.append((char)ch);
            mStream->readByte();
        }
        return ret;
    }

    virtual void EnsureClosed() override
    {
        if (JsonParentChildHelper::EnsureChildrenClosed())
            return;
        if (mType == JsonDataType::Object)
        {
            Object_Close();
            return;
        }
        if (mType == JsonDataType::Array)
        {
            Array_Close();
            return;
        }

        // other single-value datatypes have already been written and should be locked.
        CCASSERT(false);
    }
    Result Object_Close()
    {
        while (true)
        {
            auto x = this->GetNextObjectItem();
            if (x.mParseResult.IsFailure())
                return x.mParseResult;
            if (x.IsEOF())
                break;
        }
        return Result::Success();
    }

    Result Array_Close()
    {
        while (true)
        {
            auto x = this->GetNextArrayItem();
            if (x.mParseResult.IsFailure())
                return x.mParseResult;
            if (x.IsEOF())
                break;
        }
        return Result::Success();
    }

    // https://www.json.org/json-en.html
    // https://stackoverflow.com/questions/19554972/json-standard-floating-point-numbers
    void IAmANumber(char ch)
    {
        String s{ch};
        s += ReadUntilNotOf("-+0123456789eE.");
        // sign, numbers, decimal, numbers, 'e', sign, numbers
        // read until none of the above
        if (!CCJSON::parseNumber(s.c_str(), mNumericValue))
        {
            mParseResult = Result::Failure(String("bad number format @ ") + GetFilePosInfoString());
        }

        mType = JsonDataType::Number;
        mIsLocked = true;
    }

    void IAmAnArray()
    {
        // we have read the [.
        // so there's nothing to do; caller should read elements one by one.
        mType = JsonDataType::Array;
        mParseResult = Result::Success();
    }

    // array support functions

    // returns an object which will represent either:
    // - a successfully parsed child object
    // - EOF
    // - parse error
    // we have just read either a previous array value, or a '['.
    // for (JsonVariantReader item = GetNextArrayItem(); item.IsEOF(); item = arr.GetNextArrayItem())
    // {
    // }
    JsonVariantReader GetNextArrayItem()
    {
        CCASSERT(!mIsLocked);
        this->OnStreamAccess();
        AdvancePastWhitespace();

        int ch = mStream->peekByte();
        if (ch <= 0)
            return {this, Result::Failure("incomplete array")};

        if (ch == ']')
        {
            mStream->readByte();
            mIsLocked = true;
            return {this, JsonDataType::EndOfList};
        }

        // expect comma?
        if (mHasReadAnElement)
        {
            Result ret = RequireFromStream(",");
            if (ret.IsFailure())
            {
                return {this, ret};
            }
            AdvancePastWhitespace();
        }
        mHasReadAnElement = true;
        ExpectChildStreamAccess();
        return JsonVariantReader(this);
    }

    void IAmAnObject()
    {
        // we have read the {.
        // so there's nothing to do; caller should read elements one by one.
        mType = JsonDataType::Object;
        mParseResult = Result::Success();
    }

    // returns an object which will represent either:
    // - a successfully parsed child object
    // - EOF
    // - parse error
    // we have just read either a previous array value, or a '['.
    // for (auto item = GetNextObjectItem(); item.IsEOF(); item = arr.GetNextObjectItem())
    // {
    // }
    JsonVariantReader GetNextObjectItem()
    {
        CCASSERT(!mIsLocked);
        this->OnStreamAccess();
        AdvancePastWhitespace();

        int ch = mStream->peekByte();
        if (ch <= 0)
        {
            return {this, Result::Failure("incomplete object")};
        }

        if (ch == '}')
        {
            mStream->readByte();
            mIsLocked = true;
            return {this, JsonDataType::EndOfList};
        }

        // expect comma?
        if (mHasReadAnElement)
        {
            Result ret = RequireFromStream(",");
            if (ret.IsFailure())
            {
                return {this, ret};
            }
            AdvancePastWhitespace();
        }

        // read name
        String keyName;
        Result ret = RequireQuotedString(keyName);
        if (ret.IsFailure())
        {
            return {this, ret};
        }
        AdvancePastWhitespace();
        ret = RequireFromStream(":");
        if (ret.IsFailure())
        {
            return {this, ret};
        }
        AdvancePastWhitespace();

        mHasReadAnElement = true;
        ExpectChildStreamAccess();
        return {this, keyName};
    }

    void IAmNull()
    {
        mParseResult = RequireFromStream("ull");
        if (mParseResult.IsSuccess())
        {
            mType = JsonDataType::Null;
        }
        mIsLocked = true;
    }

    void IAmTrue()
    {
        mParseResult = RequireFromStream("rue");
        if (mParseResult.IsSuccess())
        {
            mType = JsonDataType::Boolean;
        }
        mIsLocked = true;
    }

    void IAmFalse()
    {
        mParseResult = RequireFromStream("alse");
        if (mParseResult.IsSuccess())
        {
            mType = JsonDataType::Boolean;
        }
        mIsLocked = true;
    }

    Result RequireQuotedString(String &outp)
    {
        int ch = mStream->readByte();
        if (ch <= 0)
            return Result::Failure("EOF expecting quoted string");
        if (ch == '\"')
            return ParseQuotedString(outp);
        return Result::Failure(String("unexpected char; expected quote @ ") + GetFilePosInfoString());
    }

    Result ParseQuotedString(String &stringStorage)
    {
        stringStorage = "";
        for (;;)
        {
            int c = mStream->readByte();
            if (c == '\"')
                break;

            if (c <= 0)
            {
                return Result::Failure("IncompleteInput");
            }

            if (c == '\\')
            {
                c = mStream->readByte();
                if (c <= 0)
                {
                    return Result::Failure("IncompleteInput");
                }

                if (c == 'u')
                {
                    // unsupported unicode
                    stringStorage.append('\\');
                    continue;
                }

                c = CCJSON::EscapeSequence::unescapeChar(c);
                if (c <= 0)
                {
                    return Result::Failure("IncompleteInput");
                }
            }

            stringStorage.append((char)c);
        }

        return Result::Success();
    }

    void IAmADoubleQuotedString()
    {
        mParseResult = ParseQuotedString(mStringValue);
        if (mParseResult.IsSuccess())
        {
            mType = JsonDataType::String;
        }
        mIsLocked = true;
    }

    // json grammar:
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON#:~:text=Full%20JSON%20grammar&text=The%20tab%20character%20(U%2B0009,the%20only%20valid%20whitespace%20characters.
    static bool IsWhitespace(char c)
    {
        switch (c)
        {
        case 0x20:
        case 0x09:
        case 0x0d:
        case 0x0a:
            return true;
        default:
            return false;
        }
    }

    // template<size_t N>
    Result RequireFromStream(const char *token)
    {
        for (const char *p = token; *p != 0; ++p)
        {
            char ch = *p;
            if (ch == 0)
                break; // char literal null term
            int ich = mStream->readByte();
            if (ich < 0)
                return Result::Failure(String("EOF before end of token @ ") + GetFilePosInfoString());
            if (ch != ich)
                return Result::Failure(String("Unexpected char @ ") + GetFilePosInfoString());
        }
        return Result::Success();
    }

    void AdvancePastWhitespace()
    {
        while (true)
        {
            int ich = mStream->peekByte();
            if (ich < 0)
            {
                return; // EOF
            }
            if (!IsWhitespace(ich))
            {
                return; // success
            }
            mStream->readByte();
        }
    }

    String GetFilePosInfoString() const
    {
        return String("[") + mStream->mLine + ":" + mStream->mColumn + "]";
    }

}; // JsonVariantReader

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SerializationMapping
{
    void *const mpvParam;
    const char *const mKey;
    bool mVisited = false;

    // these are required to be stored here because the serializer doesn't know which types it's serializing;
    // they are only known to the caller when constructing this object.
    cc::function<Result(JsonVariantWriter &myval, void *pparam)>::ptr_t const mSerializeFn;
    cc::function<Result(JsonVariantReader &myval, void *pparam)>::ptr_t const mDeserializeFn;

    constexpr SerializationMapping(void *pparam,
                                   const char *key,
                                   cc::function<Result(JsonVariantWriter &, void *pparam)>::ptr_t serializeFn,
                                   cc::function<Result(JsonVariantReader &, void *pparam)>::ptr_t deserializeFn)
        : mpvParam(pparam), mKey(key), mSerializeFn(serializeFn), mDeserializeFn(deserializeFn)
    {
    }

    template <typename T>
    T &GetParam()
    {
        return *((T *)mpvParam);
    }
};

template <size_t N> // sad.
struct SerializationObjectMap
{
    std::array<SerializationMapping, N> mMap;

    constexpr SerializationObjectMap(std::array<SerializationMapping, N> &&map) : mMap{map}
    {
    }

    using iterator = typename std::array<SerializationMapping, N>::iterator;
    iterator begin()
    {
        return mMap.begin();
    }
    iterator end()
    {
        return mMap.end();
    }

    Result Serialize(JsonVariantWriter &myval)
    {
        Result ret = Result::Success();
        for (auto &mapping : mMap)
        {
            auto ch = myval.Object_MakeKey(mapping.mKey);
            ret.AndRequires(mapping.mSerializeFn(ch, mapping.mpvParam),
                            String("error @ ") + myval.GetFilePosInfoString());
        }
        return ret;
    }

    Result Deserialize(JsonVariantReader &myval)
    {
        if (myval.mType != JsonDataType::Object)
        {
            return Result::Failure(String("expected object type @ ") + myval.GetFilePosInfoString());
        }
        // std::cout << myval.ToString().mStr << std::endl;
        Result ret = Result::Success();
        for (auto &mapping : mMap)
        {
            mapping.mVisited = false;
        }
        // for (auto ch = myval.GetNextObjectItem(); !ch.IsEOF(); ch = myval.GetNextObjectItem())
        while (true)
        {
            auto ch = myval.GetNextObjectItem();
            if (ch.IsEOF())
                break;
            if (ch.mParseResult.IsFailure())
            {
                return ch.mParseResult;
            }
            // find the mapping corresponding to this object key.
            bool wasParsed = false;
            for (auto &mapping : mMap)
            {
                if (ch.mKeyName == mapping.mKey)
                {
                    if (mapping.mVisited)
                    {
                        ret.AddPrefix(String("Duplicate key in input: ") + ch.mKeyName + " @ " +
                                      myval.GetFilePosInfoString());
                    }
                    Result chret = mapping.mDeserializeFn(ch, mapping.mpvParam);
                    if (chret.IsFailure())
                        return chret;
                    wasParsed = true;
                    mapping.mVisited = true;
                    break;
                }
            }
            if (!wasParsed)
            {
                ret.AddPrefix(String("Input key was ignored: ") + ch.mKeyName + " @ " + myval.GetFilePosInfoString());
            }
        }
        for (auto &mapping : mMap)
        {
            if (!mapping.mVisited)
            {
                ret.AddPrefix(String("Key not found in input: ") + mapping.mKey + " @ " + myval.GetFilePosInfoString());
            }
        }
        return ret;
    }
};

template <typename Tparam>
SerializationMapping CreateSerializationMapping(Tparam &param, const char *key)
{
    return {&param,
            key,
            [](JsonVariantWriter &myval, void *pparam) FLASHMEM {
                Tparam &param{*((Tparam *)pparam)};
                return Serialize(myval, param);
            },
            [](JsonVariantReader &myval, void *pparam)FLASHMEM {
                Tparam &param{*((Tparam *)pparam)};
                // std::cout << myval.ToString().mStr << std::endl;
                return Deserialize(myval, param);
            }};
}

// allows quick object mapping like,
// float a = 1.12f;
// int b = 42;
// SerializationObjectMap<2> map{{
//   CreateSerializationMapping(a, "a"),
//   CreateSerializationMapping(b, "b"),
// }};
// auto r = Serialize(doc, map);
template <size_t N>
Result Serialize(JsonVariantWriter &myval, SerializationObjectMap<N> &map)
{
    return map.Serialize(myval);
}

template <size_t N>
Result Deserialize(JsonVariantReader &myval, SerializationObjectMap<N> &map)
{
    return map.Deserialize(myval);
}

// allows auto-serializing keyed objects like
// struct SynthPatch : ISerializationObjectMap<1>
//{
//	virtual SerializationObjectMapArray GetSerializationObjectMap() override
//	{
//		return { { CreateSerializationMapping(mDetune, "det"), ... } };
//	}
//};
template <size_t N>
struct ISerializationObjectMap
{
    using SerializationObjectMapArray = SerializationObjectMap<N>;
    virtual SerializationObjectMapArray GetSerializationObjectMap() = 0;
};

template <size_t N>
static Result Serialize(JsonVariantWriter &myval, ISerializationObjectMap<N> &x)
{
    return x.GetSerializationObjectMap().Serialize(myval);
}

template <size_t N>
static Result Deserialize(JsonVariantReader &myval, ISerializationObjectMap<N> &x)
{
    return x.GetSerializationObjectMap().Deserialize(myval);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
Result Serialize(JsonVariantWriter &myval, std::array<T, N> &param)
{
    Result ret = Result::Success();
    for (auto &i : param)
    {
        auto v = myval.Array_MakeValue();
        Serialize(v, i);
    }
    // myval.Array_Close();
    return ret;
}

template <typename T, size_t N>
Result Deserialize(JsonVariantReader &myval, std::array<T, N> &param)
{
    if (myval.mType != JsonDataType::Array)
    {
        return Result::Failure(String("expected array") + myval.GetFilePosInfoString());
    }
    // std::cout << myval.ToString().mStr << std::endl;
    size_t arrayIndex = 0;
    // for (auto ch = myval.GetNextArrayItem(); !ch.IsEOF(); ch = myval.GetNextArrayItem())
    while (true)
    {
        auto ch = myval.GetNextArrayItem();
        if (ch.IsEOF())
        {
            break;
        }
        if (arrayIndex >= N)
        {
            return Result::Success(String("More items than expected in input @ ") + myval.GetFilePosInfoString());
        }
        Result chret = Deserialize(ch, param[arrayIndex]);
        arrayIndex++;
    }
    if (arrayIndex < N)
    {
        return Result::Success(String("Too few array items in input @ ") + myval.GetFilePosInfoString());
    }
    return Result::Success();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// any serializable object provide these 2 overloads
Result Serialize(JsonVariantWriter &myval, bool &param)
{
    return myval.WriteBoolean(param);
}

Result Deserialize(JsonVariantReader &myval, bool &param)
{
    if (myval.mType != JsonDataType::Boolean)
    {
        return Result::Failure("expected bool");
    }
    param = myval.mBooleanValue;
    return Result::Success();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// any serializable object provide these 2 overloads
template <typename T, std::enable_if_t<std::is_floating_point<T>::value || std::is_integral<T>::value, int> = 0>
Result Serialize(JsonVariantWriter &myval, T &param)
{
    return myval.WriteNumberValue(param);
}

template <typename T, std::enable_if_t<std::is_floating_point<T>::value || std::is_integral<T>::value, int> = 0>
Result Deserialize(JsonVariantReader &myval, T &param)
{
    if (myval.mType != JsonDataType::Number)
    {
        return Result::Failure("expected number");
    }
    param = myval.mNumericValue.Get<T>();
    return Result::Success();
}

static constexpr auto aoeue = sizeof(JsonVariantReader);

} // namespace clarinoid