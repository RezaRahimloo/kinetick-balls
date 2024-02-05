#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);
  // digitalWrite(8, 1);
  // digitalRead(8);
  // GPIO_NUM_10;
  // io_conf.pin_bit_mask =  ((1ULL<< REGISTER_BIT7_ON_PIN) | (1ULL<< REGISTER_BIT6_ON_PIN) | (1ULL<< REGISTER_BIT5_ON_PIN)); // of course, do like this all the pins
  // GPIO_ENABLE_DATA
  // GPIO_OUT_DATA
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}