#include <cstdint>

int16_t src1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
int16_t src2[8] = {8, 7, 6, 5, 4, 3, 2, 1};
int16_t dest[8];

int test() {
  asm volatile(
    "vld1.16 {q0}, [%[src1]]\n" // load src1 into q0
    "vld1.16 {q1}, [%[src2]]\n" // load src2 into q1
    "vaddq_s16 q0, q0, q1\n" // add q0 and q1 using the VADD instruction
    "vst1.16 {q0}, [%[dest]]\n" // store the result in dest
    : // no output
    : [src1] "r" (src1), [src2] "r" (src2), [dest] "r" (dest) // input
    : "q0", "q1" // clobbered registers
  );
  return 0;
}

void setup(){

}

void loop() {
  
}
