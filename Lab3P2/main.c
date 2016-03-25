#include <xc.h>
#include <sys/attribs.h>
#include <string.h>
#include "lcd.h"
#include "timer.h"
#include "config.h"
#include "interrupt.h"

#define TRIS_pin1 TRISBbits.TRISB10
#define TRIS_pin2 TRISBbits.TRISB12
#define TRIS_Apin TRISBbits.TRISB0

#define pin1 LATBbits.LATB10
#define pin2 LATBbits.LATB12 
#define ANALOG_PIN LATBbits.LATB0       //pin for 

#define ON 1
#define OFF 0
#define OUTPUT 0
#define INPUT 1



void displayVoltage();       //used to write the output voltage to the LCD
char* buildString(float value);

typedef enum stateTypeEnum{
    Forward, Backwards, Test, Idle
} stateType;

volatile float val = 0.0;
volatile int switchFlag = 0;
volatile int pressCount = 0;     //used to only switch states with one press
void calculateODC();
char* buildString(float value);
char myString[5];
char newString[6] = {0};
int prevState = 2;      //backwards = 0 Forward = 1 

int main(void){
    SYSTEMConfigPerformance(10000000);
    enableInterrupts();
    initTimer1();
    initTimer4();
    initLCD();
    initADC();
    initPWM();
    initSwitch();

    int State = Forward;
    
    TRIS_pin1 = OUTPUT;
    TRIS_pin2 = OUTPUT;
    TRIS_Apin = INPUT;
    
    while(1){
        switch(State){
            case Forward:
                pin1 = ON;
                pin2 = OFF;
                if (switchFlag == 1 && pressCount == 2){
                    pressCount = 0;
                    switchFlag = 0;
                }
                if (switchFlag == 1 && pressCount == 1){
                    switchFlag = 0;
                    State = Idle;
                    prevState= 1;
                }
                
                displayVoltage();
                delayMs(10);
                calculateODC();
                
                break;
            
            case Backwards:
                
                pin1 = OFF;
                pin2 = ON;
                if (switchFlag == 1 && pressCount == 2){
                    pressCount = 0;
                    switchFlag = 0;
                }
                if (switchFlag == 1 && pressCount == 1){
                    State = Idle;
                    switchFlag = 0;
                    prevState= 0;
                    
                }
                displayVoltage();
                delayMs(10);
                calculateODC();
                
                break;
                
            case Idle:
                
              pin1 = ON;
              pin2 = ON;
              if (switchFlag == 1 && pressCount == 2){
                  pressCount = 0;
                  switchFlag = 0;
              }
              if (switchFlag == 1 && pressCount == 1){
                  switchFlag = 0;
                  if(prevState == 0){
                      State = Forward;
                  }
                  else if(prevState == 1){
                      State=Backwards;
                  }
           
                }
              displayVoltage();
              delayMs(10);               
              break;
            }
        }
    }
void __ISR(_CHANGE_NOTICE_VECTOR, IPL7SRS) _CNInterrupt()
{
    PORTD;
    IFS1bits.CNDIF = 0;       //set switch flag back down
    switchFlag = 1;
    pressCount++;
}


//used to write the output voltage to the LCD
void displayVoltage(){
    if(IFS0bits.AD1IF == 1){
            val = (float)((ADC1BUF0 / 222.3913043)); // dividing by 222.39 to put on max scale 4.6 CHANGE WHEN SWITCH TO BATTERY to 9 SCALE
            clearLCD();
            moveCursorLCD(1,1);
            printStringLCD(buildString(val));
            IFS0bits.AD1IF = 0;
         }
}

//builds a string of the voltage. Accurate up to .001
// assumes max of 9 volt read
char* buildString(float value){
    char printThis[6] = {0};
    int i = 0;
    int integerValue = (int)value; 
    int decimalValue = (int)((value*1000)-(integerValue*1000));
    int j, k;
   // get digits into printThis[]
    while(decimalValue != 0){
        int remainder = decimalValue % 10;
        printThis[i] = remainder + '0';
        decimalValue = decimalValue / 10;
        i++;
    }
     printThis[i] = '.';
     printThis[i + 1] = integerValue + '0';
     char temp = '1';
     int len = i;
     
     //reverse the string
     for(j = i + 1, k = 0; j >= 0; j--, k++){
         newString[j] = printThis[k];
     }
 
    return newString;
}

//controls the speed of the wheels
//when pot is in the middle both motors should be going full speed
//when pot is rotated all the way the respective motor should be going full speed

void calculateODC(){

    if(val >= 4.6){          //left wheel full speed
        OC2RS = 10000;
        OC4RS = 0;
    }
    else if(val < 0.1){     //right wheel full speed
        OC2RS = 0;
        OC4RS = 10000;
    }
    else if(val > 2.1 && val < 2.3){   //both wheels full speed
        OC2RS = 10000;
        OC4RS = 10000;
    }
     else if(val >= 2.3){         //left wheel spins faster than right
        OC2RS = 10000;
        OC4RS = (int)(((4.6 - val) * 4348)); // value of 4348 comes from 10000/2.3 (max value ratio)
    }
    else if(val <= 2.1){          //right wheel spins faster than left
        OC2RS = (int)(val*4348);
        OC4RS = 10000; 
    }
}