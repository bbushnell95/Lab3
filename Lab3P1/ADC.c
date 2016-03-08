#include "ADC.h"
#include <xc.h>

void initADC(){
    
    AD1CON1bits.FORM=0;
    AD1CON1bits.SSRC=7;
    AD1CON1bits.ASAM=1;
    
    AD1CON2bits.VCFG=0;
    AD1CON2bits.CSCNA=0;
    AD1CON2bits.SMPI=15;
    AD1CON2bits.ALTS=0;
    
    AD1CON3bits.ADRC=0;
    AD1CON3bits.SAMC=3;
    AD1CON3bits.ADCS=8;
    AD1CHSbits.CH0NA=0;
    AD1CHSbits.CH0SA=0;
    
    AD1CON1bits.ADON=1;
}
