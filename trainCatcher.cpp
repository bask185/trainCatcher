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

bool internalPause ;

Weistra pwmRegelaar( pwmPin, 50, 100 ) ;

// FUNCTIONS
extern void trainCatcherInit(void)
{
    sm.nextState( waitSignal, 0 ) ;
    pwmRegelaar.begin() ;
}


Debounce section1( sectionPin1 ) ;
Debounce section2( sectionPin2 ) ;
Debounce holdLine( holdTrainPin ) ;

Debounce* breakSection ( 0 ) ;
Debounce* stopSection  ( 0 ) ;

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
        debug(F("awaiting train"));
        digitalWrite( pwmPin, HIGH ) ; // before waiting, enable trackpower first
    }
    if( sm.onState() )
    {    
        if( section1.getState() == FALLING )
        {
            breakSection = &section1 ;
            stopSection  = &section2 ;
            sm.exit() ;
        }
        if( section2.getState() == FALLING )
        {
            breakSection = &section2 ;
            stopSection  = &section1 ;
            sm.exit() ;
        } 
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
        debug(F("slowing down"));
    }
    if( sm.onState() )
    {
        pwmRegelaar.update() ;                                          // handles PWM on track

        if( stopSection -> getState() == FALLING )                    // If downgoing flank, decelerate train faster
        {
            speedInterval /= 3 ;
            debug(F("slowing down faster"));
        }

        REPEAT_MS( speedInterval )
        {
            pwmRegelaar.setSpeed( -- speed ) ;

        } END_REPEAT 

        if( speed == 0)
        {                                          // if speed = 0, exit -> 
            debug(F("train stopped"));
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
        debug(F("setting signal")) ;
        internalPause = false ;
        pinMode(received, OUTPUT) ;
        digitalWrite(received, LOW ) ;
        sm.setTimeout( 300 ) ;
    }
    if( sm.onState() )
    {
        if( sm.timeout() )
        {
            sm.exit() ;
        }
        if( holdLine.getState() == FALLING )
        {
            debug(F("pausing for 30s")) ;
            internalPause = true ;
            sm.exit() ;
        }
    }
    if( sm.exitState() )
    {
        debug(F("signal cleared")) ;
        pinMode(received, INPUT_PULLUP) ;
    }
    return sm.endState() ;
}

StateFunction( waitSignal )
{
    if( sm.entryState() )
    {
        debug(F("waiting on signal to depart"));
        // sm.setTimeout( 500 ) ;

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
        uint8_t holdState = holdLine.getState() ;
        // if( holdState == HIGH/* && sm.timeout() */)                              // if timeout happens, the train is not detected -> depart the train
        // {
        //     sm.exit() ;
        //     direction = reversed ;
        // debug(F("polarity switch signal"));
        // }
        if( holdState == RISING  )       // if signal is received -> send the train
        {
            sm.exit() ;
            // direction = forward ;
            debug(F("manual signal received"));
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
        speedInterval = 20 + analogRead( potPin ) / 13 ;                // read accelerate time from potmeter 30% faster than slowing down
        debug(F("train is allowed to depart"));
        debug(F("accelerating"));
    }

    if( sm.onState() )
    {
        REPEAT_MS( speedInterval )
        {
            pwmRegelaar.setSpeed( ++ speed ) ;
            if( speed == 100 ) sm.exit() ;                                      // if speed is maximum -> next state
        } END_REPEAT
        
        // if(  direction == reversed 
        // &&  (breakSection -> getState() == LOW || stopSection -> getState() == LOW ) 
        // {
        //     sm.exit() ;                                                         // or if sensor is made while train should be driving backwards, stop at once
        // }
    }                                                                           // it may occur that a train just loses connection with tracks for a brief moment
                                                                                // this could lead to the the train departing too soon, if this happens we go back to waitSignal
    if( sm.exitState() )
    {
        debug(F("speed at maximum"));
    }
    return sm.endState() ;
}


// STATE MACHINE
extern uint8_t trainCatcher()
{
    REPEAT_MS( 50 ) 
    {
<<<<<<< HEAD
        transceiver.debounceInputs() ;
        holdTrain.debounceInputs() ;
        breakSection.debounceInputs() ;
        stopSection.debounceInputs() ;

=======
        //transceiver.debounceInputs() ;
        holdLine.debounce() ;
        section1.debounce() ;
        section2.debounce() ;
        
>>>>>>> 07aaa4fa1c39cb7c6ad4ef6a802a1dee9bf398ba
    } END_REPEAT

    pwmRegelaar.update() ;      

    STATE_MACHINE_BEGIN

    State(awaitTrain) {
        sm.nextState( slowTrain, 0 ) ; }

    State(slowTrain) {
        sm.nextState( sendSignal, 100 ) ; }

    State( sendSignal ) {
        sm.nextState( waitSignal, 0 ) ; }

    State(waitSignal) {
        if( internalPause == true ) sm.nextState( accelerateTrain, 30000 ) ;
        else                        sm.nextState( accelerateTrain, 0 ) ; }

    State(accelerateTrain) {
        if( speed == 100 )  sm.nextState( awaitTrain, 8000 ) ;
        else                sm.nextState( waitSignal, 0 ) ; }

    STATE_MACHINE_END
}
