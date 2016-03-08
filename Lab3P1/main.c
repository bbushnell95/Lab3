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
    Forward, Backwards
} stateType;

volatile float val = 0;
volatile int switchFlag = 0;

void calculateODC();
char* buildString(float value);
char myString[5];
char newString[6] = {0};

int main(void){
    SYSTEMConfigPerformance(10000000);
    enableInterrupts();
    initTimer1();
    initTimer4();
    initLCD();
    initADC();
    initPWM();
    
    int pressCount = 0;     //used to only switch states with one press
    int State = Forward;
    
    TRIS_pin1 = OUTPUT;
    TRIS_pin2 = OUTPUT;
    TRIS_Apin = INPUT;
    
    //myString = buildString(7.456); test
    
    while(1){
        switch(State){
            case Forward:
                pin1 = ON;
                pin2 = OFF;
                
                if (switchFlag == 1 && pressCount == 2){
                    pressCount = 0;
                    switchFlag = 0;
                }
                if (switchFlag == 1 && pressCount ==1){
                    switchFlag = 0;
                    State = Backwards;
                }
                displayVoltage();
                calculateODC();
                
                break;
            
            case Backwards:
                
                pin1 = OFF;
                pin2 = ON;
                if (switchFlag == 1 && pressCount == 2){
                    pressCount = 0;
                    switchFlag = 0;
                }
                if (switchFlag == 1 && pressCount ==1){
                    State = Forward;
                    switchFlag = 0;
                    
                }
                displayVoltage();
                calculateODC();
                
                break;
            }
        }
    }
void __ISR(_CHANGE_NOTICE_VECTOR, IPL7SRS) _CNInterrupt()
{
    PORTD;
    IFS1bits.CNDIF = 0;       //set switch flag back down
    switchFlag = 1;
}


//used to write the output voltage to the LCD
void displayVoltage(){
    if(IFS0bits.AD1IF == 1){
            val = ADC1BUF0;
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
    if(val > 4.9){          //left wheel full speed
        OC2RS=10000;
        OC4RS=0;
    }
    else if(val < 0.1){     //right wheel full speed
        OC2RS=0;
        OC4RS=10000;
    }
    else if(2.4 < val < 2.6){   //both wheels full speed
        OC2RS = 10000;
        OC4RS = 10000;
    }
    else if(val > 2.6){         //left wheel spins faster than right
        OC2RS = 5000+(int)(val*1000);
        OC4RS = 5000-(int)(val*1000);
    }
    else if(val< 2.4){          //right wheel spins faster than left
        OC2RS = 5000-(int)(val*1000);
        OC4RS = 5000+(int)(val*1000); 
    }
}