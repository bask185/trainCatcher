// HEADER FILES
#include <Arduino.h>
#include "trainCatcher.h"
#include "src/macros.h"
#include "src/io.h"
#include "src/debounceClass.h"
#include "src/stateMachineClass.h"
#include "src/weistra.h"

static StateMachine sm ;

#define beginState awaitTrain
#ifndef beginState
#error beginState not defined
#endif


Weistra pwmRegelaar( pwmPin, 50, 100 ) ;

// FUNCTIONS
extern void trainCatcherInit(void)
{
    sm.nextState( waitSignal, 0 ) ;
    pwmRegelaar.begin() ;
}


Debounce transceiver(   received ) ;
Debounce holdTrain (    holdTrainPin ) ;
Debounce breakSection(  breakSectionPin ) ;

// VARIABLES
const int forward  = 1 ;
const int reversed = 2 ;

uint8_t     speed ;
uint8_t     direction ;
uint16_t    speedInterval ;
uint32_t    timeStamp ;
uint8_t     hold;

// STATE FUNCTIONS
StateFunction( awaitTrain )
{
    if( sm.entryState() )
    {
        digitalWrite( pwmPin, HIGH ) ;                                          // before waiting, enable trackpower first
    }
    if( sm.onState() )
    {  
        if( breakSection.readInput() == LOW ) sm.exit() ;
    }
    if( sm.exitState() )
    {
        
    }
    return sm.endState() ;
}

StateFunction( slowTrain )
{
    if( sm.entryState() )
    {
        speed = 100 ;
        pwmRegelaar.setSpeed( speed ) ;
        speedInterval = 20 + analogRead( potPin ) / 10 ;             // read break time from potmeter
    }
    if( sm.onState() )
    {
        pwmRegelaar.update() ;                                    // handles PWM on track

        REPEAT_MS( speedInterval )
        {
            pwmRegelaar.setSpeed( -- speed ) ;
            #ifdef DEBUG
            printNumberln("speed ", speed ) ;
            #endif
        } END_REPEAT 

        if( speed == 0)
        { 
            sm.exit() ;
        }
    }
    if( sm.exitState() )
    {
        
    }
    return sm.endState() ;
}

StateFunction( sendSignal )
{
    if( sm.entryState() )
    {
        digitalWrite(received, HIGH ) ;
        sm.setTimeout( 100 ) ;
    }
    if( sm.onState() )
    {
        if( sm.timeout() )
        {
            sm.exit() ;
        }
    }
    if( sm.exitState() )
    {
        digitalWrite(received, LOW ) ;
    }
    return sm.endState() ;
}

StateFunction( waitSignal )
{
    if( sm.entryState() )
    {
        sm.setTimeout( 500 ) ;

        speed = 0 ;
        pwmRegelaar.setSpeed( speed ) ;
    }
    if( sm.onState() )
    {
        if( holdState == RISING  )       // if signal is released -> send the train
        {
            sm.exit() ;
        }
    }
    if( sm.exitState() )
    {
        
    }
    return sm.endState() ;
}

StateFunction( accelerateTrain )
{
    if( sm.entryState() )
    {
        speed = 0 ;
        pwmRegelaar.setSpeed( speed ) ;
        speedInterval = 20 + analogRead( potPin ) / 10 ;                // read accelerate time from potmeter
    }

    if( sm.onState() )
    {
        REPEAT_MS( speedInterval )
        {
            pwmRegelaar.setSpeed( ++ speed ) ;
            if( speed == 100 ) sm.exit() ;                                  // if speed is maximum -> next state
        } END_REPEAT
    }

    if( sm.exitState() )
    {
        
    }
    return sm.endState() ;
}


// STATE MACHINE
extern uint8_t trainCatcher()
{
    REPEAT_MS( 50 ) 
    {
        transceiver.debounceInputs() ;
        holdTrain.debounceInputs() ;
    } END_REPEAT

    REPEAT_MS( 20 ) 
    {
        breakSection.debounceInputs() ;
    } END_REPEAT

    pwmRegelaar.update() ;      

    STATE_MACHINE_BEGIN

    State(awaitTrain) {
        sm.nextState( slowTrain, 0 ) ; }

    State(slowTrain) {
        sm.nextState( waitSignal, 100 ) ; }

    State(waitSignal) {
        sm.nextState( sendSignal, 0 ) ; }

    State( sendSignal ) {
        sm.nextState( accelerateTrain, 0 ) ; }

    State(accelerateTrain) {
        sm.nextState( awaitTrain, 8000 ) ; }

    STATE_MACHINE_END
}
