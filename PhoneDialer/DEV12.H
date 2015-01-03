#define BYTE  unsigned char
#define WORD  unsigned int
#define CONST far rom

#define  FREQ1477   124
#define  FREQ1336   113
#define  FREQ1209   102
#define  FREQ941    81
#define  FREQ852    74
#define  FREQ770    65 
#define  FREQ697    58
#define  NUMBER     10
#define  AMPLITUDE  140
#define  BOUNDRY1   0x32
#define  BOUNDRY2   0x23
#define  BOUNDRY3   0x2A
#define  BOUNDRY4   0x34
#define  BOUNDRY5   0x30

/* init routines */
void Init(void);
void PicInit(void);
void SpiInit(void);
void DacInit(void);
void AtoDInit(void);

//Function Prototypes

//High Level Functions
void ChoiceDialMode(BYTE KEYS);
void DialMode(void);

//DialFunction
void Dial(BYTE KEYS);

//Prompt/Display Functions
void PromptSaveOrDial(void);
void PromptForSave(void);
BYTE DispDialMode(void);
void DispAscii(BYTE row, BYTE col, char data);
void DialAndDisplay(void);

//EEPRom Functions
void WriteParam(BYTE *datap, BYTE addr);
void ReadParam(BYTE *datap, BYTE addr);
void SavePrm(BYTE *data);
void ReadPrm(BYTE *data);

//Number Save Functions
void SaveDial(BYTE data[]);
void SpeedDial(BYTE Numbers[]);

//AtoD Functions
void AnalyzeSound(void);
BYTE ReadSensor(void);
BYTE SelectSensor(BYTE parm);

/* display functions */
void InitDisp(void);
void DispInit(void);
void DispClear(void);
void DispConstText(BYTE line, BYTE col, CONST char *textp);
void DispText(BYTE line, BYTE col, char *textp);
void DispUpdate(void);
BYTE KeyInput(void);
void MemCopy(BYTE *,BYTE *,int);
void MemClear(BYTE *,int);
void Debounce(BYTE BtnStatus1,BYTE BtnStatus2,BYTE BtnStatus3,BYTE BtnStatus4);
void Debounce2(BYTE BtnStatus1,BYTE BtnStatus2,BYTE BtnStatus3,BYTE BtnStatus4);
void Debounce3(BYTE BtnStatus1,BYTE BtnStatus2,BYTE BtnStatus3,BYTE BtnStatus4);

struct FinalLab
{
   BYTE FirstNum[10];
   BYTE SecondNum[10];
   BYTE Redial[10];
   BYTE Freq;
   BYTE Freq2;
   BYTE tone_flag;
   BYTE QuitFlag;
};

#define FALSE     0
#define TRUE      1

/* data direction flags */
#define OUTPUT    0
#define INPUT     1

//----Defines for the operation of the LCD -----
#define LCD_CMND  0
#define LCD_DATA  1


//----------- PORT A ------------------
// A0 = slider-S (input-an0)
// A1 = slider-L (input-an1)
// A2 = slider-I (input-an2)
// A3 = slider-D (input-an3)
// A4 = unconnected
// A5 = slider-E (input-an4)
// A6 = photocell charge (output)
// A7 = unconnected

#define SLIDER_S                0
#define SLIDER_L                1
#define SLIDER_I                2
#define SLIDER_D                3
#define SLIDER_E                4

//----------- PORT B ------------------
// B0 = keypad row2 (input)
// B1 = keypad row3 (input)
// B2 = keypad row4 (input)
// B3 = infra red detector
// B4 = unconnected
// B5 = program-m
// B6 = program-c
// B7 = program-d

#define TRIS_ROW2               DDRBbits.RB0
#define TRIS_ROW3               DDRBbits.RB1
#define TRIS_ROW4               DDRBbits.RB2

#define ROW2_PIN                PORTBbits.RB0
#define ROW3_PIN                PORTBbits.RB1
#define ROW4_PIN                PORTBbits.RB2

//----------- PORT C ------------------
// C0 = photocell (input)
// C1 = green led3 (output)
// C2 = ir led4 (output)
// C3 = serial clock (output-spi)
// C4 = serial data (input/output-i2c) dac,eeprom
// C5 = serial data (output-spi) display
// C6 = txd (output)
// C7 = rxd (input)

#define TRIS_LED3               DDRCbits.RC1
#define TRIS_LED4               DDRCbits.RC2
#define TRIS_SCL                DDRCbits.RC3
#define TRIS_SDA                DDRCbits.RC4
#define TRIS_SDO                DDRCbits.RC5

#define LED3_PIN                PORTCbits.RC1
#define LED4_PIN                PORTCbits.RC2
#define SCL_PIN                 PORTCbits.RC3
#define SDA_PIN                 PORTCbits.RC4
#define SDO_PIN                 PORTCbits.RC5

//----------- PORT D ------------------
// D0 = display (output-cs1b)
// D1 = display (output rst)
// D2 = display (output a0)
// D3 = unconnected
// D4 = keypad col1 (output)
// D5 = keypad col2 (output)
// D6 = keypad col3 (output)
// D7 = keypad row1 (input)

#define TRIS_CS1B               DDRDbits.RD0
#define TRIS_RST                DDRDbits.RD1
#define TRIS_A0                 DDRDbits.RD2
#define TRIS_COL1               DDRDbits.RD4
#define TRIS_COL2               DDRDbits.RD5
#define TRIS_COL3               DDRDbits.RD6
#define TRIS_ROW1               DDRDbits.RD7

#define CS1B_PIN                PORTDbits.RD0
#define RST_PIN                 PORTDbits.RD1
#define A0_PIN                  PORTDbits.RD2
#define COL1_PIN                PORTDbits.RD4
#define COL2_PIN                PORTDbits.RD5
#define COL3_PIN                PORTDbits.RD6
#define ROW1_PIN                PORTDbits.RD7

//----------- PORT E ------------------
// E0 = slider-R (input-an5)
// E1 = microphone (input-an6)
// E2 = unconnected (an7)
// E3 = mclr (input)

#define SLIDER_R                5
#define MIC_PIN                 6

