#include <touchablePin.h>

#define TOUCH_PIN 1
touchablePin pin(TOUCH_PIN);

void setup() { 
      Serial.begin(115200);
      delay(1000);
      Serial.print("Started...");
      pin.initUntouched();
      Serial.println("...Finished");
}
unsigned long lastLoopTime = 0,
              mainLoopCount = 0;

void loop() {
    unsigned long timeStart = micros();   
    bool y = pin.isTouched();
    unsigned long timeEnd = micros();    

    Serial.printf("Count: %d\tisTouched() = %d in %d microseconds\n",
                  mainLoopCount, y , timeEnd - timeStart); 
    mainLoopCount++;
}
