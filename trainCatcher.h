#include <Arduino.h>

#define DEBUG

enum trainCatcherStates
{
    trainCatcherIDLE,
    awaitTrain,
    slowTrain,
    waitSignal,
    sendSignal,
    accelerateTrain
} ;

extern uint8_t trainCatcher(void) ; 
extern void trainCatcherInit(void) ; 
