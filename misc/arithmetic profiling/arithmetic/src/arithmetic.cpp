
#include <Arduino.h>

volatile int y = 0;

int Overhead(int a, int b)
{
  return 0;
}


// MUL ////////////////////////////////////////////////

int8_t Mul_S8(int8_t a, int8_t b)
{
  return a * b;
}

uint8_t Mul_U8(uint8_t a, uint8_t b)
{
  return a * b;
}

int16_t Mul_S16(int16_t a, int16_t b)
{
  return a * b;
}

uint16_t Mul_U16(uint16_t a, uint16_t b)
{
  return a * b;
}

int32_t Mul_S32(int32_t a, int32_t b)
{
  return a * b;
}

uint32_t Mul_U32(uint32_t a, uint32_t b)
{
  return a * b;
}

int64_t Mul_S64(int64_t a, int64_t b)
{
  return a * b;
}

uint64_t Mul_U64(uint64_t a, uint64_t b)
{
  return a * b;
}

float Mul_F(float a, float b)
{
  return a * b;
}

double Mul_D(double a, double b)
{
  return a * b;
}

// DIV ////////////////////////////////////////////////

int8_t Div_S8(int8_t a, int8_t b)
{
  return a / b;
}

uint8_t Div_U8(uint8_t a, uint8_t b)
{
  return a / b;
}

int16_t Div_S16(int16_t a, int16_t b)
{
  return a / b;
}

uint16_t Div_U16(uint16_t a, uint16_t b)
{
  return a / b;
}

int32_t Div_S32(int32_t a, int32_t b)
{
  return a / b;
}

uint32_t Div_U32(uint32_t a, uint32_t b)
{
  return a / b;
}

int64_t Div_S64(int64_t a, int64_t b)
{
  return a / b;
}

uint64_t Div_U64(uint64_t a, uint64_t b)
{
  return a / b;
}

float Div_F(float a, float b)
{
  return a / b;
}

double Div_D(double a, double b)
{
  return a / b;
}

// ADD ////////////////////////////////////////////////

int8_t Add_S8(int8_t a, int8_t b)
{
  return a + b;
}

uint8_t Add_U8(uint8_t a, uint8_t b)
{
  return a + b;
}

int16_t Add_S16(int16_t a, int16_t b)
{
  return a + b;
}

uint16_t Add_U16(uint16_t a, uint16_t b)
{
  return a + b;
}

int32_t Add_S32(int32_t a, int32_t b)
{
  return a + b;
}

uint32_t Add_U32(uint32_t a, uint32_t b)
{
  return a + b;
}

int64_t Add_S64(int64_t a, int64_t b)
{
  return a + b;
}

uint64_t Add_U64(uint64_t a, uint64_t b)
{
  return a + b;
}

double Add_D(double a, double b)
{
  return a + b;
}

float Add_F(float a, float b)
{
  return a + b;
}


// SUB ////////////////////////////////////////////////

int8_t Sub_S8(int8_t a, int8_t b)
{
  return a - b;
}

uint8_t Sub_U8(uint8_t a, uint8_t b)
{
  return a - b;
}

int16_t Sub_S16(int16_t a, int16_t b)
{
  return a - b;
}

uint16_t Sub_U16(uint16_t a, uint16_t b)
{
  return a - b;
}

int32_t Sub_S32(int32_t a, int32_t b)
{
  return a - b;
}

uint32_t Sub_U32(uint32_t a, uint32_t b)
{
  return a - b;
}

double Sub_D(double a, double b)
{
  return a - b;
}

int64_t Sub_S64(int64_t a, int64_t b)
{
  return a - b;
}

uint64_t Sub_U64(uint64_t a, uint64_t b)
{
  return a - b;
}

float Sub_F(float a, float b)
{
  return a - b;
}



// TO32 ////////////////////////////////////////////////

int32_t To32_S8(int8_t a, int8_t b)
{
  return int32_t(a);
}

uint32_t To32_U8(uint8_t a, uint8_t b)
{
  return uint32_t(a);
}

int32_t To32_S16(int16_t a, int16_t b)
{
  return int32_t(a);
}

uint32_t To32_U16(uint16_t a, uint16_t b)
{
  return uint32_t(a);
}

int32_t To32_S32(int32_t a, int32_t b)
{
  return int32_t(a);
}

uint32_t To32_U32(uint32_t a, uint32_t b)
{
  return uint32_t(a);
}

int32_t To32_D(double a, double b)
{
  return int32_t(a);
}

int32_t To32_S64(int64_t a, int64_t b)
{
  return int32_t(a);
}

uint32_t To32_U64(uint64_t a, uint64_t b)
{
  return uint32_t(a);
}

int32_t To32_F(float a, float b)
{
  return int32_t(a);
}



// FROM 32 ////////////////////////////////////////////////

int8_t From32_S8(int32_t a, int32_t b)
{
  return int8_t(a);
}

uint8_t From32_U8(uint32_t a, uint32_t b)
{
  return uint8_t(a);
}

int16_t From32_S16(int32_t a, int32_t b)
{
  return int16_t(a);
}

uint16_t From32_U16(uint16_t a, uint16_t b)
{
  return uint16_t(a);
}

int32_t From32_S32(int32_t a, int32_t b)
{
  return int32_t(a);
}

uint32_t From32_U32(uint32_t a, uint32_t b)
{
  return uint32_t(a);
}

double From32_D(int32_t a, int32_t b)
{
  return double(a);
}

int64_t From32_S64(int32_t a, int32_t b)
{
  return int64_t(a);
}

uint64_t From32_U64(uint32_t a, uint32_t b)
{
  return uint64_t(a);
}

float From32_F(int32_t a, int32_t b)
{
  return float(a);
}




// SHL ////////////////////////////////////////////////

int8_t Shl_S8(int8_t a, int8_t b)
{
  return a << b;
}

uint8_t Shl_U8(uint8_t a, uint8_t b)
{
  return a << b;
}

int16_t Shl_S16(int16_t a, int16_t b)
{
  return a << b;
}

uint16_t Shl_U16(uint16_t a, uint16_t b)
{
  return a << b;
}

int32_t Shl_S32(int32_t a, int32_t b)
{
  return a << b;
}

uint32_t Shl_U32(uint32_t a, uint32_t b)
{
  return a << b;
}

int64_t Shl_S64(int64_t a, int64_t b)
{
  return a << b;
}

uint64_t Shl_U64(uint64_t a, uint64_t b)
{
  return a << b;
}





// Shr ////////////////////////////////////////////////

int8_t Shr_S8(int8_t a, int8_t b)
{
  return a >> b;
}

uint8_t Shr_U8(uint8_t a, uint8_t b)
{
  return a >> b;
}

int16_t Shr_S16(int16_t a, int16_t b)
{
  return a >> b;
}

uint16_t Shr_U16(uint16_t a, uint16_t b)
{
  return a >> b;
}

int32_t Shr_S32(int32_t a, int32_t b)
{
  return a >> b;
}

uint32_t Shr_U32(uint32_t a, uint32_t b)
{
  return a >> b;
}

int64_t Shr_S64(int64_t a, int64_t b)
{
  return a >> b;
}

uint64_t Shr_U64(uint64_t a, uint64_t b)
{
  return a >> b;
}



// and ////////////////////////////////////////////////

int8_t And_S8(int8_t a, int8_t b)
{
  return a & b;
}

uint8_t And_U8(uint8_t a, uint8_t b)
{
  return a & b;
}

int16_t And_S16(int16_t a, int16_t b)
{
  return a & b;
}

uint16_t And_U16(uint16_t a, uint16_t b)
{
  return a & b;
}

int32_t And_S32(int32_t a, int32_t b)
{
  return a & b;
}

uint32_t And_U32(uint32_t a, uint32_t b)
{
  return a & b;
}

int64_t And_S64(int64_t a, int64_t b)
{
  return a & b;
}

uint64_t And_U64(uint64_t a, uint64_t b)
{
  return a & b;
}



// Or ////////////////////////////////////////////////

int8_t Or_S8(int8_t a, int8_t b)
{
  return a | b;
}

uint8_t Or_U8(uint8_t a, uint8_t b)
{
  return a | b;
}

int16_t Or_S16(int16_t a, int16_t b)
{
  return a | b;
}

uint16_t Or_U16(uint16_t a, uint16_t b)
{
  return a | b;
}

int32_t Or_S32(int32_t a, int32_t b)
{
  return a | b;
}

uint32_t Or_U32(uint32_t a, uint32_t b)
{
  return a | b;
}

int64_t Or_S64(int64_t a, int64_t b)
{
  return a | b;
}

uint64_t Or_U64(uint64_t a, uint64_t b)
{
  return a | b;
}




// Mod ////////////////////////////////////////////////

int8_t Mod_S8(int8_t a, int8_t b)
{
  return a % b;
}

uint8_t Mod_U8(uint8_t a, uint8_t b)
{
  return a % b;
}

int16_t Mod_S16(int16_t a, int16_t b)
{
  return a % b;
}

uint16_t Mod_U16(uint16_t a, uint16_t b)
{
  return a % b;
}

int32_t Mod_S32(int32_t a, int32_t b)
{
  return a % b;
}

uint32_t Mod_U32(uint32_t a, uint32_t b)
{
  return a % b;
}

int64_t Mod_S64(int64_t a, int64_t b)
{
  return a % b;
}

uint64_t Mod_U64(uint64_t a, uint64_t b)
{
  return a % b;
}

float Mod_F(float a, float b)
{
  return ::fmodf(a, b);
}

double Mod_D(double a, double b)
{
  return ::fmod(a, b);
}


int gOverheadMicros = 0;

void TestOverhead()
{
  static constexpr int iterations = 100000;
  int x = 0;
  int m1 = micros();
  for (int i = 0; i < iterations; ++ i) {
    x += Overhead(rand(), rand()) + y;
  }
  int m2 = micros();
  int m = m2 - m1;
  gOverheadMicros = m;

  Serial.println(String("OVERHEAD") +
  "  x=" + String(int32_t(x),16) +
  ", micros:" + m
  );
}

template<typename T, typename Tfn>
void Test(const char * testName, const char *op, const char *type, const char *rawtype, const char *signness, Tfn fn)
{
  static constexpr int iterations = 100000;
  T x = 0;
  int m1 = micros();
  for (int i = 0; i < iterations; ++ i) {
    x += fn(rand(), rand()) + y;
  }
  int m2 = micros();
  int m = m2 - m1;
  m -= gOverheadMicros;

  Serial.println(String(testName) +
  "|" + String(int32_t(x),16) + // necessary to prevent optimizing out.
  "|" + op + // necessary to prevent optimizing out.
  "|" + type + // necessary to prevent optimizing out.
  "|" + rawtype + // necessary to prevent optimizing out.
  "|" + signness + // necessary to prevent optimizing out.
  "|" + m
  );
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  srand(99);
  
  TestOverhead();
  TestOverhead();
  TestOverhead();
  TestOverhead();
    
  
  Test<int8_t>("Mul_S8", "Mul", "S8", "i8", "S", &Mul_S8);
  Test<uint8_t>("Mul_U8", "Mul", "U8", "i8", "U", &Mul_U8);
  Test<int16_t>("Mul_S16", "Mul", "S16", "i16", "S", &Mul_S16);
  Test<uint16_t>("Mul_U16", "Mul", "U16", "i16", "U", &Mul_U16);
  Test<int32_t>("Mul_S32", "Mul", "S32", "i32", "S", &Mul_S32);
  Test<uint32_t>("Mul_U32", "Mul", "U32", "i32", "U", &Mul_U32);
  Test<int64_t>("Mul_S64", "Mul", "S64", "i64", "S", &Mul_S64);
  Test<uint64_t>("Mul_U64", "Mul", "U64", "i64", "U", &Mul_U64);
  Test<float>("Mul_F", "Mul", "F", "F", "F", &Mul_F);
  Test<double>("Mul_D", "Mul", "D", "F", "D", &Mul_D);
  Test<int8_t>("Div_S8", "Div", "S8", "i8", "S", &Div_S8);
  Test<uint8_t>("Div_U8", "Div", "U8", "i8", "U", &Div_U8);
  Test<int16_t>("Div_S16", "Div", "S16", "i16", "S", &Div_S16);
  Test<uint16_t>("Div_U16", "Div", "U16", "i16", "U", &Div_U16);
  Test<int32_t>("Div_S32", "Div", "S32", "i32", "S", &Div_S32);
  Test<uint32_t>("Div_U32", "Div", "U32", "i32", "U", &Div_U32);
  Test<int64_t>("Div_S64", "Div", "S64", "i64", "S", &Div_S64);
  Test<uint64_t>("Div_U64", "Div", "U64", "i64", "U", &Div_U64);
  Test<float>("Div_F", "Div", "F", "F", "F", &Div_F);
  Test<double>("Div_D", "Div", "D", "D", "D", &Div_D);
  Test<int8_t>("Add_S8", "Add", "S8", "i8", "S", &Add_S8);
  Test<uint8_t>("Add_U8", "Add", "U8", "i8", "U", &Add_U8);
  Test<int16_t>("Add_S16", "Add", "S16", "i16", "S", &Add_S16);
  Test<uint16_t>("Add_U16", "Add", "U16", "i16", "U", &Add_U16);
  Test<int32_t>("Add_S32", "Add", "S32", "i32", "S", &Add_S32);
  Test<uint32_t>("Add_U32", "Add", "U32", "i32", "U", &Add_U32);
  Test<int64_t>("Add_S64", "Add", "S64", "i64", "S", &Add_S64);
  Test<uint64_t>("Add_U64", "Add", "U64", "i64", "U", &Add_U64);
  Test<float>("Add_F", "Add", "F", "F", "F", &Add_F);
  Test<double>("Add_D", "Add", "D", "D", "D", &Add_D);
  Test<int8_t>("Sub_S8", "Sub", "S8", "i8", "S", &Sub_S8);
  Test<uint8_t>("Sub_U8", "Sub", "U8", "i8", "U", &Sub_U8);
  Test<int16_t>("Sub_S16", "Sub", "S16", "i16", "S", &Sub_S16);
  Test<uint16_t>("Sub_U16", "Sub", "U16", "i16", "U", &Sub_U16);
  Test<int32_t>("Sub_S32", "Sub", "S32", "i32", "S", &Sub_S32);
  Test<uint32_t>("Sub_U32", "Sub", "U32", "i32", "U", &Sub_U32);
  Test<int64_t>("Sub_S64", "Sub", "S64", "i64", "S", &Sub_S64);
  Test<uint64_t>("Sub_U64", "Sub", "U64", "i64", "U", &Sub_U64);
  Test<float>("Sub_F", "Sub", "F", "F", "F", &Sub_F);
  Test<double>("Sub_D", "Sub", "D", "D", "D", &Sub_D);
  
  // to 32
  Test<int8_t>("To32_S8", "To32", "S8", "i8", "S", &To32_S8);
  Test<uint8_t>("To32_U8", "To32", "U8", "i8", "U", &To32_U8);
  Test<int16_t>("To32_S16", "To32", "S16", "i16", "S", &To32_S16);
  Test<uint16_t>("To32_U16", "To32", "U16", "i16", "U", &To32_U16);
  Test<int32_t>("To32_S32", "To32", "S32", "i32", "S", &To32_S32);
  Test<uint32_t>("To32_U32", "To32", "U32", "i32", "U", &To32_U32);
  Test<int64_t>("To32_S64", "To32", "S64", "i64", "S", &To32_S64);
  Test<uint64_t>("To32_U64", "To32", "U64", "i64", "U", &To32_U64);
  Test<float>("To32_F", "To32", "F", "F", "F", &To32_F);
  Test<double>("To32_D", "To32", "D", "D", "D", &To32_D);

  // from 32
  Test<int8_t>("From32_S8", "From32", "S8", "i8", "S", &From32_S8);
  Test<uint8_t>("From32_U8", "From32", "U8", "i8", "U", &From32_U8);
  Test<int16_t>("From32_S16", "From32", "S16", "i16", "S", &From32_S16);
  Test<uint16_t>("From32_U16", "From32", "U16", "i16", "U", &From32_U16);
  Test<int32_t>("From32_S32", "From32", "S32", "i32", "S", &From32_S32);
  Test<uint32_t>("From32_U32", "From32", "U32", "i32", "U", &From32_U32);
  Test<int64_t>("From32_S64", "From32", "S64", "i64", "S", &From32_S64);
  Test<uint64_t>("From32_U64", "From32", "U64", "i64", "U", &From32_U64);
  Test<float>("From32_F", "From32", "F", "F", "F", &From32_F);
  Test<double>("From32_D", "From32", "D", "D", "D", &From32_D);

  // shl
  Test<int8_t>("Shl_S8", "Shl", "S8", "i8", "S", &Shl_S8);
  Test<uint8_t>("Shl_U8", "Shl", "U8", "i8", "U", &Shl_U8);
  Test<int16_t>("Shl_S16", "Shl", "S16", "i16", "S", &Shl_S16);
  Test<uint16_t>("Shl_U16", "Shl", "U16", "i16", "U", &Shl_U16);
  Test<int32_t>("Shl_S32", "Shl", "S32", "i32", "S", &Shl_S32);
  Test<uint32_t>("Shl_U32", "Shl", "U32", "i32", "U", &Shl_U32);
  Test<int64_t>("Shl_S64", "Shl", "S64", "i64", "S", &Shl_S64);
  Test<uint64_t>("Shl_U64", "Shl", "U64", "i64", "U", &Shl_U64);

  // shr
  Test<int8_t>("Shr_S8", "Shr", "S8", "i8", "S", &Shr_S8);
  Test<uint8_t>("Shr_U8", "Shr", "U8", "i8", "U", &Shr_U8);
  Test<int16_t>("Shr_S16", "Shr", "S16", "i16", "S", &Shr_S16);
  Test<uint16_t>("Shr_U16", "Shr", "U16", "i16", "U", &Shr_U16);
  Test<int32_t>("Shr_S32", "Shr", "S32", "i32", "S", &Shr_S32);
  Test<uint32_t>("Shr_U32", "Shr", "U32", "i32", "U", &Shr_U32);
  Test<int64_t>("Shr_S64", "Shr", "S64", "i64", "S", &Shr_S64);
  Test<uint64_t>("Shr_U64", "Shr", "U64", "i64", "U", &Shr_U64);

  // and
  Test<int8_t>("And_S8", "And", "S8", "i8", "S", &And_S8);
  Test<uint8_t>("And_U8", "And", "U8", "i8", "U", &And_U8);
  Test<int16_t>("And_S16", "And", "S16", "i16", "S", &And_S16);
  Test<uint16_t>("And_U16", "And", "U16", "i16", "U", &And_U16);
  Test<int32_t>("And_S32", "And", "S32", "i32", "S", &And_S32);
  Test<uint32_t>("And_U32", "And", "U32", "i32", "U", &And_U32);
  Test<int64_t>("And_S64", "And", "S64", "i64", "S", &And_S64);
  Test<uint64_t>("And_U64", "And", "U64", "i64", "U", &And_U64);

  // or
  Test<int8_t>("Or_S8", "Or", "S8", "i8", "S", &Or_S8);
  Test<uint8_t>("Or_U8", "Or", "U8", "i8", "U", &Or_U8);
  Test<int16_t>("Or_S16", "Or", "S16", "i16", "S", &Or_S16);
  Test<uint16_t>("Or_U16", "Or", "U16", "i16", "U", &Or_U16);
  Test<int32_t>("Or_S32", "Or", "S32", "i32", "S", &Or_S32);
  Test<uint32_t>("Or_U32", "Or", "U32", "i32", "U", &Or_U32);
  Test<int64_t>("Or_S64", "Or", "S64", "i64", "S", &Or_S64);
  Test<uint64_t>("Or_U64", "Or", "U64", "i64", "U", &Or_U64);

  // mod
  Test<int8_t>("Mod_S8", "Mod", "S8", "i8", "S", &Mod_S8);
  Test<uint8_t>("Mod_U8", "Mod", "U8", "i8", "U", &Mod_U8);
  Test<int16_t>("Mod_S16", "Mod", "S16", "i16", "S", &Mod_S16);
  Test<uint16_t>("Mod_U16", "Mod", "U16", "i16", "U", &Mod_U16);
  Test<int32_t>("Mod_S32", "Mod", "S32", "i32", "S", &Mod_S32);
  Test<uint32_t>("Mod_U32", "Mod", "U32", "i32", "U", &Mod_U32);
  Test<int64_t>("Mod_S64", "Mod", "S64", "i64", "S", &Mod_S64);
  Test<uint64_t>("Mod_U64", "Mod", "U64", "i64", "U", &Mod_U64);
  Test<float>("Mod_F", "Mod", "F", "F", "F", &Mod_F);
  Test<double>("Mod_D", "Mod", "D", "D", "D", &Mod_D);


}

void loop() {
  
}
