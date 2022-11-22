/*

arduino:

#include <cmath>

using Real = float;

static constexpr Real pi2 = Real(2.0) * Real(3.1415926535897932385);

Real slider1 = 0.6f;

inline Real ccsin(Real x)
{
  return std::sin(x);
}


static void outp(Real acc1, Real fb1, Real sda1ample, Real samfbmulple, Real phase, Real s)
{
    Serial.println(
      String(acc1, 6) + "\t" +//
    String(fb1, 6) + "\t" +//
    String(sda1ample, 6) + "\t" +//
    String(samfbmulple, 6) + "\t" +//
    String(phase, 6) + "\t" +//
    String(s, 6) //
    );
}


void loop() {}



Real factorial(int x) // calculates the factorial
{
    Real fact = 1;
    for (; x >= 1; x--)
    {
        fact = x * fact;
    }
    return fact;
}

Real power(Real x, Real n) // calculates the power of x
{
    double output = 1;
    while (n > 0)
    {
        output = (x * output);
        n--;
    }
    return output;
}

Real taylorsin(Real radians) // value of sine by Taylors series
{
    Real a, b, c;
    Real result = 0;
    for (int y = 0; y != 9; y++)
    {
        a = power(-1, y);
        b = power(radians, (2 * y) + 1);
        c = factorial((2 * y) + 1);
        result = result + (a * b) / c;
    }
    return result;
}

void setup() {
  Serial.begin(9600);
  while(!Serial);


    Real fbN1 = 0;
    Real acc = 0;
    Real da = 512.0 / 32768.0;

    size_t samplesToRecord = 128;

    for (size_t i = 0; i < samplesToRecord; ++i)
    {
        Real acc1 = acc;
        Real fb1 = fbN1;
        Real da1 = da;

        acc += da;

        Real fbmul = (fbN1 * 3.14);
        Real phase = acc + fbmul;
        Real s = taylorsin(phase);
        fbN1 = s;

        outp(acc1, fb1, da1, fbmul, phase, s);
    }




}




*/

#include <cmath>
#include <iostream>
#include <string>
#include <sstream>

#include <Windows.h>

using Real = float;

static constexpr Real pi2 = Real(2.0) * Real(3.1415926535897932385);

Real slider1 = 0.6f;

inline Real ccsin(Real x)
{
    return std::sin(x);
}

//outp(acc1, fb1, da1, fbmul, phase, s);
static void outp(Real acc1, Real fb1, Real sda1ample, Real samfbmulple, Real phase, Real s)
{
    std::stringstream ss;
    const char tab[] = "\t";
    ss << //
        acc1 << tab << //
        fb1 << tab <<  //
        sda1ample << tab << //
        samfbmulple << tab << //
        phase << tab <<       //
        s <<  //
        std::endl;
    ::OutputDebugStringA(ss.str().c_str());
    std::cout << ss.str();
}



Real factorial(int x) // calculates the factorial
{
    Real fact = 1;
    for (; x >= 1; x--)
    {
        fact = x * fact;
    }
    return fact;
}

Real power(Real x, Real n) // calculates the power of x
{
    double output = 1;
    while (n > 0)
    {
        output = (x * output);
        n--;
    }
    return output;
}

Real taylorsin(Real radians) // value of sine by Taylors series
{
    Real a, b, c;
    Real result = 0;
    for (int y = 0; y != 9; y++)
    {
        a = power(-1, y);
        b = power(radians, (2 * y) + 1);
        c = factorial((2 * y) + 1);
        result = result + (a * b) / c;
    }
    return result;
}


int main()
{
    Real fbN1 = 0;
    Real acc = 0;
    Real da = 512.0 / 32768.0;

    size_t samplesToRecord = 128;

    for (size_t i = 0; i < samplesToRecord; ++i)
    {
        Real acc1 = acc;
        Real fb1 = fbN1;
        Real da1 = da;

        acc += da;

        Real fbmul = (fbN1 * 3.14);
        Real phase = acc + fbmul;
        Real s = taylorsin(phase);
        fbN1 = s;
        
        outp(acc1, fb1, da1, fbmul, phase, s);
    }
}
