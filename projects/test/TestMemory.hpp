
#pragma once

#include <clarinoid/basic/Basic.hpp>

#include "Test.hpp"

uint8_t gTempBuffer10[10];

void TestBufferUnification()
{
  {
    uint8_t b[] = "Zabcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
    auto s = clarinoid::UnifyCircularBuffer_Left<10>(b + 1, clarinoid::EndPtr(b) - 1, b, b + 1, gTempBuffer10);
    Test(strcmp("abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789Z", (const char*)b) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "4567123";
    auto s = clarinoid::UnifyCircularBuffer_Left(b + 4, clarinoid::EndPtr(b) - 1, b, b + 4, gTempBuffer10);
    Test(strncmp("1234567", (const char*)b, 7) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "456123";
    clarinoid::UnifyCircularBuffer_Left(b + 3, clarinoid::EndPtr(b) - 1, b, b + 3, gTempBuffer10);
    Test(strncmp("123456", (const char*)b, 6) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "45123";
    clarinoid::UnifyCircularBuffer_Left(b + 2, clarinoid::EndPtr(b) - 1, b, b + 2, gTempBuffer10);
    Test(strncmp("12345", (const char*)b, 5) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "4123";
    clarinoid::UnifyCircularBuffer_Left(b + 1, clarinoid::EndPtr(b) - 1, b, b + 1, gTempBuffer10);
    Test(strncmp("1234", (const char*)b, 4) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }


  {
    uint8_t b[] = "34..12";
    clarinoid::UnifyCircularBuffer_Left(b + 4, clarinoid::EndPtr(b) - 1, b, b + 2, gTempBuffer10);
    Test(strncmp("1234", (const char*)b, 4) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "3..12";
    clarinoid::UnifyCircularBuffer_Left(b + 3, clarinoid::EndPtr(b) - 1, b, b + 1, gTempBuffer10);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "23..1";
    clarinoid::UnifyCircularBuffer_Left(b + 4, clarinoid::EndPtr(b) - 1, b, b + 2, gTempBuffer10);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "9..12345678";
    clarinoid::UnifyCircularBuffer_Left(b + 3, clarinoid::EndPtr(b) - 1, b, b + 1, gTempBuffer10);
    Test(strncmp("123456789", (const char*)b, 9) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "23456789..1";
    clarinoid::UnifyCircularBuffer_Left(b + 10, clarinoid::EndPtr(b) - 1, b, b + 8, gTempBuffer10);
    Test(strncmp("123456789", (const char*)b, 9) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "456789.123";
    clarinoid::UnifyCircularBuffer_Left(b + 7, clarinoid::EndPtr(b) - 1, b, b + 6, gTempBuffer10);
    Test(strncmp("123456789", (const char*)b, 9) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0); // no overruns.
  }

  // test 0-sized segments.
  {
    uint8_t b[] = "123";
    clarinoid::UnifyCircularBuffer_Left(b, clarinoid::EndPtr(b) - 1, b, b, gTempBuffer10);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "123";
    clarinoid::UnifyCircularBuffer_Left(clarinoid::EndPtr(b) - 1, clarinoid::EndPtr(b) - 1, b, clarinoid::EndPtr(b) - 1, gTempBuffer10);
    Test(strncmp("123", (const char*)b, 3) == 0);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }
  {
    uint8_t b[] = "";
    clarinoid::UnifyCircularBuffer_Left(b, clarinoid::EndPtr(b) - 1, 0, 0, gTempBuffer10);
    Test(*(clarinoid::EndPtr(b) - 1) == 0);
  }

}



void TestDivRem()
{
  size_t whole;
  uint32_t rem;
  clarinoid::DivRemBitwise<8>(0x200, whole, rem);
  Test(whole == 2 && rem == 2);
  clarinoid::DivRemBitwise<8>(0x234, whole, rem);
  Test(whole == 2 && rem == 0x36);
  clarinoid::DivRemBitwise<8>(0xf00, whole, rem);
  Test(whole == 0xf && rem == 0xf);
  clarinoid::DivRemBitwise<8>(0xf10, whole, rem);
  Test(whole == 0xf && rem == 0x1f);
}

