#include <Arduino.h>
#include "io.h"
extern void initIO(void) {
	pinMode(received, INPUT_PULLUP);
	pinMode(holdTrainPin, INPUT_PULLUP);
	pinMode(breakSectionPin, INPUT_PULLUP);
	pinMode(stopSectionPin, INPUT_PULLUP);
	pinMode(pwmPin, OUTPUT);
	pinMode(potPin, INPUT);
	pinMode(led, OUTPUT);
}