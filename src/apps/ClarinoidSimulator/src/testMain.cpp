
extern int gTestPoint;

#include <iostream>
#include <windows.h>

#include <lib_example/example.hpp>

void setup() {}

void loop()
{
  gTestPoint++;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  ::OutputDebugStringW(L"Starting testMain.cpp\n");
  std::cout << "Starting testMain.cpp" << std::endl;
  return 0;
}