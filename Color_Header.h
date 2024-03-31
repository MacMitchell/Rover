/* 
 * File:   Color_Header.h
 * Author: chum_
 *
 * Created on March 6, 2024, 2:39 PM
 */

#ifndef COLOR_HEADER_H
#define	COLOR_HEADER_H
#define COLOR_ENABLE 0b11;

#define COLOR_ADDRESS_WRITE 0x39 << 1;
#define COLOR_ADDRESS_READ (0x39 << 1) | 1;
#define COLOR_ENABLE_REGISTER_ADDRESS 0x80;
#define COLOR_ID_REGISTER_ADDRESS 0x92;
#define COLOR_STATUS_ADDRESS 0x93;

#define COLOR_CLEAR_LOW 0x94;
#define COLOR_CLEAR_HIGH 0x95;
#define COLOR_RED_LOW 0x96;
#define COLOR_RED_HIGH 0x97;

#define COLOR_GREEN_LOW 0x98;
#define COLOR_GREEN_HIGH 0x99;

#define COLOR_BLUE_LOW 0x9A;
#define COLOR_BLUE_HIGH 0x9B;


unsigned char RED = 1;
unsigned char BLUE = 2;
unsigned char GREEN =  3;

int frequency = 2700;
int cycle = 32000000 / 2700;

//instead of 2700 it might be 1000
#define NOTE_C4 2700 /262
#define NOTE_F4 2700/ 349
#define NOTE_A4 2700/ 440



void StartIC2Transmission();
void EndIC2Transmission();
void StartWriteRequest();
void StartReadRequest();
void WriteChar(char input);
int ReadChar();

int ReadRed();
int ReadBlue();
int ReadGreen();
int IsColorDataReady();
void RepeatedStart();

typedef struct colorScheme{
    int clear;
    int green;
    int blue;
    int red;
    unsigned char temp;
    unsigned char ready;
    unsigned char colorStage;
    unsigned char allDone;
} ColorScheme;

void SetUpI2CPins(){
    SSP1CLKPPSbits.SSP1CLKPPS = 0x13; //RC3
    SSP1DATPPSbits.SSP1DATPPS = 0x14; //RC4
    
    RC3PPS = 0x14;
    RC4PPS = 0x15;
    
    
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;
    
    ANSELCbits.ANSC3 = 0;
    ANSELCbits.ANSC4 = 0;
    
    WPUCbits.WPUC3 = 1;
    WPUCbits.WPUC4 = 1;
}
void SetUpIC2Clock(){
    //Color sensor supports up to 400kHz, our clock is set to 320000
    //we need to add set up the SppxAdd such that it will be less than 400kHz, 
    //the value 19 works, but will increase to add a safety margine
    SSP1ADD = 79;//(int) (_XTAL_FREQ/(4*600000))-1;
}
void SetUpI2C(){
    SSP1CON1 =0x28;
    
    SetUpI2CPins();

    SetUpIC2Clock();
    
}


int IsColorDataReady(){
    StartIC2Transmission();
    
    StartWriteRequest();
    
    char input = (char) COLOR_STATUS_ADDRESS;
    WriteChar(input);
    
    //EndIC2Transmission();
    
    RepeatedStart();
    //StartIC2Transmission();
    StartReadRequest();
    
   
    int value = ReadChar();
    value = value & 0x1;
    
    EndIC2Transmission();
    return value;
}

void WriteChar(char input){
    SSP1BUF = input;

    while(!PIR3bits.SSP1IF){}
    while(SSP1CON2bits.ACKSTAT); // Wait for acknowledgment
    PIR3bits.SSP1IF = 0;
}

int ReadChar(){
    
    SSP1CON2bits.RCEN = 1;
    while(!PIR3bits.SSP1IF){}
    PIR3bits.SSP1IF = 0;
    int value = SSP1BUF;
    SSP1CON2bits.ACKDT = 0;
    return value;
}
void StartWriteRequest(){
    SSP1BUFbits.SSPBUF = COLOR_ADDRESS_WRITE;

    while(!PIR3bits.SSP1IF){}
    while(SSP1CON2bits.ACKSTAT); // Wait for acknowledgment
    PIR3bits.SSP1IF = 0;
}

void StartReadRequest(){
    SSP1BUF = COLOR_ADDRESS_READ;
    while(!PIR3bits.SSP1IF){}
    while(SSP1CON2bits.ACKSTAT); // Wait for acknowledgment
    PIR3bits.SSP1IF = 0;
}

void StartIC2Transmission(){
    SSP1CON2bits.SEN = 1;
    PIR3bits.SSP1IF = 0;
    //while(SSP1CON2bits.SEN == 1){}
    while(!PIR3bits.SSP1IF){}
    PIR3bits.SSP1IF = 0;
}
void EndIC2Transmission(){
    SSP1CON2bits.PEN = 1;
    //while(SSP1CON2bits.PEN == 1){}
    while(!PIR3bits.SSP1IF){}
    PIR3bits.SSP1IF = 0;
}

void RepeatedStart(){
    SSP1CON2bits.RSEN = 1;
    //while(SSP1CON2bits.RSEN == 1){}
    while(!PIR3bits.SSP1IF){}
    PIR3bits.SSP1IF = 0;
}

void SetUpAnalog(){
    DAC1CON0bits.DAC1EN = 1;
    
    //enable DAC1CON0 to output analog signal to RB7
    DAC1CON0bits.DAC1OE2 = 1;
    
    TRISBbits.TRISB7 = 0;
    ANSELBbits.ANSB7 = 1;
}
//intializes the Sensor and sets checks that that ID matchs
int SetUp(){
    StartIC2Transmission();
    //SSP1CON2bits.ACKSTAT = 0;
    StartWriteRequest();
    
    
    
    SSP1BUF = COLOR_ENABLE_REGISTER_ADDRESS;

    while(!PIR3bits.SSP1IF){}
    while(SSP1CON2bits.ACKSTAT); // Wait for acknowledgment
    PIR3bits.SSP1IF = 0;
    
    
    SSP1BUF = COLOR_ENABLE;

    while(!PIR3bits.SSP1IF){}
    while(SSP1CON2bits.ACKSTAT == 1){}
    PIR3bits.SSP1IF = 0;
    
    RepeatedStart();
    
    
    
    StartWriteRequest();    
    
    SSP1BUF = COLOR_ID_REGISTER_ADDRESS;
    while(!PIR3bits.SSP1IF){}
    while(SSP1CON2bits.ACKSTAT); // Wait for acknowledgment
    PIR3bits.SSP1IF = 0;
    
    //EndIC2Transmission();
    RepeatedStart();
    //StartIC2Transmission();
    
    StartReadRequest();
    
    SSP1CON2bits.RCEN = 1;
    while(!PIR3bits.SSP1IF){}
    PIR3bits.SSP1IF = 0; 
    int value = SSP1BUF;
    SSP1CON2bits.ACKDT = 0;
    
    EndIC2Transmission();
    if(value == 0xAB){
        return 1;
    }
    return 0;
}



int ReadClear(){
    StartIC2Transmission();
    StartWriteRequest();
    char input = (char) COLOR_CLEAR_LOW;
    WriteChar(input);
    //EndIC2Transmission();
    
    RepeatedStart();
    StartReadRequest();
    
    int redLow = ReadChar();
    EndIC2Transmission();
        //RepeatedStart();
    StartIC2Transmission();
    StartWriteRequest();
    input = (char) COLOR_CLEAR_HIGH;
    WriteChar(input);
    RepeatedStart();
    StartReadRequest();
    
    int redHigh = ReadChar();
    EndIC2Transmission();
    return (redHigh << 8) + redLow;
}

int ReadRed(){
    
    StartIC2Transmission();
    StartWriteRequest();
    char input = (char) COLOR_RED_LOW;
    WriteChar(input);
    //EndIC2Transmission();
    
    RepeatedStart();
    StartReadRequest();
    
    int redLow = ReadChar();
    
    EndIC2Transmission();
    //RepeatedStart();
   StartIC2Transmission();
    
    StartWriteRequest();
    input = (char) COLOR_RED_HIGH;
    WriteChar(input);
    RepeatedStart();
    StartReadRequest();
    
    int redHigh = ReadChar();
    EndIC2Transmission();
    return (redHigh << 8) + redLow;
}

int ReadBlue(){
    
    StartIC2Transmission();
    StartWriteRequest();
    char input = (char) COLOR_BLUE_LOW;
    WriteChar(input);
    //EndIC2Transmission();
    
    RepeatedStart();
    //StartIC2Transmission();
    StartReadRequest();
    
    int redLow = ReadChar();
    
    EndIC2Transmission();
    //RepeatedStart();
    StartIC2Transmission();
    StartWriteRequest();
    input = (char) COLOR_BLUE_HIGH;
    WriteChar(input);
    EndIC2Transmission();
    
    
    StartIC2Transmission();
    StartReadRequest();
    
    int redHigh = ReadChar();
    EndIC2Transmission();
    return (redHigh << 8) + redLow;
}

int ReadGreen(){
    
    StartIC2Transmission();
    StartWriteRequest();
    char input = (char) COLOR_GREEN_LOW;
    WriteChar(input);
    //EndIC2Transmission();
    
    RepeatedStart();
    //StartIC2Transmission();
    StartReadRequest();
    
    int redLow = ReadChar();
    
    EndIC2Transmission();
    //RepeatedStart();
    StartIC2Transmission();
    
    StartWriteRequest();
    
    input = (char) COLOR_GREEN_HIGH;
    WriteChar(input);
    EndIC2Transmission();
    
    
    StartIC2Transmission();
    StartReadRequest();
    
    int redHigh = ReadChar();
    EndIC2Transmission();
    return (redHigh << 8) + redLow;
}

void SelectColourRegister(int colorStage){
    unsigned char writeValue = 0;
    switch(colorStage){
        case 0:writeValue = COLOR_STATUS_ADDRESS; break;
        case 1:  writeValue = COLOR_CLEAR_LOW; break;
        case 2: writeValue = COLOR_CLEAR_HIGH; break;
        case 3: writeValue = COLOR_RED_LOW; break;
        case 4: writeValue = COLOR_RED_HIGH; break;
        case 5: writeValue =COLOR_BLUE_LOW; break;
        case 6: writeValue = COLOR_BLUE_HIGH; break;
        case 7: writeValue = COLOR_GREEN_LOW; break;
        case 8: writeValue = COLOR_GREEN_HIGH; break;
    }
    SSP1BUFbits.SSPBUF = writeValue;
}

void Reset(ColorScheme * colors){
    colors->colorStage = 0;
    colors->ready = 0;
    colors->allDone = 1;
}
void StartI2C(){
    PIR3bits.SSP1IF = 0; 
    SSP1CON2bits.SEN = 1;
}
void EndI2C(){
    SSP1CON2bits.PEN = 1;
}
void WriteI2C(){
    PIR3bits.SSP1IF = 0;
    SSP1BUFbits.SSPBUF = COLOR_ADDRESS_WRITE;
}
void ReadAddressI2C(){
    PIR3bits.SSP1IF = 0;
    SSP1BUFbits.SSPBUF = COLOR_ADDRESS_READ;
}
void StartReadI2C(){
    PIR3bits.SSP1IF = 0;
    SSP1BUF = COLOR_ADDRESS_READ;
}
void ReadI2C(){
    SSP1CON2bits.RCEN = 1;
}
void RepeatI2C(){
    //PIR3bits.SSP1IF = 0;
    SSP1CON2bits.RSEN = 1;
}
void IncrementColorStage(ColorScheme * colors){
    colors->colorStage = colors->colorStage +1;
    
}
void LoadTemp(ColorScheme * colors, unsigned char value){
    colors->temp = value; 
}
void ReadColorReady(ColorScheme * colors, unsigned char buffer){
    buffer = buffer & 0x1;
    colors->ready = buffer;
    if(colors->ready == 1){
        IncrementColorStage(colors);
    }
}

void GetValueAndEndI2C(ColorScheme* colors){
    
    unsigned char value = SSP1BUF;
    switch(colors->colorStage){
        //check if colors are ready
        case 0: ReadColorReady(colors,value); break;
        case 1: LoadTemp(colors,value); IncrementColorStage(colors);break;
        case 2: colors->clear = colors->temp + (value<<8); IncrementColorStage(colors); break;
        case 3: LoadTemp(colors,value); IncrementColorStage(colors);;
        case 4: colors->red = colors->temp + (value <<8); IncrementColorStage(colors); break;
        case 5: LoadTemp(colors,value); IncrementColorStage(colors);break;
        case 6: colors->blue = colors->temp + (value << 8); IncrementColorStage(colors); break;
        case 7: LoadTemp(colors,value);IncrementColorStage(colors); break;
        case 8: colors->green = colors->temp + (value << 8); Reset(colors); break;
    }
    EndI2C();
    
}

unsigned char UpdateColors(ColorScheme * colors, int stageValue){
    switch(stageValue){
        case 0: StartI2C();break;
        case 1: WriteI2C(); break;
        case 2: SelectColourRegister(colors->colorStage); break;
        case 3: RepeatI2C(); break; //RepeatedStart(); break;
        case 4: ReadAddressI2C(); break;
        case 5: ReadI2C(); break;
        case 6: GetValueAndEndI2C(colors);  break;
    }
    return (stageValue+1) % 7;
}

void PlayTune(unsigned char color){
    if(color == RED){
            DAC1CON1bits.DAC1R = 31;
            __delay_ms(NOTE_C4/2);
            DAC1CON1bits.DAC1R = 0;
            __delay_ms(NOTE_C4/2);
        }
        else if(color == BLUE){
            DAC1CON1bits.DAC1R = 31;
            __delay_ms(NOTE_A4/2);
            DAC1CON1bits.DAC1R = 0;
            __delay_ms(NOTE_A4/2);
        }
        else{
            DAC1CON1bits.DAC1R = 31;
            __delay_ms(NOTE_F4/2);
            DAC1CON1bits.DAC1R = 0;
            __delay_ms(NOTE_F4/2);
        }
}

//CALL THIS METHOD IN MAIN, not others
void SetUpColorSensor(){
    //3.3V to Vin
    //ground to GND
    //SDA on board (RC3) to SCL on sensor
    //SCL on board (RC5) to SDA on sensor
    
    SetUpI2C();
    int result = SetUp();
    if(result == 1){
        LATAbits.LATA0 = 1;
        LATAbits.LATA1 = 1;
        LATAbits.LATA2 = 1;
        LATAbits.LATA3 = 1;
        __delay_ms(500);
        LATAbits.LATA0 = 0;
        LATAbits.LATA1 = 0;
        LATAbits.LATA2 = 0;
        LATAbits.LATA3 = 0;
        __delay_ms(500);
        
        LATAbits.LATA0 = 1;
        LATAbits.LATA1 = 1;
        LATAbits.LATA2 = 1;
        LATAbits.LATA3 = 1;
        __delay_ms(500);
        LATAbits.LATA0 = 0;
        LATAbits.LATA1 = 0;
        LATAbits.LATA2 = 0;
        LATAbits.LATA3 = 0;
        __delay_ms(500);
        
    }
}

void ColorSensor(unsigned char *newI2CMessage,int *I2CStage, ColorScheme* colors, unsigned char * color){
    if(*newI2CMessage == 1 ){
            *newI2CMessage =0;
            *I2CStage = UpdateColors(colors,*I2CStage);

            if(colors->allDone){
                if(colors->red > colors->blue && colors->red > colors->green){
                    LATAbits.LATA0 = 1;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA2 = 0;
                    *color = RED;
                }
                else if(colors->blue > colors->red && colors->blue > colors->green){
                    LATAbits.LATA0 = 0;
                    LATAbits.LATA1 = 1;
                    LATAbits.LATA2 = 0;
                    *color = BLUE;
                }
                else if(colors->green > colors->blue && colors->green > colors->red){
                    LATAbits.LATA0 = 0;
                    LATAbits.LATA1 = 0;
                    LATAbits.LATA2 = 1;
                    *color = GREEN;
                }
            }
        }
    PlayTune(*color);
    
}




#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* COLOR_HEADER_H */

