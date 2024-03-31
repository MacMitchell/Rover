/*
 * File:   PCLMain.c
 * Author: chum_
 *
 * Created on February 28, 2024, 6:50 PM
 */
// CONFIG1
#pragma config FEXTOSC = ECH    // External Oscillator mode selection bits (EC above 8MHz; PFM set to high power)
#pragma config RSTOSC = HFINT32  // Power-up default value for COSC bits (HFINTOSC (1MHz))
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (FSCM timer disabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config WRT = OFF        // UserNVM self-write protection bits (Write protection off)
#pragma config SCANE = available// Scanner Enable bit (Scanner module is available for use)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (Program Memory code protection disabled)
#pragma config CPD = OFF        // DataNVM code protection bit (Data EEPROM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#define _XTAL_FREQ 32000000

#include "Color_Header.h"





struct controller{
    int rightX;
    int rightY;
    int leftY;
    int leftX;
    int switchA;
    int switchB;
    int switchC;
    int switchD;
    int potentionmeterA;
    int potentionmeterB;
    int temp;
} Controls;


struct command{
    int sendLimit;
    int sendIt;
    int sendId;
    unsigned char* toSend;
    
    int receiveIt;
    int receiveLimit;
    int receiveId;
    
    unsigned char done;
} Command;

const int CONTROL_INPUT =1;
int CONTROL_OUTPUT = 1;

int currentInput =0;
int inputStage = 0;

unsigned char GetUserDataCommand[6] = {0xFE, 0x19, 0x01, 0x5,0x00,0x00};
unsigned char EnableLaserScopeCommand[7] = {0xFE, 0x19, 0x01, 0x08, 0x01, 0x00, 0x01};

unsigned char TurnLeftCommand[10] = {0xFE, 0x19, 0x01, 0x06, 0x04, 0x00, 0x01, 0x32, 0x02, 0x32};
unsigned char TurnRightCommand[10] = {0xFE, 0x19, 0x01, 0x06, 0x04, 0x00, 0x02, 0x32, 0x01, 0x32};
unsigned char MoveForwardCommand[10] = {0xFE, 0x19, 0x01, 0x06, 0x04, 0x00, 0x01, 0x32, 0x01, 0x32};
unsigned char MoveBackwardCommand[10] = {0xFE, 0x19, 0x01, 0x06, 0x04, 0x00, 0x02, 0x32, 0x02, 0x32};
unsigned char Break[10] = {0xFE, 0x19, 0x01, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};

int commandIt = 0;
unsigned int SWITCH_MAX = 2000;// (int)0xE803*0.9;
unsigned int SWITCH_MIN = 1000; //(int)0xD007*1.1;
unsigned int SWITCH_MID = 1500;
struct controller controls;
struct command currentCommand;
void __interrupt() myFunction();
void CreateControlsCommand();
void CreateTurnRightCommmand(unsigned int pwm);
void CreateTurnLeftCommmand(unsigned int pwm);
void CreateMoveForwardCommmand(unsigned int pwm);
void CreateMoveBackwardCommmand(unsigned int pwm);
void CreateBreak();
void drive();


void SetUpPumpArm();
void SetUpPump();
void ActivatePump(int switchValue);
void MovePumpArm(int switchValue);


//COLOR SENSOR

unsigned char color=0;
ColorScheme colors;

int I2CStage = 0;
unsigned char newI2CMessage = 1;

int counter = 0;
void main(void) {
    
   TRISAbits.TRISA5 = 1;
    ANSELAbits.ANSA5 = 0;
    
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;
    
    
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 0;
    
    SetUpPumpArm();
    SetUpPump();
    //tell it we need 16 bits
    BAUD1CONbits.BRG16=1;
    //divide clock by 4
    TX1STAbits.BRGH = 0x1;
    
    
    //Sets RC5 to be transmitter
    RC5PPS = 0x10;
    TRISCbits.TRISC5 = 0;
    //Sets rc6 to be reciever
    RXPPSbits.RXPPS = 0x16;
    TRISCbits.TRISC6 = 1;
    ANSELCbits.ANSC6 = 0;

    
    SP1BRGLbits.SP1BRGL = 68;
    
    CreateControlsCommand();
    
    //setting up the transmission
    RC1STAbits.SPEN = 1;
    TX1STAbits.TXEN =1;
    TX1STAbits.SYNC=0;
           
    RC1STAbits.CREN = 1;
    
    
    SetUpColorSensor();
    
    
    
    
    //enable interrupts
    INTCONbits.PEIE = 1;
    PIE3bits.RCIE = 1;
    PIE3bits.TXIE = 1;
    PIE3bits.SSP1IE = 1;
    INTCONbits.GIE = 1;

    
    while(1){
        if(controls.switchA <= SWITCH_MIN){
            //LATAbits.LATA0 = 1;
            MovePumpArm(controls.switchC);
            ActivatePump(controls.switchD);
            
            if(controls.switchB <= SWITCH_MIN){
                ColorSensor(&newI2CMessage,&I2CStage,&colors,&color);
            }
        }
        else{
            //LATAbits.LATA0 = 0;
            MovePumpArm(SWITCH_MID); //do not move pump arm
            ActivatePump(SWITCH_MAX);//turn off pump
        }
        if(controls.switchB <= SWITCH_MIN){
            //LATAbits.LATA1 = 1;
        }
        else{
            //LATAbits.LATA1 = 0;
        }  
        if(controls.switchC == SWITCH_MID){
            //LATAbits.LATA2 = 1;
        }
        else{
            //LATAbits.LATA2 = 0;
        }  
        if(controls.switchD <= SWITCH_MIN){
            //LATAbits.LATA3 = 1;
        }
        else{
            //LATAbits.LATA3 = 0;
        }
        
        
        if(currentCommand.done){
            if(currentCommand.sendId == CONTROL_INPUT){
                drive();
            }
            else{
                CreateControlsCommand();
            }
        }
    

        
    }
    return;
}

void GetControllerInput(int input){
    if(currentCommand.receiveIt <0 ){
        return;
    }
    switch(currentCommand.receiveIt){
        case 6: controls.temp = input; break;
        case 7: controls.rightX = controls.temp + (input << 8); break;
        case 8: controls.temp = input; break;
        case 9: controls.rightY = controls.temp + (input << 8); break;
        case 10: controls.temp = input; break;
        case 11: controls.leftY = controls.temp + (input << 8); break;
        case 12: controls.temp = input; break;
        case 13: controls.leftX = (input << 8) + controls.temp; break;
        case 14: controls.temp = input; break;
        case 15: controls.switchA = controls.temp + (input << 8); break;
        case 16: controls.temp = input; break;
        case 17: controls.switchB = controls.temp + (input << 8); break;
        case 18: controls.temp = input; break;
        case 19: controls.switchC = controls.temp + (input << 8); break;
        case 20: controls.temp = input; break;
        case 21: controls.switchD = controls.temp + (input << 8); break;
        case 22: controls.temp = input; break;
        case 23: controls.potentionmeterA = controls.temp + (input << 8); break;
        case 24: controls.temp = input; break;
        case 25: controls.potentionmeterB = controls.temp + (input << 8); currentInput = 0;break;

    }
}

void drive(){
    unsigned int power = 75;
    
    if((controls.rightX >= 0x6A4) && (controls.rightY >= 0x546 && controls.rightY <= 0x672)){
        CreateTurnRightCommmand(power);
    }else if((controls.rightX <= 0x514) && (controls.rightY >= 0x546 && controls.rightY <= 0x672)){
        CreateTurnLeftCommmand(power);
    }else if((controls.rightY >= 0x6A4) && (controls.rightX >= 0x546 && controls.rightX <= 0x672)){
        CreateMoveForwardCommmand(power);
    }else if((controls.rightY <= 0x514) && (controls.rightX >= 0x546 && controls.rightX <= 0x672)){
        CreateMoveBackwardCommmand(power);
    }else if((controls.rightY >= 0x546 && controls.rightY <= 0x672) && (controls.rightX >= 0x546 && controls.rightX <= 0x672)){
        CreateBreak();
    }
    else{
        CreateControlsCommand();
    }
}

void CreateTurnRightCommmand(unsigned int pwm){
    currentCommand.sendId = 3;
    currentCommand.sendIt = 0;
    currentCommand.sendLimit = 10;
    currentCommand.receiveId = 0;
    currentCommand.receiveLimit = 0;
    currentCommand.receiveIt = 0;
    currentCommand.done = 0;
    
    TurnRightCommand[7] = pwm;
    TurnRightCommand[9] = pwm;
    currentCommand.toSend = TurnRightCommand;
    PIE3bits.TXIE = 1;
}

void CreateTurnLeftCommmand(unsigned int pwm){
    currentCommand.sendId = 4;
    currentCommand.sendIt = 0;
    currentCommand.sendLimit = 10;
    currentCommand.receiveId = 0;
    currentCommand.receiveLimit = 0;
    currentCommand.receiveIt = 0;
    currentCommand.done = 0;
    
    TurnLeftCommand[7] = pwm;
    TurnLeftCommand[9] = pwm;
    currentCommand.toSend = TurnLeftCommand;
    PIE3bits.TXIE = 1;
}

void CreateMoveForwardCommmand(unsigned int pwm){
    currentCommand.sendId = 5;
    currentCommand.sendIt = 0;
    currentCommand.sendLimit = 10;
    currentCommand.receiveId = 0;
    currentCommand.receiveLimit = 0;
    currentCommand.receiveIt = 0;
    currentCommand.done = 0;
    
    MoveForwardCommand[7] = pwm;
    MoveForwardCommand[9] = pwm;
    currentCommand.toSend = MoveForwardCommand;
    PIE3bits.TXIE = 1;
}

void CreateMoveBackwardCommmand(unsigned int pwm){
    currentCommand.sendId = 6;
    currentCommand.sendIt = 0;
    currentCommand.sendLimit = 10;
    currentCommand.receiveId = 0;
    currentCommand.receiveLimit = 0;
    currentCommand.receiveIt = 0;
    currentCommand.done = 0;
    
    MoveBackwardCommand[7] = pwm;
    MoveBackwardCommand[9] = pwm;
    currentCommand.toSend = MoveBackwardCommand;
    PIE3bits.TXIE = 1;
}

void CreateBreak(){
    currentCommand.sendId = 7;
    currentCommand.sendIt = 0;
    currentCommand.sendLimit = 10;
    currentCommand.receiveId = 0;
    currentCommand.receiveLimit = 0;
    currentCommand.receiveIt = 0;
    currentCommand.done = 0;
    
    currentCommand.toSend = Break;
    PIE3bits.TXIE = 1;
}

void CreateControlsCommand(){
    currentCommand.receiveId = CONTROL_INPUT;
    currentCommand.receiveLimit = 26;
    currentCommand.receiveIt = 0;
    currentCommand.sendId = CONTROL_OUTPUT;
    currentCommand.toSend = (unsigned char*) GetUserDataCommand;
    currentCommand.sendIt = 0;
    currentCommand.sendLimit = 6;
    currentCommand.done = 0;
    PIE3bits.TXIE = 1;
}



void SetUpPumpArm(){
    
    //for the actual pump arm device: red wire connects to left side of MOT.B and black wire is the right side of MOT.B
    TRISBbits.TRISB0 = 0; //BIN2 on motor driver
    TRISBbits.TRISB1 = 0; //BIN1 on motor driver
    ANSELBbits.ANSB0 = 0;
    ANSELBbits.ANSB1 = 0;
    
    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;
    
}

void SetUpPump(){
    //For the actual pump: Red wire goes the left side of MOT.A and black wire goes to right side of MOT.A
    
    TRISBbits.TRISB2 = 0; //AIN1 on motor driver
    TRISBbits.TRISB3 = 0; //AIN2 on motor driver
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    
    LATBbits.LATB2 = 0;
    LATBbits.LATB3 = 0;
}
    
void MovePumpArm(int switchValue){
    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;
    if(switchValue == SWITCH_MIN){
            LATBbits.LATB0 = 1;
            LATBbits.LATB1 = 0;
    }
    else if(switchValue == SWITCH_MAX){    
            LATBbits.LATB0 = 0;
            LATBbits.LATB1 = 1;
    }
}

void ActivatePump(int switchValue){
    if(switchValue <= SWITCH_MIN){
        LATBbits.LATB2 = 1;
        LATBbits.LATB3 = 0;
    }
    else{
        LATBbits.LATB2 = 0;
        LATBbits.LATB3 = 0;
    }
}
void __interrupt() myFunction(){
    if(PIR3bits.SSP1IF == 1){
        //I2CStage++;
        newI2CMessage = 1;
        PIR3bits.SSP1IF = 0;
    }
    if(PIR3bits.RCIF == 1){
        int input = RCREG;
        if(currentCommand.receiveId == CONTROL_INPUT){
            GetControllerInput(input);
            currentCommand.receiveIt = currentCommand.receiveIt + 1;
        }
        if(currentCommand.receiveIt  >= currentCommand.receiveLimit){
            currentCommand.done = 1;
        }
        inputStage++;
        PIR3bits.RCIF =0;

    }
    if(PIR3bits.TXIF == 1){
        if(currentCommand.sendIt >= currentCommand.sendLimit){
            PIE3bits.TXIE = 0;
            if(currentCommand.receiveLimit <= 0){
                currentCommand.done = 1;
            }
        }
        else{
            TX1REGbits.TX1REG = currentCommand.toSend[currentCommand.sendIt];
            currentCommand.sendIt = currentCommand.sendIt + 1;
        }
        PIR3bits.TXIF = 0;
    }
    
}

