#include <xc.h>
#include <sys/attribs.h>
#include <string.h>
#include "lcd.h"
#include "timer.h"
#include "config.h"
#include "interrupt.h"

#define TRIS_pin1 TRISBbits.TRISB10
#define TRIS_pin2 TRISBbits.TRISB12
#define pin1 LATBbits.LATB10
#define pin2 LATBbits.LATB12 
#define ANALOG_PIN LATBbits.LATB0
#define OUTPUT 0
#define INPUT 1



void displayVoltage();       //used to write the output voltage to the LCD
char* buildString(float value);

typedef enum stateTypeEnum{
    Forward, Backwards
} stateType;

volatile float val = 0;
volatile int switchFlag = 0;

char* buildString(float value);
char myString[5];

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
    
    //myString = buildString(7.456); test
    
    while(1){
        switch(State){
            case Forward:
                if (switchFlag == 1 && pressCount == 2){
                    pressCount = 0;
                }
                if (switchFlag == 1 && pressCount ==1){
                    State = Backwards;
                }
                displayVoltage();
                pin1 = 1;
                pin2 = 0;
                break;
            
            case Backwards:
                if (switchFlag == 1 && pressCount == 2){
                    pressCount = 0;
                }
                if (switchFlag == 1 && pressCount ==1){
                    State = Forward;
                }
                displayVoltage();
                pin1 = 0;
                pin2 = 1;
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
     char newString[6] = {0};
     //reverse the string
     for(j = i + 1, k = 0; j >= 0; j--, k++){
         newString[j] = printThis[k];
     }
 
    return newString;
}