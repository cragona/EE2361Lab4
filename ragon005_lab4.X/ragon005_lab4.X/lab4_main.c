/*
 * File:   lab04_main.c
 * Author: Charlie
 *
 * Created on February 25, 2018, 5:47 PM
 */
#include "xc.h"
#include <p24Fxxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include "servolib.h"
#include "buttonlib.h"

// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1 // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF // JTAG Port Enable (JTAG port is disabled)


// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME // Clock Switching and Monitor (Clock switching is enabled,


// Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

void setup(void)
{
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0x9fff;
    TRISB = 0x0300;
    LATBbits.LATB5 = 0;
    
    //SETUP PPS
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS 
    RPINR7bits.IC1R = 8;  // left button
    RPINR7bits.IC2R = 9;  // right button
    RPOR3bits.RP6R = 18; //output to servo
    //RPOR2bits.RP5R = 18; //output for led
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}

volatile unsigned long int leftAvg[4] = {0,0,0,0};
volatile unsigned long int rightAvg[4] = {0,0,0,0};


volatile unsigned long int leftPersonScore;
volatile unsigned long int rightPersonScore;
static int leftOverflow;
static int rightOverflow;
static int leftStart;
static int rightStart;

void __attribute__((__interrupt__,__auto_psv__)) _IC1Interrupt(void)
{
    leftStart = 1;
    static int prevEdge = 0;
    int currentEdge;
    static int count = 0;
    _IC1IF = 0;
    currentEdge = IC1BUF;
    
    if(currentEdge >= 125)
    {
        //printf("\n\nIC1BUF: %lu \n", IC1BUF );
        leftAvg[count] = abs(abs(currentEdge - prevEdge) - (leftOverflow*PR2));
        //printf("count: %d \n", count );
        //printf("LeftAvg[count]: %lu \n", leftAvg[count] );
        prevEdge = currentEdge;
        count++;
        count &= 3;
        

        int i;
        leftPersonScore = 0; //reset score
        for(i = 0; i < 4; i++)
        {
            leftPersonScore += leftAvg[i]; //keep adding last clicks
        }
        leftPersonScore /= 4; //get avg of last 4 clicks

        
       
           
        leftOverflow = 0;
    }
    
}

void __attribute__((__interrupt__,__auto_psv__)) _IC2Interrupt(void)
{
    
    static int prevEdge;
    int currentEdge;
    static int count = 0;
    rightStart = 1;
    _IC2IF = 0;
    
    currentEdge = IC2BUF;
    if(currentEdge >= 125)
    {  
        //printf("\n\nIC2BUF: %lu \n", IC2BUF );
        
        rightAvg[count] = abs(abs(currentEdge - prevEdge) - (rightOverflow*PR2));
        
       // printf("count: %d \n", count );
        //printf("RightAvg[count]: %lu \n", rightAvg[count] );
        
        prevEdge = currentEdge;
        count++;
        count &= 3;

        int i;
        rightPersonScore = 0;
        for(i = 0; i < 4; i++)
        {
            rightPersonScore += rightAvg[i]; //keep adding last clicks
        }
        rightPersonScore /= 4; //get avg of last 4 clicks

        
        rightOverflow = 0;
    }
}
static int leftWins = 0;
static int rightWins = 0;
//every 100ms seconds decide the winner
void __attribute__((__interrupt__,__auto_psv__)) _T1Interrupt(void)
{
    _T1IF = 0;
    
    static int count = 0;
    count++;
  
    if(count == 1000) //start update servo
    {
        //printf("\nleft score: %lu", leftPersonScore);
        //printf("\nright score: %lu", rightPersonScore);
        double denom = (double)(leftPersonScore + rightPersonScore);
        //printf("\ndenom: %.2f", denom);
        
        if(leftPersonScore < rightPersonScore)
        {
            double calculation = (((double)(rightPersonScore - leftPersonScore))/denom)*1000;
            //printf("\nleft win calc: %d", (int)calculation);
            setServo(3000 - (int)calculation);
            
            leftWins++;
            rightWins = 0;

        }
        else if(leftPersonScore > rightPersonScore)
        {
            double calculation = (((double)(leftPersonScore - rightPersonScore))/denom)*1000;
            //printf("\nright win calc: %d", (int)calculation);
            setServo(3000 + (int)calculation);
            
            rightWins++;
            leftWins = 0;
        }
        else
        {
            setServo(3000);
            leftWins = 0;
            rightWins = 0;
        }
        
        
        leftPersonScore = 0;
        rightPersonScore = 0;
        count = 0;
    }
}

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)
{
    _T2IF = 0;
    leftOverflow++;
    rightOverflow++;
}

void endGame(void)
{
    _IC1IE = 0;    
    _IC2IE = 0;
    
    setServo(3000);
    
    while(1)
    {
        LATB = 0b100000;
    }
}



int main(void) 
{
    
    //TIMER 1 SETUP for updating servo every .5 secs
    T1CON = 0;
    PR1 = 15999;
    TMR1 = 0;
    IFS0bits.T1IF = 0;
    T1CONbits.TON = 1; // now turn on the timer
    
    leftStart = 0;
    rightStart = 0;

    setup();
    initServo();
    initTwoPushButtons();
    setServo(3000); //set initial position
    _IC1IE = 1;    
    _IC2IE = 1;
    
    
    while(1)
    {
        //int pos = 1;


        if(leftStart == 1 && rightStart == 1) //start game
        {
            _T1IE = 1; //start updating servo
            
            if(leftWins == 5 || rightWins == 5)
            {
                endGame();
            }
        }

//        int count = 5;
//        
//        while(count > 0)
//        {
//            setServoTaDemo(10000); //25%
//            delay(1000);
//            count--;
//        }
//        
//        count = 5;
//        while(count > 0)
//        {
//            setServoTaDemo(30000); //25%
//            delay(1000);
//            count--;
//        }
        
            
        
//        printf("\n\nLeft Person Score: %d \n", leftPersonScore );
//        printf("Left Avg 0: %d \n", leftAvg[0] );
//        printf("Left Avg 1: %d \n", leftAvg[1] );
//        printf("Left Avg 2: %d \n", leftAvg[2] );
//        printf("Left Avg 3: %d \n", leftAvg[3] );
//        
//        printf("\n\nrRight Person Score: %d \n", rightPersonScore );
//        printf("Right Avg 0: %d \n", rightAvg[0] );
//        printf("Right Avg 0: %d \n", rightAvg[1] );
//        printf("Right Avg 0: %d \n", rightAvg[2] );
//        printf("Right Avg 0: %d \n", rightAvg[3] );

    }
    return 0;
}
