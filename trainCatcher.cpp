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
//Debounce stopSection(   stopSectionPin ) ;

// static void foo()
// {
//     ;
// }

// VARIABLES
const int forward  = 1 ;
const int reversed = 2 ;

uint8_t     speed ;
uint8_t     direction ;
uint16_t    speedInterval ;
uint32_t    timeStamp ;
uint8_t     hold;


void debug(String txt)
{
    #ifdef DEBUG
    Serial.println(txt);
    Serial.println();
    
    #endif
}

// STATE FUNCTIONS
StateFunction( awaitTrain )
{
    if( sm.entryState() )
    {
        // debug(F("awaiting train"));
        digitalWrite( pwmPin, HIGH ) ; // before waiting, enable trackpower first
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
        // debug(F("slowing down"));
    }
    if( sm.onState() )
    {
        pwmRegelaar.update() ;                                    // handles PWM on track

        // if( stopSection.readInput() == FALLING )                    // If downgoing flank, decelerate train faster
        // {
        //     speedInterval /= 3 ;
        //     // debug(F("slowing down faster"));
        // }

        REPEAT_MS( speedInterval )
        {
            pwmRegelaar.setSpeed( -- speed ) ;
            #ifdef DEBUG
            printNumberln("speed ", speed ) ;
            #endif
        } END_REPEAT 

        if( speed == 0)
        {                                          // if speed = 0, exit -> 
            // debug(F("train stopped"));
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
        // debug(F("waiting on signal to depart"));
        sm.setTimeout( 500 ) ;

        speed = 0 ;
        pwmRegelaar.setSpeed( speed ) ;
    }
    if( sm.onState() )
    {
        // REPEAT_MS( 1 )
        // {
        //     static uint8_t counter = 0 ;

        //     if( counter == 0 ) { digitalWrite( pwmPin, HIGH ); delayMicroseconds(500); }
        //     else                 digitalWrite( pwmPin,  LOW );

        //     if( ++counter > 250 ) counter = 0 ;
        // } END_REPEAT


        // if( digitalRead( pwmPin ) == HIGH && digitalRead( breakSectionPin ) == LOW ) 
        // {
        //    sm.setTimeout( 500 ) ;                          // reload timer
        //    digitalWrite( pwmPin,  LOW );
        // }
        // uint8_t holdState = holdTrain.readInput() ;
        // if( holdState == HIGH && sm.timeout() )                              // if timeout happens, the train is not detected -> depart the train
        // {
        //     sm.exit() ;
        //     direction = reversed ;
        //     // debug(F("polarity switch signal"));
        // }
        if( holdState == RISING  )       // if signal is received -> send the train
        {
            sm.exit() ;
            direction = forward ;
            // debug(F("manual signal received"));
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
        // debug(F("train is allowed to depart"));
        // debug(F("accelerating"));
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
        // debug(F("speed at maximum"));
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

    REPEAT_MS( 1 ) 
    {
        breakSection.debounceInputs() ;
        //stopSection.debounceInputs() ;
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
