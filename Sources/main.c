#include <hidef.h>           /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#define LCD_DATA PORTK
#define LCD_CTRL PORTK
#define RS 0x01
#define EN 0x02

void COMWRT4(unsigned char);
void DATWRT4(unsigned char);
void LCD(void);
void MSDelay(unsigned int);
void compare(unsigned char *word11, unsigned char *word22);
void Win (void);
void Lose(void);


unsigned char LETTER;
unsigned int flag;


/*Game logic variables*/
unsigned char word2[5];
unsigned int x,y;
unsigned int letterCount;
unsigned int correctCount;
unsigned int guesses, temp;
unsigned char mode;
unsigned char state;
unsigned char gword[5];



void main(void) 
{
  DDRB = 0xFF;      //PTB as output for LEDs
 
/*Serial Configuration*/
  SCI0BDH=0x00;    //Serial Monitor used for LOAD works at 48MHz  
  SCI0BDL=26;      //8MHz/2=4MHz, 4MHz/16=250,000 and 250,000/9600=26 for run when SW=10 
  SCI0CR1=0x00;
  SCI0CR2=0x24;    //enable receive interrupt too
                                                                                                                                                                                                                      
/*LCD Configuration*/
  DDRK = 0xFF;   
  COMWRT4(0x33);   //reset sequence provided by data sheet
  MSDelay(1);
  COMWRT4(0x32);   //reset sequence provided by data sheet
  MSDelay(1);
  COMWRT4(0x28);   //Function set to four bit data length
                                   //2 line, 5 x 7 dot format
  MSDelay(1);
  COMWRT4(0x06);  //entry mode set, increment, no shift
  MSDelay(1);
  COMWRT4(0x0E);  //Display set, disp on, cursor on, blink off
  MSDelay(1);
  COMWRT4(0x01);  //Clear display
  MSDelay(1);
  COMWRT4(0x80);  //set start posistion, home position  
  
  state = 0;
  letterCount = 0;
  guesses = 48;    //decimal 48 give ascii for 0
 


/*Initial propmpt on LCD*/
  
  COMWRT4(0x01);   
  COMWRT4(0xC0);           
  MSDelay(1);               
  DATWRT4('E');             
  MSDelay(1);
  DATWRT4('n');
  MSDelay(1);
  DATWRT4('t');
  MSDelay(1);
  DATWRT4('e');
  MSDelay(1);
  DATWRT4('r');
  MSDelay(1);
  DATWRT4(' ');
  MSDelay(1);
  DATWRT4('A');
  MSDelay(1); 
  DATWRT4('n'); 
  MSDelay(1);
  DATWRT4('s');              
  MSDelay(1);
  DATWRT4('w');
  MSDelay(1);
  DATWRT4('e');
  MSDelay(1);
  DATWRT4('r');
  MSDelay(1);
  DATWRT4(':');
  MSDelay(1);
  DATWRT4(' ');
  MSDelay(1);
  COMWRT4(0x80);
  __asm CLI;  //Enable interrupts globally 



  
  for(;;) 
  {
  
  //////////////Player 1 Provide Answer/////////////////////////  
    while (state == 0){
       if(LETTER == 0x7F){         //if keystroke == backspace
          if(letterCount > 0){           //clear previous letter
             COMWRT4(0x10);
             MSDelay(1);
             DATWRT4(' ');
             MSDelay(1);
             COMWRT4(0x10);
             
             gword[letterCount] = ' ';   //delete previous letter from array (array length is alawys 5)
             flag = 0;                   //flag to sync LCD & Interrupt
             letterCount--;              //keep track of letters entered (can't use array length)
          }
        } 
        else if(LETTER == 0x0D){         //if keystroke == ENTER                                  
           if(letterCount == 5){          //proceed only if we have a full word (5 letters)
              flag = 0;
              state = 1;
              letterCount = 0;
              PORTB = 0;                      //clear LEDS
              COMWRT4(0x01);             
              COMWRT4(0xC0);            
              MSDelay(1);               
              DATWRT4('E');              
              MSDelay(1);
              DATWRT4('n');
              MSDelay(1);
              DATWRT4('t');
              MSDelay(1);
              DATWRT4('e');
              MSDelay(1);
              DATWRT4('r');
              MSDelay(1);
              DATWRT4(' ');
              MSDelay(1);
              DATWRT4('g');
              MSDelay(1); 
              DATWRT4('u'); 
              MSDelay(1);
              DATWRT4('e');              //Display Most recent guess on second line, next to #guess
              MSDelay(1);
              DATWRT4('s');
              MSDelay(1);
              DATWRT4('s');
              MSDelay(1);
              DATWRT4(' ');
              MSDelay(1);
              DATWRT4('#');
              MSDelay(1);
              DATWRT4('1');
              MSDelay(1);
              COMWRT4(0x80); 
           }
        }

      else{
       if(letterCount < 5){
          if((LETTER <= 0x5a && LETTER >= 0x41) || ((LETTER <= 0x7a && LETTER >= 0x61))){               //if keystroke == any other key
            DATWRT4(LETTER);               //display letter
            gword[letterCount] = LETTER;   //add letter to the array in correct place              
            flag = 0;
            letterCount++;                 //increment letter count
          }
         }
      }
    
      while(flag == 0);                  //check lcd & interrupt sync flag
    }
    
    
    
///////////////////Player 2 Provides Guesses////////////////////////////
    while (state == 1){
        if(LETTER == 0x7F){         //if keystrok == backspace
          if(letterCount > 0){           //clear previous letter
             COMWRT4(0x10);
             MSDelay(1);
             DATWRT4(' ');
             MSDelay(1);
             COMWRT4(0x10);
             
             word2[letterCount] = ' ';   //delete previous letter from array (array length is alawys 5)
             flag = 0;                   //flag to sync LCD & Interrupt
             letterCount--;              //keep track of letters entered (can't use array length)
          }
        } 
        else if(LETTER == 0x0D){         //if keystroke == ENTER                                  
           if(letterCount == 5){           //proceed only if we have a full word (5 letters) 
              flag = 0;
              guesses++;
              PORTB = 0;                 //clear LEDS
              correctCount = 0;
              compare(gword, word2);       //call compare or end game function depending on guesses count
              if (correctCount == 5){
                Win();
              }
              else if ((guesses == 53)&&(correctCount<5)){
                Lose();
              }
              letterCount=0;
              COMWRT4(0xC0);            //move cursor to beginning of 2nd line
              MSDelay(1);               
              DATWRT4('#');              //Display # guess on second line
              MSDelay(1);
              DATWRT4('G');
              MSDelay(1);
              DATWRT4('u');
              MSDelay(1);
              DATWRT4('e');
              MSDelay(1);
              DATWRT4('s');
              MSDelay(1);
              DATWRT4('s');
              MSDelay(1);
              DATWRT4('=');
              MSDelay(1); 
              DATWRT4(guesses); 
              MSDelay(1);
              DATWRT4(' ');              //Display Most recent guess on second line, next to #guess
              MSDelay(1);
              DATWRT4(word2[0]);
              MSDelay(1);
              DATWRT4(word2[1]);
              MSDelay(1);
              DATWRT4(word2[2]);
              MSDelay(1);
              DATWRT4(word2[3]);
              MSDelay(1);
              DATWRT4(word2[4]);
              MSDelay(1);
              COMWRT4(0x80); 
           }
          }  
          else{
           if(letterCount < 5){
              if((LETTER <= 0x5a && LETTER >= 0x41) || ((LETTER <= 0x7a && LETTER >= 0x61))){  //if keystroke == any other key
                DATWRT4(LETTER);               //display letter
                word2[letterCount] = LETTER;   //add letter to the array in correct place              
                flag = 0;
                letterCount++;                 //increment letter count
              }
           }
         }
              
          while(flag == 0);                  
        }//close state 1
       //while (flag == 0);
     
    }   // close infinite for loop
    
}//close main
 





/*Helper function*/  


interrupt (((0x10000-Vsci0)/2)-1) void SCI0_ISR(void)
  {
   if(SCI0SR1 & SCI0SR1_RDRF_MASK) {
       LETTER = SCI0DRL;
       flag = 1;  
   }
  } 
  
 void MSDelay(unsigned int itime)
  {
    unsigned int i; unsigned int j;
    for(i=0;i<itime;i++)
      for(j=0;j<667;j++);
  }

void LCD(void) 
{
        DDRK = 0xFF;   
        COMWRT4(0x33);   //reset sequence provided by data sheet
        MSDelay(1);
        COMWRT4(0x32);   //reset sequence provided by data sheet
        MSDelay(1);
        COMWRT4(0x28);   //Function set to four bit data length
                                         //2 line, 5 x 7 dot format
        MSDelay(1);
        COMWRT4(0x06);  //entry mode set, increment, no shift
        MSDelay(1);
        COMWRT4(0x0E);  //Display set, disp on, cursor on, blink off
        MSDelay(1);
        COMWRT4(0x01);  //Clear display
        MSDelay(1);
        COMWRT4(0x80);  //set start posistion, home position
        MSDelay(1);
        DATWRT4(SCI0DRL);
}
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  
void COMWRT4(unsigned char command)
  {
        unsigned char x;
        
        x = (command & 0xF0) >> 2;         //shift high nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;          //clear bits Pk5-Pk2
        LCD_DATA = LCD_DATA | x;          //sends high nibble to PORTK
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~RS;         //set RS to command (RS=0)
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //Drop enable to capture command
        MSDelay(15);                       //wait
        x = (command & 0x0F)<< 2;          // shift low nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;         //clear bits Pk5-Pk2
        LCD_DATA =LCD_DATA | x;             //send low nibble to PORTK
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //drop enable to capture command
        MSDelay(15);
  }  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

 void DATWRT4(unsigned char data)
  {
  unsigned char x;
       
        
        
        x = (data & 0xF0) >> 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | RS;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(5);
       
        x = (data & 0x0F)<< 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(15);
  }

void compare(unsigned char *word11, unsigned char *word22){
     unsigned int i,j;
  
     COMWRT4(0x80);                 //cursor to start of first line
     for(i=0;i<5;i++){
        unsigned char k = ' ';           
        for(j=0;j<5;j++){
        /*Correct letter, wrong spot*/ 
           if(word11[i] == word22[j]){   // is letter from guess word == letter from answer?
              if(j==0){                  
                PORTB_BIT7 = 1;
              } 
              
              else if(j==1){
                PORTB_BIT6 = 1;
              } 
              
              else if(j==2){
                PORTB_BIT5 = 1;
              } 
              
              else if(j==3){
                PORTB_BIT4 = 1;
              } 
              
              else if(j==4){
              
                PORTB_BIT3 = 1;
              } 
             /*Correct letter, Correct spot*/ 
             if(i == j){            // are indecies equal?
                k = word11[i];
                correctCount++;     //keep track of correct letters in the guess word
                if(j==0){
                PORTB_BIT7 = 0;
                } 
                
                else if(j==1){
                  PORTB_BIT6 = 0;
                } 
                
                else if(j==2){
                  PORTB_BIT5 = 0;
                } 
                
                else if(j==3){
                  PORTB_BIT4 = 0;
                } 
                
                else if(j==4){
                
                  PORTB_BIT3 = 0;
                } 
             }
           }
        }
     
      DATWRT4(k);
     } 
}

void Lose(void){
     
  COMWRT4(0x01);             
  COMWRT4(0xC0);            
  MSDelay(1);               
  DATWRT4('L');             
  MSDelay(1);
  DATWRT4('O');
  MSDelay(1);
  DATWRT4('S');
  MSDelay(1);
  DATWRT4('E');
  MSDelay(1);
  DATWRT4('R');
  MSDelay(1);
  DATWRT4('!');
  MSDelay(1);
  DATWRT4('!');
  MSDelay(1); 
  DATWRT4('!'); 
  MSDelay(1);
 
  COMWRT4(0x80); 
  DATWRT4('A');             
  MSDelay(1);
  DATWRT4('n');
  MSDelay(1);
  DATWRT4('s');
  MSDelay(1);
  DATWRT4('w');
  MSDelay(1);
  DATWRT4('e');
  MSDelay(1);
  DATWRT4('r');
  MSDelay(1);
  DATWRT4(':');
  MSDelay(1); 
  DATWRT4(' '); 
  MSDelay(1);
  DATWRT4(gword[0]);             
  MSDelay(1);
  DATWRT4(gword[1]);
  MSDelay(1);
  DATWRT4(gword[2]);
  MSDelay(1);
  DATWRT4(gword[3]);
  MSDelay(1);
  DATWRT4(gword[4]);
  MSDelay(1);
  for(;;);
}


void Win(void){
  COMWRT4(0x01);         
  COMWRT4(0xC0);          
  MSDelay(1);               
  DATWRT4('Y');              
  MSDelay(1);
  DATWRT4('o');
  MSDelay(1);
  DATWRT4('u');
  MSDelay(1);
  DATWRT4(' ');
  MSDelay(1);
  DATWRT4('W');
  MSDelay(1);
  DATWRT4('i');
  MSDelay(1);
  DATWRT4('n');
  MSDelay(1); 
  DATWRT4('!'); 
  MSDelay(1);
  COMWRT4(0x80);
  for(;;);
}

             
