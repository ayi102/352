//SUMMARY
//The objective for this final project was to implement Dual-Tone Multi-Frequency Dialing.
//DTMF works as follows, two tones at two different frequencies are added together to create
//a dual-tone. This dual tone reprepents a specific digit or character that can be recognized
//by a telephone on a land line. The phone acts as a speaker waiting to receive these
//dual-tones. Overall, I was able to produce the dual-tones for each digit and character and added
//many phone like features that are emulated by my PCB. My PCB is capable of saving
//two phone numbers, Speedial, Redial, Sound recognition, and a formal dialer. My algorithm works
//as follows, first the program prompts the user to go to SaveMode or DialMode. If the user presses
//'1', they go to the SaveMode. The user is then prompted to choose from 1 of 2 slots to save to.
//When a slot is chosen, the algorithm displays "saving.." and waits for the user to press a key.
//When a key is pressed, it is saved to the chosen slot and then displayed to the LCD display. When
//the user has chosen 10 digits, the algorithm saves the information to the EEProm and returns to
//the Main Menu. If the user presses '2', the user will go to DialMode. A new menu is displayed and
//prompts the user to speedial the first saved number('1'), prompts the user to speedial the second saved number('2'),
//prompts the user to dial a regular phone number('3') and prompts to Redial the most recent dialed number('4').
//The algorithm waits for a key press or the user to say "Quit", which will quit to the main menu.
//The algorithm polls the microphone for noise and when if it reaches a certain threshold, the program
//will signal the quit flag. Otherwise, if the user chooses a valid key the LCD will display "Dialing..."
//If the user pressed '1','2', or '4' the algorithm will read the EEProm for the 10 digit number
//the user wants to speedial. Speedial works as follows, a key is dialed, displayed, a pause occurs,
//and this is done until all ten digits have been dialed. Then there is another pause to make sure
//all the digits are displayed before going back to the DialMode option Menu. If the user chooses '3',
//they want to manually enter a 10 digit number. This mode receives the key, dials the key, saves the
//key to the struct, and then the digit is displayed. This is done util all 10 digits have been dialed.
//The struct is then written to the EEPROM and the algorithm returns to the DialMode Option Menu.
//
//In addition, to dial each digit there is communication between a dialer function and a state table in
//TIMER1. The Dialer function receives a key, sets two frequency variables that correspond to that key, enables
//the DAC to run continously, intializes the DAC and waits for the DAC to finish outputting the tone
//for about a second and then quits. When the dialer function writes the two frequency variables, these values
//are written to two global variables used by the state table in TIMER1. The algorithm begins in the
//DAC_START state that asserts start condition, initializes a sysclk2 variable to keep track of elapsed time.
//The algorithm moves to the DAC_ADDRESS state. In this state the 1st Byte (address) is written to the DAC
//and then the state is set to go to the DAC_COMMAND state. In DAC_COMMAND, the interrupt is disabled, the 2nd byte
//is written to the DAC(0x00) and the state is set to DAC_DATA. In DAC_DATA, the global frequencies assigned in
//the dialer function are used to calculated two indicies. The two indicies are calucalted used a floating method
//to gain accurate manipulation of the frequencies output by the DAC. These two indicies are added together and
//used to access the sine table. The sine table contains different voltage levels that are written as the 3rd byte
//to the DAC. DAC_DATA also increment sysclk2 and checks if the variable reaches a certain value(2000). When this
//happens, the DAC goes to DAC_STOP where the stop bit of the DAC is set high, the state is set to go
//back to DAC_START, the indexes are reset, there is a wait for the interrupt to be finished, and finally the
//interrupt flag is finished.Otherwise, the algorithm goes to DAC_PAUSE state where the timer is set for 10 counts.
//To achieve more accuracy, the timer had to be set to 10 counts to output a cleaner dual-tone.

//FUNCTIONALITY
//First Menu Option
//   1 = SAVE MODE
//   2 = DIAL MODE
// *(SAVE MODE)
//   1 = SAVE 1ST #
//   2 = SAVE 2ND #
// *If you press 1 or 2 you get this
//   Saving...
//
//
//   7243473108
// * Goes back to First Menu Option
//
// *(DIAL MODE)
// 1 = SAVED NUM1
// 2 = SAVED NUM2
// 3 = KEYPAD    
// 4 = REDIAL    
// *If you press 1,2,3,4 it will look like this
//   Dialing...
//
//
//   7243473108
// *The only difference between 1,2,4 with 3 is that 3 will display a lot slower
//  since the digit are being manually pressed.
// *Goes back to DialMode Option Menu
// *Yell at microphone to go to First Menu Option


#include <p18F4525.h>
#include "dev12.h"


/* Set up the configuration bits */
#pragma config OSC = INTIO67
#pragma config STVREN = OFF
#pragma config PWRT = ON
#pragma config BOREN = OFF
#pragma config WDT = OFF
#pragma config PBADEN = OFF
#pragma config CCP2MX = PORTC
#pragma config MCLRE = ON
#pragma config LVP = OFF

//----------------------------------------------------
// The #pragma sets the location of the subsequent
// code to address 0x08.  The _asm directive tells
// the compiler to insert this assembly language
// instruction into the code.  The final #pragma
// tells the compiler to resume normal compilation.
//----------------------------------------------------
//void tmr2_isr(void);
//#pragma code high_vector=0x08
//void interrupt_at_high_vector(void) {
//    _asm GOTO tmr2_isr _endasm
//}
//#pragma code
//----------------------------------------------------
// Set the vector for the high priority interrupt
//----------------------------------------------------
void tmr1_isr(void);
#pragma code high_vector=0x08
void interrupt_at_high_vector(void) {
    _asm GOTO tmr1_isr _endasm
}
#pragma code

//----------------------------------------------------
// Set the vector for the low priority interrupt
//----------------------------------------------------
void tmr0_isr(void);
#pragma code low_vector=0x18
void interrupt_at_low_vector(void) {
    _asm GOTO tmr0_isr _endasm
}
#pragma code


void main(void)
{ 
   //Initializations
   Init();
   AtoDInit();
   
   while(1)
   {
      PromptSaveOrDial();
   }
}
