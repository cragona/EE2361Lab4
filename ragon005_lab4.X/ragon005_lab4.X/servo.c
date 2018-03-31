#include "xc.h"
#include <p24Fxxxx.h>
#include <stdio.h>
#include "servolib.h"

void initServo(void)
{
    //TIMER 3 SETUP
    T3CON = 0x0010; //1:8
    PR3 = 39999;    //40,000 = 20ms
    TMR3 = 0;
    IFS0bits.T3IF = 0;
    T3CON = 0x8010;
    
    //OUTPUT COMPARE
    OC1CON = 0;
    OC1R = 3000; // 1:8 pre-scaler, thus 1.5ms - inital position
    OC1RS = 3000; // start at initial, can be updated -  changes servo position
    OC1CONbits.OCTSEL = 1;
    OC1CONbits.OCM = 0b110;
    
}

// I think i am going to want to make an algorithm that calculates the 
// position by typing -75 through 0 through 75
// what we know for the servo (black or blue)
// --75 is 1ms
// 0 is 1.5ms
// 75 is 2ms
// there for we can transfer this from 1 - 150 degress
// -75 being 1, 0 75 and 150 being 75
// ratio of 1:2000 - 150:4000
// OC1RS should be between 2000-4000
// based on win average speed we can set the zervo to any range
void setServo(unsigned long int val)
{
    OC1RS = val;
}

//void setServoTaDemo(int val)
//{
//    OC1RS = val;
//}