
 /**************************************
 *                                     *
 *   IIIIII  NN   NN  IIIIII  TTTTTT   *
 *     II    NNN  NN    II      TT     *
 *     II    NNNN NN    II      TT     *
 *     II    NN NNNN    II      TT     *
 *     II    NN  NNN    II      TT     *
 *     II    NN   NN    II      TT     *
 *   IIIIII  NN   NN  IIIIII    TT     *
 *                                     *
 **************************************/

#include <p18F4525.h>
#include "dev12.h"


#define T05MSEC         (0xFEC0) /* 5 msec interrupt */
#define T1_HALFMSEC     (0xFE0C) /* .5 msec interrupt */
#define PAUSE		    (0xFFF0) /* 10 count*/
#define DAC_START       0   
#define DAC_ADDRESS     1      
#define DAC_COMMAND     2
#define DAC_DATA        3
#define DAC_PAUSE       4
#define DAC_STOP	    5
#define MSECS           10
#define MICROPHONE      6

// Global Variables
BYTE state = DAC_START;
unsigned int index = 0;
unsigned int index2 = 0; 
extern struct FinalLab param;
BYTE pauseFlag = 0;
static unsigned int sysclk = 0;       /* 5 msec system clock counter */
static BYTE sysclk2 = 0;
BYTE keydata = 0;             /* keypad ascii character (or 0=none) */

// Sine voltage values
static BYTE sin[64] = {
64, 70, 76, 82, 87, 93, 98, 102, 107, 111, 114, 117, 120, 122, 123, 124,
124, 124, 123, 122, 120, 117, 114, 111, 107, 102, 98, 93, 87, 82, 76, 70,
64, 58, 53, 47, 41, 36, 31, 26, 22, 18, 14, 11, 9, 7, 5, 5,
4, 5, 5, 7, 9, 11, 14, 18, 22, 26, 31, 36, 41, 47, 53, 58
};

//----------------------------------------------
// Pic Initialization
//----------------------------------------------
void Init(void)
{
   BYTE ofst;

   PicInit();
   SpiInit();
   DispInit();
}

void PicInit (void)
{
    //----------Configure the Oscillator ---------------
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;
    OSCTUNEbits.PLLEN = 1;      // enable the 4xPLL

    //----------Configure A-to-D inputs----------------
    ADCON1 = 0b00001000;    // AN0-AN6 analog channels
    ADCON0 = 0b00000001;    // Select channel AN0
    ADCON2 = 0b00010010;    // Left justify, 4 Tad acquisition time, 32 Tosc
    ADCON0bits.ADON = 1;    // Turn on ADC

    //------- Configure keypad keys ----------------
    TRIS_COL3 = INPUT;
    TRIS_COL2 = INPUT;
    TRIS_COL1 = INPUT;
    TRIS_ROW4 = INPUT;
    TRIS_ROW3 = INPUT;
    TRIS_ROW2 = INPUT;
    TRIS_ROW1 = INPUT;
    COL3_PIN = 0;
    COL2_PIN = 0;
    COL1_PIN = 0;

    TRIS_LED3 = OUTPUT;
    LED3_PIN = 0;

    TRIS_LED4 = OUTPUT;
    LED4_PIN = 0;

    // TRISE &= 0xEF;   // disable PSP
    TRISEbits.PSPMODE = 0;  // disable Parallel Slave Port

    // 32Mhz/ 9.6k/16 SPBRGH = 207
    //  4Mhz/ 9.6k/16 SPBRGH = 207/8 = 26
    //  OpenUSART( USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE &
    //             USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 207);

    // configure timer0 as a vanilla prescaled 16-bit timer
    T0CONbits.TMR0ON = 1;       // turn on TMR0
    T0CONbits.T08BIT = 0;       // configured as 16-bit
    T0CONbits.T0CS = 0;         // clock source is Fosc/4
    T0CONbits.PSA = 0;          // enable prescalar
    T0CONbits.T0PS2 = 1;        // 1:128 prescalar for 1khz second interval
    T0CONbits.T0PS1 = 1;
    T0CONbits.T0PS0 = 0;
    TMR0H = (BYTE)(T05MSEC >> 8);
    TMR0L = (BYTE)(T05MSEC & 0xff);
    INTCON2bits.TMR0IP = 0;     // Timer0 will be a low priority interrupt
    INTCONbits.TMR0IE = 1;      // Enable interrupts
    
    // configure timer1 as a vanilla prescaled 16-bit timer
    T1CONbits.RD16 = 1;         // 16 bit operation
    T1CONbits.T1CKPS1 = 1;              // 1:8 prescalar 
    T1CONbits. T1CKPS0 = 1;            
    T1CONbits.T1OSCEN =0;               // Turn Oscillator OFF
    T1CONbits.TMR1CS = 0;               // Use FOSC/4 clock(8Mhz)
    T1CONbits.TMR1ON = 1;               // Turn Timer ON      
    TMR1H = (BYTE)(T1_HALFMSEC >> 8);
    TMR1L = (BYTE)(T1_HALFMSEC & 0xff);
    IPR1bits.TMR1IP = 1;                // Timer1 will be high priority interrupt
    PIE1bits.TMR1IE = 0;                // Disable interrupts


   // Enable low and high priority interrupts
   RCONbits.IPEN = 1;                   // enable priority level interrupts
   INTCONbits.PEIE = 1;                 // Enable the perhiphal interrupts
   INTCONbits.GIEH = 1;                 // Let the high priority interupts begin
   INTCONbits.GIEL = 1;                 // Let the low priority interupts begin


    CCP1CON = 0b00001100;       // Setup CCP1 = RC2 in PWM mode
    PR2 = 0xFF;                 // 256uS period
    CCPR1L = 0x80;

    // Enable low and high priority interrupts
    RCONbits.IPEN = 1;          // enable priority level interrupts
    INTCONbits.PEIE = 1;        // Enable the perhiphal interrupts
    INTCONbits.GIEH = 1;        // Let the high priority interupts begin
    INTCONbits.GIEL = 1;        // Let the low priority interupts begin
}

//-----------------------------------------------------------------------------
// Initializes the MSSP to run an SPI interface for the display
//-----------------------------------------------------------------------------
void SpiInit(void)
{
    SSPCON1bits.SSPEN = 0;  // disable SPI module during preliminary initialization
    //TRIS_WP = INPUT;        // pull up resistor on ths line disables the EEPROM

    //---------------DISPLAY CONFIGURATION------------------------
    TRIS_SDO = OUTPUT;      // Nominal values set by SPI init below
    TRIS_SDA = INPUT;       // Nominal values set by SPI init below
    TRIS_SCL = OUTPUT;
    TRIS_A0 = OUTPUT;
    TRIS_RST = OUTPUT;      // Nominal value set below
    TRIS_CS1B = OUTPUT;     // Nominal value set in SEND_DISP
    RST_PIN = 1;            // active low reset
    CS1B_PIN = 1;           // chip select, active low

    SSPCON1bits.CKP = 1;        // Idle state for clock = 1
    SSPSTATbits.CKE = 0;        // Transmitt occurs on low to high
    SSPCON1bits.SSPM3 = 0;      //      50nS min clock period
    SSPCON1bits.SSPM2 = 0;      //
    SSPCON1bits.SSPM1 = 0;      //      SPI clock rate = FOSC/4
    SSPCON1bits.SSPM0 = 1;      //
    SSPCON1bits.SSPEN = 1;      // enable SPI module
    PIE1bits.SSPIE = 0;         // Clear any flags that may have been set
    PIR1bits.SSPIF = 0;         // Clear any flags that may have been set
}
//-----------------------------------------------------------------------------
//Initializes the Digital to Analog Converter
//-----------------------------------------------------------------------------
void DacInit(void)
{
   state = DAC_START;
   //(assumes that TIMER-1 is set for 1 msec high-priority interrupt)
   SSPCON1bits.SSPEN = 0;     // disable SPI module during initialization
   TRIS_SDO = INPUT;          // RC4 - pull up resistor needed
   TRIS_SCL = INPUT;          // RC3 - pull up resistor needed
   SSPSTATbits.CKE = 0;       // Disable SMBus inputs (I2C)
   SSPSTATbits.SMP = 0;       // Enable slew rate control for 400kHz
   SSPADD = 19;               // 400kHz with a 32Mhz clock (page 187)
   SSPCON1 = 0x28;            // I2C Master mode. Clock = Fosc/(4*SPADD)
   SSPCON2 = 0x00;            // Just in case, reset values
   PIR1bits.SSPIF = 0;        // Clear any flags that may have been set
   PIE1bits.SSPIE = 1;        // enable the interrupt (default is high priority)
   // *** skip this if you are not using interrupts
   SSPCON1bits.SSPEN = 1;  
   PIE1bits.TMR1IE = 1;       // enable timer interrupt
}
//-----------------------------------------------------------------------------
// Initializes the AtoD
//-----------------------------------------------------------------------------
void AtoDInit(void)
{
//----------Configure A-to-D inputs----------------

    TRISAbits.TRISA2 = 1;         //(set each Analog channel as an “input”)
    ADCON1 = 0x08;                //(1000)(selects AN0, AN1, AN2,AN3,AN4,AN5 as potential a-to-d inputs)
    //(disables the pins for use as general purpose I/O)
    ADCON2bits.ACQT2 = 0;       //(001) (sets the acquisition time)
    ADCON2bits.ACQT1 = 0;       //(001) (sets the acquisition time)
    ADCON2bits.ACQT0 = 1;       //(001) (sets the acquisition time)
    ADCON2bits.ADCS2 = 0;       //(010) (sets the acquisition clock)
    ADCON2bits.ADCS1 = 1;       //(010) (sets the acquisition clock)
    ADCON2bits.ADCS0 = 0;       //(010) (sets the acquisition clock)
    ADCON2bits.ADFM = 0;          // left justified
    ADCON0bits.ADON = 1;          //(enables the a-to-d circuitry)
}
//-----------------------------------------------------------------------------
// High priority TMR1 interrupt, high frequency
//-----------------------------------------------------------------------------
#pragma interrupt tmr1_isr
void tmr1_isr(void)
{
   /* reset the timer */
   PIR1bits.TMR1IF = 0;
   PIR1bits.SSPIF = 0;


      switch (state)
      {
         case DAC_START:
         //Assert the start condition
         SSPCON2bits.SEN = 1;
         //reset sysclk
         sysclk = 0;
         // Move to the next state
         state = DAC_ADDRESS;
         break;

         case DAC_ADDRESS:
         // write device address and zeros to DAC
         SSPBUF = 0xC0;
         // Move to next state
         state = DAC_COMMAND;
         break;

         case DAC_COMMAND:
         // Disable interrupt
         PIE1bits.TMR1IE = 0;
         // Write zeros to DAC
         SSPBUF = 0x00;
         // Move to next State
         state = DAC_DATA;
         break;

         case DAC_DATA:
         /*calculate sine table index based on the frequencies of key press*/
         index = (index + param.Freq) & (0x3ff);
         index2 = (index2 + param.Freq2) & (0x3ff);
         //Add the tones together and write to the DAC
         SSPBUF = ((sin[index>>4] + sin[index2>>4])) >> 1;
         // Run through tone for 1 second
         if(++sysclk == 2000)
         {
             //turn off the dac
             param.tone_flag = FALSE;
         }
         //check if the dac should stay on
         if (param.tone_flag == TRUE)
            state = DAC_PAUSE;
         else
            state = DAC_STOP;
         break;

         // Reset time timer, enable interrupt, move to Command state
         case DAC_PAUSE:
         TMR1H = (BYTE)(PAUSE >> 8);
         TMR1L = (BYTE)(PAUSE & 0xff);
         PIE1bits.TMR1IE = 1;
         state = DAC_COMMAND;

         break;

         // assert stop bit to high
         case DAC_STOP:
         SSPCON2bits.PEN = 1;
         // send algorithm back to the beginning
         state = DAC_START;
         // reset the indicies
         index = 0;
         index2 = 0;
         // wait for the interrupt to be finished
         while(!PIR1bits.SSPIF);
         // set interrupt flag low
         PIR1bits.SSPIF = 0;
      }
   
}
//-----------------------------------------------------------------------------
// Low priority TMR0 interrupt, low frequency
//-----------------------------------------------------------------------------
#pragma interruptlow tmr0_isr
void tmr0_isr(void)
{
   /* keypad column state flag */
   static BYTE keyoff = 0;

   /* reset the timer */
   TMR0H = (BYTE)(T05MSEC >> 8);
   TMR0L = (BYTE)(T05MSEC & 0xff);
   INTCONbits.TMR0IF = 0;

  
   /* increment the 5 msec timer */
   sysclk2++;
   if(sysclk2 == MSECS)
   {
      sysclk2 = 0;
      pauseFlag = TRUE;
   } 
   //choose the sensor reading sound
   //start reading
   SelectSensor(MICROPHONE);


   /*Continously Poll*/
   if (1)
   {
      switch (++keyoff)
      {
         case 1:
             Debounce(ROW1_PIN,ROW2_PIN, ROW3_PIN, ROW4_PIN);

             COL1_PIN = 0;   TRIS_COL1 = INPUT;
             COL2_PIN = 1;   TRIS_COL2 = OUTPUT;
             break;
         case 2:

             Debounce2(ROW1_PIN,ROW2_PIN, ROW3_PIN, ROW4_PIN);
             COL2_PIN = 0;   TRIS_COL2 = INPUT;
             COL3_PIN = 1;   TRIS_COL3 = OUTPUT;
             break;
         default:
             Debounce3(ROW1_PIN,ROW2_PIN, ROW3_PIN, ROW4_PIN);

             COL3_PIN = 0;   TRIS_COL3 = INPUT;
             COL1_PIN = 1;   TRIS_COL1 = OUTPUT;
             keyoff = 0;
             break;
      }
   }

}

/* return the system 5 msec time clock */
BYTE clock(void)
{
   return(sysclk);
}
/* Debounce the pushbuttons*/
void Debounce(BYTE BtnStatus1,BYTE BtnStatus2,BYTE BtnStatus3,BYTE BtnStatus4)
{
   //Declare Array to hold status of buttons
   static BYTE Status[4][3];

   //History of the first Button
   Status[0][0] = Status[0][1];
   Status[0][1] = Status[0][2];
   Status[0][2] = BtnStatus1;

   //Check status of the first button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[0][2] == 1 && Status[0][2] == Status[0][1] && Status[0][0] == 0 )
   {
       keydata = '3';
   }

   //History of the second Button
   Status[1][0] = Status[1][1];
   Status[1][1] = Status[1][2];
   Status[1][2] = BtnStatus2;

   //Check status of the second button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[1][2] == 1 && Status[1][2] == Status[1][1] && Status[1][0] == 0 )
   {
       keydata = '6';
   }

   //History of the third Button
   Status[2][0] = Status[2][1];
   Status[2][1] = Status[2][2];
   Status[2][2] = BtnStatus3;

   //Check status of the third button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[2][2] == 1 && Status[2][2] == Status[2][1] && Status[2][0] == 0 )
   {
       keydata = '9';
   }

   //History of the fourth Button
   Status[3][0] = Status[3][1];
   Status[3][1] = Status[3][2];
   Status[3][2] = BtnStatus4;

   //Check status of the fourth button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[3][2] == 1 && Status[3][2] == Status[3][1] && Status[3][0] == 0 )
   {
       keydata = '#';
   }

}
/* Debounce the pushbuttons*/
void Debounce2(BYTE BtnStatus1,BYTE BtnStatus2,BYTE BtnStatus3,BYTE BtnStatus4)
{
   //Declare Array to hold status of buttons
   static BYTE Status[4][3];

   //History of the first Button
   Status[0][0] = Status[0][1];
   Status[0][1] = Status[0][2];
   Status[0][2] = BtnStatus1;

   //Check status of the first button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[0][2] == 1 && Status[0][2] == Status[0][1] && Status[0][0] == 0 )
   {
       keydata = '2';
   }

   //History of the second Button
   Status[1][0] = Status[1][1];
   Status[1][1] = Status[1][2];
   Status[1][2] = BtnStatus2;

   //Check status of the second button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[1][2] == 1 && Status[1][2] == Status[1][1] && Status[1][0] == 0 )
   {
       keydata = '5';
   }

   //History of the third Button
   Status[2][0] = Status[2][1];
   Status[2][1] = Status[2][2];
   Status[2][2] = BtnStatus3;

   //Check status of the third button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[2][2] == 1 && Status[2][2] == Status[2][1] && Status[2][0] == 0 )
   {
       keydata = '8';
   }

   //History of the fourth Button
   Status[3][0] = Status[3][1];
   Status[3][1] = Status[3][2];
   Status[3][2] = BtnStatus4;

   //Check status of the fourth button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[3][2] == 1 && Status[3][2] == Status[3][1] && Status[3][0] == 0 )
   {
       keydata = '0';
   }
}
/* Debounce the pushbuttons*/
void Debounce3(BYTE BtnStatus1,BYTE BtnStatus2,BYTE BtnStatus3,BYTE BtnStatus4)
{
   //Declare Array to hold status of buttons
   static BYTE Status[4][3];

   //History of the first Button
   Status[0][0] = Status[0][1];
   Status[0][1] = Status[0][2];
   Status[0][2] = BtnStatus1;

   //Check status of the first button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[0][2] == 1 && Status[0][2] == Status[0][1] && Status[0][0] == 0 )
   {
       keydata = '1';
   }

   //History of the second Button
   Status[1][0] = Status[1][1];
   Status[1][1] = Status[1][2];
   Status[1][2] = BtnStatus2;

   //Check status of the second button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[1][2] == 1 && Status[1][2] == Status[1][1] && Status[1][0] == 0 )
   {
       keydata = '4';
   }

   //History of the third Button
   Status[2][0] = Status[2][1];
   Status[2][1] = Status[2][2];
   Status[2][2] = BtnStatus3;

   //Check status of the thirst button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[2][2] == 1 && Status[2][2] == Status[2][1] && Status[2][0] == 0 )
   {
       keydata = '7';
   }

   //History of the fourth Button
   Status[3][0] = Status[3][1];
   Status[3][1] = Status[3][2];
   Status[3][2] = BtnStatus4;

   //Check status of the fourth button
   // If there is a transition from low to active and active for two polls
   // There has been a key press
   if(Status[3][2] == 1 && Status[3][2] == Status[3][1] && Status[3][0] == 0 )
   {
       keydata = '*';
   }

}