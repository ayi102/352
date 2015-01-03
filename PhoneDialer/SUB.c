#include <p18F4525.h>
#include "dev12.h"

/* display the functionality of the program*/

CONST char mesg01[] = "1 = SAVED NUM1";
CONST char mesg02[] = "2 = SAVED NUM2";
CONST char mesg03[] = "3 = KEYPAD    ";
CONST char mesg04[] = "4 = REDIAL    ";
CONST char mesg05[] = "Dialing...    ";
CONST char mesg06[] = "1 = SAVE MODE ";
CONST char mesg07[] = "2 = DIAL MODE ";
CONST char mesg08[] = "1 = SAVE 1ST #";
CONST char mesg09[] = "2 = SAVE 2ND #";
CONST char mesg10[] = "Saving...";

//Global Variables
struct FinalLab param = {0,0,0,0,0,1,0};
extern BYTE pauseFlag;

//Function: PromptSaveOrDial()
//Prompts the user to save a speed dial number or
//Prompts the user to go to the dialer mode
//Input: Nothing
//Returns: Nothing
void PromptSaveOrDial(void)
{
   BYTE key;

   //Initialize SPI
   InitDisp();
   DispClear();

   //Prompt the user
   DispConstText(0,0,mesg06);
   DispConstText(1,0,mesg07);
   DispUpdate();

   // wait for a key press and make sure it is valid(Not 0,#,*, or >2)
   while((key = KeyInput()) == 0 || key > BOUNDRY1 || key == BOUNDRY2 || key == BOUNDRY3 || key == BOUNDRY5);
   
   //1 and 2 correspond to the save and dial modes
   switch(key)
   {
      case '1':PromptForSave();break;
      case '2':DialMode();break;
   }
}
//Function: PromptForSave()
//Prompt the user a choice to save 2 numbers
//The numbers are then saves
//Input: nothing
//Output: nothing
void PromptForSave(void)
{  
   BYTE Digit;
   InitDisp();
   DispClear();

   //Display options to the user
   DispConstText(0,0,mesg08);
   DispConstText(1,0,mesg09);
   DispUpdate();

   //Wait for user choice but makes sure press is valid
   while((Digit = KeyInput()) == 0 || (Digit > BOUNDRY1 || Digit == BOUNDRY2 || Digit == BOUNDRY3 || Digit == BOUNDRY5));

   //Clear and display "saving..."
   DispClear();
   DispConstText(0,0,mesg10);
   DispUpdate();
  
   //Take the choice from the user and save the number
   switch(Digit)
   {
      case '1': SaveDial(param.FirstNum); break;
      case '2': SaveDial(param.SecondNum); break;
   }
}
//Function: SaveDial()
//Save 10 dials that the user dials in an array
//Save the array to the prom
//Input: an array holding a number
//Oupt: nothing
void SaveDial(BYTE data[])
{
   BYTE DialPress;
   BYTE cntr = 0;

   //Accept only 10 keypresses
   while(cntr < NUMBER)
   {
      // Wait for the user to dial a key
      while((DialPress = KeyInput()) == 0);
      //save the key press 
      data[cntr] = DialPress;
      //display the key press
      DispAscii(3,cntr,DialPress);
      cntr++;
   }
   //Save to the EEprom
   SavePrm((BYTE*)&param);
}
//Function: DialMode()
//This function simply takes the user to dial mode
//Input: Nothing
//Returns: nothing
void DialMode(void)
{
   BYTE choice = 0;
   //Stay in Dial Mode until the user decides to quit
   while(choice != 'Q')
   {
      //Get the command from the user
      choice = DispDialMode();
      //Check if the has quit the program
      if(choice != 'Q')
      {
         //Fullfill the comand of the user in Dial Mode
         ChoiceDialMode(choice);
      }
   }
}
//Function: DispDialMode()
//This function displays all of the options of the Dial Mode
//It prompts the user for a speed dial, redial, or regular dial
//Input: nothing
//Returns: Menu Option the User would like to run or if they want to quit
BYTE DispDialMode(void)
{
   BYTE key;
   InitDisp();
   // Display the functionality of the program
   DispConstText(0,0,mesg01);
   DispConstText(1,0,mesg02);
   DispConstText(2,0,mesg03);
   DispConstText(3,0,mesg04);
   DispUpdate();

   // wait for a key press and make sure it is valid or wait for the user to make a loud enought sound to quit Dial Mode
   while(((key = KeyInput()) == 0 && param.QuitFlag == FALSE) || (key > BOUNDRY4 || key == BOUNDRY2 || key == BOUNDRY3 || key == BOUNDRY5))
   {
      //Analyze the sound going into the microphone
      AnalyzeSound();
   }

   //Check if the user wants to quit the dialer
   //If yes, set the key to 'Q' to signal a quit
   if( param.QuitFlag == FALSE)
   {
      //Display "Dialing..."
      InitDisp();
      DispConstText(0,0,mesg05);
      DispUpdate();
   }
   else
   {
      key = 'Q';
      //Reset quit flag
      param.QuitFlag = FALSE;
   }

   return key;
}
//Function: AnalyzeSound()
//Writes all the data from the AtoD into an array
//Input: nothing
//Output: nothing
void AnalyzeSound(void)
{
   int data;
   BYTE QuitFlag = FALSE;
   
   //Get a reading from the AtoD and store in an array
   data = ReadSensor();

   // Check if the sound passes the threshold
   // Set the Quit Flag to TRUE
   if(data > AMPLITUDE)
   {
      param.QuitFlag = TRUE;
   }
}
//Function: SelectSensor()
//Sets up AtoD to read a specific sensor
//Input:  parm = atod input number (0,1,2,3…) 
//Output: nothing
BYTE SelectSensor(BYTE parm)
{
   //choose the Sensor
   ADCON0 = (parm << 2) + 1;
   // set the sensor to get ready to read
   ADCON0bits.GO = 1;
}
//Function: ReadSensor()
//Reads a specific sensor
//assumes the atod initialized as left justified 
//Input:  nothing 
//Output: returns the 8 bits of the sensor
BYTE ReadSensor(void) 
{
   // wait for the senosr is ready, then return the value
   while (ADCON0bits.GO != 0);
   return(ADRESH);
}
//Function: ChoiceDialMode()
//Takes the choice of the user and does the action the user wishes to do
//The user can Speed dial two numbers, Dial a number or Redial
//Input: choice of the user
//Returns: Nothing
void ChoiceDialMode(BYTE KEYS)
{
   //Case 1 and 2 read the save numbers in the prom and dials that number
   //Case 3 calls a function that displays the user types in
   //Case 4 dials the most recend dial of the user
   switch(KEYS)
   {
      case'1': ReadPrm((BYTE*)&param); SpeedDial(param.FirstNum); break;
      case'2': ReadPrm((BYTE*)&param); SpeedDial(param.SecondNum);break;
      case'3': DialAndDisplay(); break;
      case'4': ReadPrm((BYTE*)&param); SpeedDial(param.Redial); break;
   }
}
//Function: SpeedDial()
//Loops and grabs each digit the user choose to dial
//The tone is output and written to the display
//Input: Array holding 10 digit number
//Return: Nothing
void SpeedDial(BYTE Numbers[])
{
   char counter;

   for(counter = 0;counter < NUMBER;counter++)
   {
      //Dial the digits in the array
      Dial(Numbers[counter]);
      //Reinitialize SPI to diplay the dialed digits
      SpiInit();
      //Display the digit to the LCD display that was dialed
      DispAscii(3,counter,Numbers[counter]);
      //Pause before outputting the next dial tone
      while(pauseFlag == 0);
      pauseFlag = 0;
   } 
   //Pause to see the 10th digit that was dialed    
   while(pauseFlag == 0);
   pauseFlag = 0;
   while(pauseFlag == 0);
   pauseFlag = 0;      
   while(pauseFlag == 0);
   pauseFlag = 0;
}
//Function: DialAndDisplay
//This function serves as the regular dialer
//As the user presses a digit, the tone is output
//and displayed on the screen
//Input: nothing
//Return: nothing
void DialAndDisplay(void)
{
   BYTE DialPress;
   BYTE cntr = 0;

   //Get 10 digits/characters from the user
   while(cntr < NUMBER)
   {
      //wait for the user to press a key
      while((DialPress = KeyInput()) == 0);
      //Dial the key
      Dial(DialPress);
      //Save the key press
      param.Redial[cntr] = DialPress;
      //reinitialize the SPI to output the keypress
      SpiInit();
      //Display the key press
      DispAscii(3,cntr,DialPress);
      cntr++;
   }
   //Save the contents to the prom
   SavePrm((BYTE*)&param);
}
//Function: Dial()
//This function takes in a dial and sets the frequency of that dial
//The dac is turned and the correct frequency is output for a short amount of time
//Input: key press
//Return: nothing
void Dial(BYTE KEYS)
{
   //Sets the frequencies for any dialed number of character   
   switch (KEYS)
   {
      case '*':  param.Freq = FREQ1209; param.Freq2 = FREQ941; break;
      case '#':  param.Freq = FREQ1477; param.Freq2 = FREQ941; break;
      case '0':  param.Freq = FREQ1336; param.Freq2 = FREQ941; break;  
      case '1':  param.Freq = FREQ1209; param.Freq2 = FREQ697; break;     
      case '2':  param.Freq = FREQ1336; param.Freq2 = FREQ697; break;          
      case '3':  param.Freq = FREQ1477; param.Freq2 = FREQ697; break;          
      case '4':  param.Freq = FREQ1209; param.Freq2 = FREQ770; break;          
      case '5':  param.Freq = FREQ1336; param.Freq2 = FREQ770; break;                 
      case '6':  param.Freq = FREQ1477; param.Freq2 = FREQ770; break;     
      case '7':  param.Freq = FREQ1209; param.Freq2 = FREQ852; break;          
      case '8':  param.Freq = FREQ1336; param.Freq2 = FREQ852; break;          
      case '9':  param.Freq = FREQ1477; param.Freq2 = FREQ852; break;          
   }
   //Turn the dac on 
   param.tone_flag = TRUE;
   //Intialize the dac
   DacInit();
   //output the dual-tone for a short time
   while(param.tone_flag != FALSE);
}
//Function: CharToAscii()
//Converts Char into Ascii value that is stored into null-terminated array
//After conversion, it is displayed onto LCD
//Input: A row(1-4), A column(1-18), and a data to be converted and displayed
//Output: Returns Nothing
void DispAscii(BYTE row, BYTE col, char data)
{
   /*Declare array to hold digit and 0x00*/
   char text[2];

  /*Store the digit into char string with a null-terminator*/
   text[0] = data;
   text[1] = 0x00;

   /*Display the digit at a specific row and column*/
   DispText(row, col, text);
   
   DispUpdate();
}
//Function: InitDisp()
// Initialized SPI and Clears Display
//Input: nothing
//Output: nothing
void InitDisp(void)
{
   SpiInit();
   DispClear();
}
//Function: ReadParam()
//Writes all the data in the PROM to the structure
//Input: pointer pointing to the struct
//Output: nothing
void ReadPrm(BYTE *data)
{
   BYTE count = 0;
   
   //Read every byte in the sturct
   for(count = 0; count < sizeof(struct FinalLab); count++)
      ReadParam((BYTE*)&data[count],count);  
}
//Function: SaveParam()
//Writes all the data in the structure to the PROM
//Input: pointer pointing to the struct
//Output: nothing
void SavePrm(BYTE *data)
{
   BYTE count;

   //Write each byte of the struct into the EEprom
   //Wait to give the EEProm to write
   for(count = 0; count < sizeof(struct FinalLab); count++)
   {
      WriteParam((BYTE*)&data[count],count);
      pauseFlag = 0; while(pauseFlag == 0);
      pauseFlag = 0; while(pauseFlag == 0);
   }  
}
//Function: ReadParam()
//Reads a parameter fromt the prom
//Input:  pointer to accept data and address into prom 
//Output: value read from prom
void ReadParam(BYTE *datap, BYTE addr)
{
   EECON1 = 0;      /* disable the EEPROM read/write circuitry */
   EEADRH = 0;
   EEADR = addr;    /* set the address of the byte to be read */
   EECON1 = 1;      /* request an EEPROM data read */
   *datap = EEDATA; /* get the data */
   EECON1 = 0;
}
//Function: WriteParam()
//Writes a parameter fromt the prom
//Input:  pointer to write data and address into prom 
//Output: nothing
void WriteParam(BYTE *datap, BYTE addr)
{
   EECON1 = 0;      /* disable the EEPROM read/write circuitry */
   EEADRH = 0;
   EEADR = addr;    /* set the address of the byte to be written */
   EEDATA = *datap; /* load the data to be written */
   EECON1 = 4;      /* request an EEPROM data write */
   INTCON &= 0x7f;  /* disable interrupts */
   EECON2 = 0x55;   /* this sequence initiates an EEPROM write */
   EECON2 = 0xaa;
   EECON1 |= 2;     /* we are doing an erase/write cycle */
   INTCON |= 0x80;  /* turn interrupts back on */
   EECON1 = 0;
}