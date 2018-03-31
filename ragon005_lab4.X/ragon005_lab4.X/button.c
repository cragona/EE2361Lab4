#include "xc.h"
#include <p24Fxxxx.h>
#include <stdio.h>
#include "buttonlib.h"

void initTwoPushButtons(void)
{
    CNPU2bits.CN21PUE = 1;
    CNPU2bits.CN22PUE = 1;
    //TIMER 2 SETUP > 1s
    T2CON = 0x0030; // 1:256 pre-scaler
    PR2 = 0xFFFF; //set max PR2 for overflows
    TMR2 = 0;
    IFS0bits.T2IF = 0;
    T2CON = 0x8030;
    _T2IE = 1;
    
    //SETUP INPUT CAPTURE 1
    IC1CON = 0;
    IC1BUF = 0;
    IC1CONbits.ICTMR = 1;
    
    IC1CONbits.ICM = 0b010; //capture falling edge
   
    
    //SETUP INPUT CAPTURE 2
    IC2CON = 0;
    IC2BUF = 0;
    IC2CONbits.ICTMR = 1;
    IC2CONbits.ICM = 0b010; //capture falling edge
}