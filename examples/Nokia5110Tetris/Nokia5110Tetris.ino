
/* Tetris example for a Nokia5110 LCD module.
 * Also requires MatrixTest lib, used to display the score: 
 *   https://github.com/daniel1111/MatrixText
 */

#include <MatrixText.h>
#include <System5x7.h>
#include <CTetris.h>
#include <Nokia5110.h>
#include <SPI.h>

/* the board */
#define COLS  13
#define ROWS  40

#define SCORE_GAP 10 // how much space to leave at the top of the display for the score

CTetris *_ct;
Nokia5110 *_disp;
MatrixText *_mt;

/* from Nokia5110.h
#define LCD_WIDTH   84 // Note: x-coordinates go wide
#define LCD_HEIGHT  48 // Note: y-coordinates go high
*/

void setup()
{
  int level = 1; /* Between 1 and 9 */
  
  Serial.begin(19200);
  
  // Setup display
  _disp = new Nokia5110();
  _disp->lcdBegin();
  _disp->setContrast(50); 
  
  // Setup Tetris
  _ct = new CTetris(set_xy_tetris, (LCD_WIDTH-SCORE_GAP)/2, LCD_HEIGHT/2, level); // -SCORE_GAP to allow space for score
  
  // Setup the score text at the top of the screen
  _mt = new MatrixText(set_xy_mt);
}


void loop()
{
  char s_score[10]="";
   
  if (_ct->loop())
  {    
    itoa(_ct->get_score(), s_score, 10);
    _mt->show_text(s_score, 0,  0, LCD_HEIGHT, SCORE_GAP, false); // HEIGHT becasue display is used sideways
    _mt->loop();
    _disp->updateDisplay(); 
  }
    
   
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    
    switch (inChar)
    {
      case 'w':
      case 'W':
        _ct->rotate();
        break;
        
      case ' ':
      case 'p':
        _ct->drop();
        break;
         
      case 'a':
      case 'A':
        _ct->right();
        break;
        
      case 'D':
      case 'd':
        _ct->left();

    }
  }   
}

// Called by CTetris. 
void set_xy_tetris(byte x1, byte y1, byte val)
{
  // Rotate, so show tetris sideways on the screen
  byte x = y1;
  byte y = x1;
  
  // Show the Tetris game at double size, i.e. four pixels per block
  x *= 2;
  y *= 2;
  
  // Move Tetris game to the bottom of the screen (well, actaully far right) to 
  // allow space for the score at the top.
  x += SCORE_GAP;
 
  // Write to display buffer
  set_real_xy(x  , y,   val);
  set_real_xy(x  , y+1, val);
  set_real_xy(x+1, y+1, val);
  set_real_xy(x+1, y  , val);
}

void set_xy_mt(byte x, byte y, byte val)
{
  // Want the text sideways, so swap x<->y
  set_real_xy(y, (LCD_HEIGHT-5)-x, val);
}

void set_real_xy(byte x, byte y, byte val)
/* set pixel(x,y) on display framebuffer to be val */
{
  _disp->setPixel(x, y, val);
}

