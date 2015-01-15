/* Tetris for one of Nottingham Hackspace's big clocks:
 * https://wiki.nottinghack.org.uk/wiki/BigClocks
 *
 * Also requires:
 * - BigClock library, which is in (along with snake): https://github.com/daniel1111/BigClockSnake
 * - MatrixTest lib, used to display the score       : https://github.com/daniel1111/MatrixText
 */

#include <MatrixText.h>
#include <System5x7.h>
#include <CTetris.h>
#include <BigClock.h>
#include "SPI.h"
#include "TimerOne.h"

/* the board */
#define COLS  13
#define ROWS  40

#define SCORE_GAP 10 // how much space to leave at the top of the display for the score

byte _framebuf[MAX_X][MAX_Y]; 

CTetris *_ct;
BigClock _bc;
MatrixText *_mt;

boolean _paused;

#define LCD_WIDTH   92 // Note: x-coordinates go wide
#define LCD_HEIGHT  26 // Note: y-coordinates go high


void setup()
{
  int level = 2;
  Serial.begin(19200);
  
  // Setup display
  _bc.init(); // Init BigClock/SPI, etc
  
  // Setup Tetris
  _ct = new CTetris(set_xy_tetris, (LCD_WIDTH-SCORE_GAP)/2, LCD_HEIGHT/2, level); // -SCORE_GAP to allow space for score
  
  // Setup the score text at the top of the screen
  _mt = new MatrixText(set_xy_mt);
  
  _paused = false;
}


void loop()
{
  memset(_framebuf, 0, sizeof(_framebuf));
  char s_score[10]="";
   
  if (!_paused)
  {
    if (_ct->loop())
    {    
      itoa(_ct->get_score(), s_score, 10);
      _mt->show_text(s_score, 0,  0, LCD_HEIGHT, SCORE_GAP, false); // HEIGHT becasue display is used sideways
      _mt->loop();
      _bc.output(&_framebuf[0][0]); 
    }
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
        _ct->drop();
        break;
         
      case 'd':
      case 'D':
        _ct->right();
        break;
        
      case 'a':
      case 'A':
        _ct->left();
        break;
        
      case 'p':
      case 'P':
        _paused = !_paused;
        break;
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
  set_real_xy(y, x, val);
}


void set_real_xy(byte x, byte y, byte val)
/* set pixel(x,y) on display framebuffer to be val */
{
  x += 4;
  
  byte *a=  &_framebuf[x/8][y];

  if (val)
    *a |= (1 << x%8);
  else
    *a &= ~(1 << x%8);  
 
}
