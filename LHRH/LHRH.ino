// Teensy LC
// 48mhz

#include "CCSwitch.h"
#include "CCLeds.h"

CCOnOffSwitch backButton(6, 100, 5);
CCLeds leds(10, 2, 10);

void setup() {
  Serial.begin(9600);
  SetupUpdateObjects();
}

int c = 0;

void loop() {
  UpdateUpdateObjects();

  c ++;
  leds.mStrip.setPixelColor(c % 5, c % 16, (c / 3) % 16, (c / 18) % 16);
  leds.mStrip.setPixelColor(9, backButton.State() ? 255 : 0, 0, 0);
  leds.mStrip.show();

  //Serial.println(sw.State());
  delay(20);
}
