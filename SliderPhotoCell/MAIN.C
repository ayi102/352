/*Summary:
The objective for Lab 6 was to implement the PhotoCell, green LED and the A/D converter to get readings
from the capacitive touch pad. Ideally, I was to create a program that demonstrated all three components.
My program measures light coming through the photocell and displays a value that measure if it is dark or
light. The darker the greater the value. When the value reaches 600, the green LED turns on. As for the capacitive
touch pad, all six voltage values for each pad is displayed on the LCD screen. Also, the corresponding letter for each
pad appears on the screen as each pad is touched. The algorithm for the Photocell works as follows: the output is turned off,
there is a tenth of a second pause, and then the capicotor begins to charge. A for loop counts until the PHOTOCELL_IN flag goes low
and the count value is converted into a 4 digit ASCII values and displayed on the LCD. Then, the count value is compared to 
600. If it is larger than this value, the LED is turned on otherwise it turns off. The capacitive slider works in a similar way.
The first mux channell is chosen to read from, a GO bit is set high to start conversion. Next, the algorithm waits until the A/D
is finished and then it stores the data for the first pad in an Array. The next 17 readings are stored in the array, 3 for each pad.
Next, the averages for each pad is stored in an Array. The array is sent to function that finds the largest values and returns the
index of the larget value. The index represents the pad that is being touched. All of the average values of the pads are displayed, 
but the pad that is being touched has its representive letter displayed on the LCD screen. The slider algorithm compares the largest
cap value to a fixed value that ensures that the value has been touched.*/

/*FUNCTIONALITY:
 The LCD displays the PHOTOCELL VALUE, the 6 slider values, and, if a pad is touched, its corresponding letter
will be displayed.

                                                   0000 (PHOTOCELL VALUE)
                                        0000 (S)   0000(L)   0000(I)
                                        0000 (D)   0000(E)   0000(R)
                                        SLIDER
This is what the output should look like.*/

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
void tmr2_isr(void);
#pragma code high_vector=0x08
void interrupt_at_high_vector(void) {
    _asm GOTO tmr2_isr _endasm
}
#pragma code
//----------------------------------------------------
// Set the vector for the high priority interrupt
//----------------------------------------------------
//void tmr1_isr(void);
//#pragma code high_vector=0x08
//void interrupt_at_high_vector(void) {
//    _asm GOTO tmr1_isr _endasm
//}
//#pragma code
//----------------------------------------------------
// Set the vector for the low priority interrupt
//----------------------------------------------------
void tmr0_isr(void);
#pragma code low_vector=0x18
void interrupt_at_low_vector(void) {
    _asm GOTO tmr0_isr _endasm
}
#pragma code

//Global Variables
extern BYTE tenthflag;

/* display letters depending on which pad is touched*/
CONST char mesg02[] = "S";
CONST char mesg03[] = "L";
CONST char mesg04[] = "I";
CONST char mesg05[] = "D";
CONST char mesg06[] = "E";
CONST char mesg07[] = "R";
CONST char mesg08[] = "                ";
CONST char mesg09[] = "Button to Launch";


void main(void)
{
   // Index that recognizes which pad is being touched
   char slider = 0;
   
   //Initializations
   Init();
   DispClear();
   DispUpdate();
   AtoDInit();

   while (1)
   {
      DispConstText(3,0,mesg09);
      //Call PhotoCell function to activate PhotoCell
      PhotoCell();

      //Call CapSlider to identify which pad is being touched
      slider = CapSlider();

      //Display the letter that corresponds with the correct touch pad
      //Otherwise display a blank line
      switch (slider)
      {
         case SLIDER_S:  DispConstText(3,0,mesg02);  break;
         case SLIDER_L:  DispConstText(3,1,mesg03);  break;
         case SLIDER_I:  DispConstText(3,2,mesg04);  break;
         case SLIDER_D:  DispConstText(3,3,mesg05);  break;
         case SLIDER_E:  DispConstText(3,4,mesg06);  break;
         case SLIDER_R:  DispConstText(3,5,mesg07);  break;
         default: DispConstText(3,0,mesg08);break;
      }
      DispUpdate();
   }
}
