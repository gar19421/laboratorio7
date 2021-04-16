
/* 
 * File:   lab07_main.c
 * Author: brand
 *
 * Created on 12 de abril de 2021, 03:28 PM
 */

/*
 *Entrega Prelab: Botones puertos RB0 y RB1, leds contador botones puerto C
 * leds contador timer0 puerto D
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT//Oscillator Selection bits(INTOSC oscillator: 
                                  //CLKOUT function on RA6/OSC2/CLKOUT pin, 
                                  //I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT disabled and 
                          //can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR  
                                //pin function is digital input, MCLR internally 
                                //tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code 
                                //protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code 
                                //protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit 
                                //Internal/External Switchover mode is disabled
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit 
                                //(Fail-Safe Clock Monitor is disabled)
#pragma config LVP = ON         //Low Voltage Programming Enable bit(RB3/PGM pin 
                                //has PGM function, low voltage programming 
                                //enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out 
                                //Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits 
                                //(Write protection off)


// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Declaraciones y variables
 */
//tabla para el display 7 segmentos
char tabla[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67};

//variables temporales en multiplexado
char var_temp;
char decenas_temp = 0;

//variables de conversion Hex to Dec
char unidades = 0;
char decenas = 0;
char centenas = 0;

//variables de valores decimales convertidos con la tabla
char unidades_display = 0;
char decenas_display = 0;
char centenas_display = 0;



// Prototipos
void setup();
void config_reloj();
void config_io();
void config_int_enable();
void config_iocb();
void config_timer0();
void TMR0_interrupt();
void IOCB_interrupt();
void contador();

/*
 *  main
 */

void main(void) {
    
    setup(); // llamar función de configuraciones
    
    while(1){  //loop principal
        contador();
    }
    
    return;
}

void setup(){
    
    config_reloj();// configuraciones principales del programa
    config_io();
    config_int_enable();
    config_iocb();
    config_timer0();
    
    return;
};


// funcion contador
void contador(void){
    var_temp = PORTC;
        
    centenas = var_temp / 100; // se divide dentro de la base 
    decenas_temp = var_temp % 100; // y luego se obtiene el modulo para ver 
    decenas = decenas_temp / 10; // el residuo y guardar el valor en la variable
    unidades = var_temp % 10;
    
    unidades_display = tabla[unidades];//se obtiene valores para los displays
    decenas_display = tabla[decenas];
    centenas_display = tabla[centenas];
}




// interrupciones

void __interrupt() isr(void){

    if (INTCONbits.T0IF){//se verifica que bandera de interrupcion se levanto
        TMR0_interrupt();
    }
    if(INTCONbits.RBIF){
        IOCB_interrupt();
    }

}

void TMR0_interrupt(){
    
    PORTE = 0x00; // se limpia el puerto de multiplexado
    if(PORTD == 0){
        PORTA = unidades_display;
        PORTEbits.RE0 = 1;
        PORTD++; // se utiliza port D como la bandera de displays a multiplexar
    }else if(PORTD == 1){
        PORTA = decenas_display;
        PORTEbits.RE1 = 1;
        PORTD++;        
    }else {
        PORTA = centenas_display;
        PORTEbits.RE2 = 1;
        PORTD = PORTD + 2;
    }
    
    TMR0 = 235; // valor de n para t=5ms
    INTCONbits.T0IF = 0;
    
    return;
}


void IOCB_interrupt(){ // se verifica el push presionado e incrementa o decrem..

    if (PORTBbits.RB0 == 0){
        PORTC++;
    }
    if(PORTBbits.RB1 == 0) {
        PORTC--; 
    }
    
    INTCONbits.RBIF = 0;
    
    return;
} 

// configuraciones

void config_reloj(){

    OSCCONbits.IRCF2 =1 ; // IRCF = 110 (4Mz) 
    OSCCONbits.IRCF1 =1 ;
    OSCCONbits.IRCF0 =0 ;
    OSCCONbits.SCS = 1; // Habilitar reloj interno
    
    return;
}

void config_io(){
   
    ANSELH = 0x00;
    ANSEL = 0x00;
    
    TRISB = 0x03; // habilitar pines RB0 y RB1 como inputs
    
    TRISA = 0x00;
    TRISC = 0x00; // puertos c y d como salidas
    TRISD = 0xF8;
    TRISE = 0x00;

  
    OPTION_REGbits.nRBPU =  0 ; // se habilita el pull up interno en PORTB
    WPUB = 0x03;  // se habilita los pull ups para los pines RB0 y RB1
    
    PORTA = 0x00;
    PORTB = 0x03; // se limpian las salidas de los puertos y valores iniciales
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
    
   
    
    return;
}

void config_int_enable(){
    
    INTCONbits.GIE = 1; // Se habilitan las interrupciones globales
    
    INTCONbits.RBIE = 1; // habilitar banderas de interrupción puertos B
    INTCONbits.RBIF = 0; 	
    
    INTCONbits.T0IE = 1; // Se habilitan la interrupción del TMR0
    INTCONbits.T0IF = 0; // Se limpia la bandera
    
    return;
}
    

void config_iocb(){
    
    IOCB = 0x03; // setear interrupciones en los pines RB0 y RB1 
    
    INTCONbits.RBIF = 0;  
    
    return;
} 
    
void config_timer0(){

    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS2 = 1; // PS 111 = 256
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    
    TMR0 = 235; // valor de n para t=5ms
    INTCONbits.T0IF = 0;
            
    return;
}

