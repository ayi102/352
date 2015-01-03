#include <p18F4525.h>
#include "dev12.h"

//Global variables
extern BYTE tenthflag;

//Function: CapSlider()
//Polls the capacitive touch pad. Takes an average of three sets of data
//Determines which pad is being touched
//Input: Nothing
//Output: Returns index of max value between 6 touch pads
char CapSlider(void)
{
   //define variable to accept a pad value
   int data1;
   char count;
   char index = 0;
   char MaxIndex = 0;
   // define array to store all 18 data points
   int Data[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   // define array to store average of 3 sets of data in Data[]
   int AvgData[6] = {0,0,0,0,0,0};
   
   //In order to accept 18 data points, the index sent to the CHS register
   // Had to be kept between 0 and 5. Also, since the bits were sent 1 at a time
   // the index value had to be AND'd and shifted to keep intact the correct mux channell
   for(count = 0; count < THREE_SETS; count++)
   {
      //Each bit had to be written separatley to the CHS register
      index = (count % SLIDERS) & 0x01;      
      ADCON0bits.CHS0 = index;         //(sets the mux to select a specific channel)
      index = (count % SLIDERS) & 0x02;
      index = index >> 1;
      ADCON0bits.CHS1 = index;        //(sets the mux to select a specific channel)
      index = (count % SLIDERS) & 0x04;
      index = index >> 2;
      ADCON0bits.CHS2 = index;        //(sets the mux to select a specific channel)
      // begin the AtoD conversion
      ADCON0bits.GO = 1;
      // wait for the AtoD to complete
      while (ADCON0bits.GO != 0);
      // read back the 10bit A-to-D result
      data1 = ADRESH;
      //Store the data into an Array
      Data[count] = data1;

      // Pause of .1 sec is taken only twice to get accurate readings
      // When the flag goes high, the flag is reset for another iteration
      if (count == 0 || count == 6)
      {
         while(tenthflag == 0);
         tenthflag = 0;             
      }
   }
   // Average the 3 sets of data
   for(count = 0; count < SLIDERS; count++)
   {
      AvgData[count] = AVERAGE(Data[count],Data[count+6],Data[count+12]);
   }
   // Display the values of the touch pads
   IntToAscii(1,0,AvgData[0]);          //S
   IntToAscii(1,5,AvgData[1]);          //L
   IntToAscii(1,10,AvgData[2]);         //I
   IntToAscii(2,0,AvgData[3]);          //D
   IntToAscii(2,5,AvgData[4]);          //E
   IntToAscii(2,10,AvgData[5]);         //R
   // Indetify the Index of the largest average touch pad value
   MaxIndex = FindMax(AvgData);
   // If the cap value does not reach a certain value, it will not be recognize
   // as being touched
   if(AvgData[MaxIndex] < THRESHOLD)
   {
      //Return -1 if the cap values are not large enough
      MaxIndex = -1;
   }
   return MaxIndex;
}
//Function: PhotoCell()
//Polls the value of the PhotoCell
//Determines and outputs light intensity and Lights LED if too dark
//Input: Nothing
//Output: Display Light Intensity/Lights LED
void PhotoCell(void)
{
   // variable to contain light intensity value
   int num;

   // Set Photocell output low  
   PHOTOCELL_OUT = 0;

   // Poll photo cell every .1 seconds 
   while(tenthflag == 0);
   tenthflag = 0;
   // Set Photocell output high
   PHOTOCELL_OUT = 1;

   //Read light intenisity through the PhotoCell
   for (num=0; PHOTOCELL_IN != 0; num++);
   //Display the Photocell intensity value
   IntToAscii(0,5,num);

   // The darker, the larger the value
   // If the intensity value reaches over 600 turn the LED ON
   if(num > TOODARK)
      LED3_PIN = LED_ON;
   else
      LED3_PIN = LED_OFF;
   
}
//Function: InitToAscii()
//Converts 4 digit integer into Ascii value that is stored into null-terminated array
//After conversion, it is displayed onto LCD
//Input: A row(1-4), A column(1-18), and a data to be converted and displayed
//Output: Returns Nothing
void IntToAscii(BYTE row, BYTE col, int data)
{
   /*Declare array to hold digit and 0x00*/
   char text[5];

   /*Convert integer digit to ASCII digit*/
   /*start with the ms digit, convert to ASCII, store in array and cut*/
   text[0] = (data/1000) + 48;
   data %= 1000;
   text[1] = (data/100) + 48;
   data %= 100;
   text[2] = (data/10) + 48;
   text[3] = (data % 10) + 48;

  /*Store the digit into char string with a null-terminator*/
   text[4] = 0x00;

   /*Display the digit at a specific row and column*/
   DispText(row, col, text);
}
//Function: FindMax()
//Finds the largest value in an array of size 6
//Input: an integer Array
//Output: Returns index of largest value
char FindMax(int Array[])
{
   char count;
   //set max value to first element in array
   int max = Array[0];        
   char index = 0;

   for(count = 1; count < 6; count++)
   {
      // if array value is larger than max
      // set new max and identify the index
      if(Array[count] > max)
      {
         max = Array[count];
         index = count;
      }
   }
   
   return index;
}
//Function: DispNumber()
//Displays a char value onto LCD Display
//Input: 5 chars to be displayed
//Output: Returns Nothing
void DispNumber(char num1, char num2, char num3, char num4, char num5)
{
   // Place holders for timer
   static CONST char mesg04[] = ":";
   static CONST char mesg05[] = ".";

   
   /*Displaying the digits onto LCD*/
   CharToAscii(3,12,num1);
   DispConstText(3,11,mesg05);
   CharToAscii(3,10,num2);
   CharToAscii(3,9,num3);
   DispConstText(3,8,mesg04);
   CharToAscii(3,7,num4);
   CharToAscii(3,6,num5);
}
//Function: CharToAscii()
//Converts Char into Ascii value that is stored into null-terminated array
//After conversion, it is displayed onto LCD
//Input: A row(1-4), A column(1-18), and a data to be converted and displayed
//Output: Returns Nothing
void CharToAscii(BYTE row, BYTE col, char data)
{
   /*Declare array to hold digit and 0x00*/
   char text[2];

   /*Convert char digit to ASCII digit*/
   ASCII(data);

  /*Store the digit into char string with a null-terminator*/
   text[0] = data;
   text[1] = 0x00;

   /*Display the digit at a specific row and column*/
   DispText(row, col, text);

}
//Function: TransData
//Converts Char into Ascii value and then transmits the data via RS-232
//Input: 5 chars digits to be sent through RS-232
//Output: Returns Nothing
void TransData(char time1, char time2, char time3, char time4, char time5)
{
   // Transmit ASCII tens of minutes
   ASCII(time5);
   Trans(time5);

   // Transmit ASCII minutes
   ASCII(time4);
   Trans(time4);
   
   /*Transmit a colon for the minutes place holder*/
   Trans(':');

   //Transmit ASCII tens of seconds
   ASCII(time3);
   Trans(time3);
   
   //Transmit ASCII seconds
   ASCII(time2);
   Trans(time2);

   //Transmit period for the tenths place holder
   Trans('.');

   //Transmit tenths of seconds
   ASCII(time1);
   Trans(time1);

   //Tells receiver of data to write over the data instead of writing data continuously
   Trans('\r');

}
//Function: Trans
//Transmits data via RS-232
//Input: a char to be sent through RS-232
//Output: Returns Nothing
void Trans(char TxT)
{
   // wait for the transmit shift register to be empty
   // then write out the byte to be sent
   while (TXSTAbits.TRMT == 0);
   TXREG = TxT;
}
//Function: Receive
//Receives data via RS-232
//Input: nothing
//Output: Returns Rxt- Data received via RS-232
BYTE Receive(void)
{
   // Declare BYTE to hold contain received data
   BYTE RxT;
   
   // set RxT to data received
   RxT = RCREG;
   
  // return data received
   return RxT;
}
void KeyPadPoll(void)
{

   BYTE last=0;
   BYTE key;

   /* wait for a key press */
   while ((key = KeyInput()) == 0);

   while(1)
   {
      /*Check if a key has been pressed twice, if yes output a message saying they have*/
      /*Otherwise, acknowledge they have chosen a new key*/
      if (last == key)
         continue;
         
      switch (key)
      {
       /*Insert Data to to display*/
      }
       DispUpdate();
       last = key;
   }
   
}