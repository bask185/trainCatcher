#include "src/io.h"
#include "src/date.h"
#include "src/version.h"
#include "src/macros.h"
#include "trainCatcher.h"

void setup()
{
    initIO() ;
    #ifdef DEBUG
    Serial.begin( 115200 ) ;
    Serial.println( version ) ;
    Serial.println( date ) ;
    #endif
    trainCatcherInit() ;
}

void loop()
{
	trainCatcher();
}

/*        U    
1                   8 VCC
2                   7 current sense
3 pot               6 PWM
4 GND               5
*/