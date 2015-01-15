
/* Arduino Tetris library, particularly suitable for low-resolution matrix displays. 
 * Adapted from Tetris in the bsdgames package, see licence below.
 * - Daniel Swann 15/01/2015
 */


/*-
 * Copyright (c) 1992, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek and Darren F. Provine.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)tetris.c  8.1 (Berkeley) 5/31/93
 */

#include <stdlib.h>
#include <string.h>

#include "Arduino.h" // for micros()
#include "CTetris.h"

const struct shape *curshape;
const struct shape *nextshape;
struct shape shapes[19];


CTetris::CTetris(set_xy_fuct set_xy, int rows, int cols, int level)
{
  _set_xy = set_xy;
  
  __b_cols = cols;
  __b_rows = rows;
  _board = (cell*)malloc(__b_cols*__b_rows);  
  _a_first = 1;
  _a_last = rows-2;
  
  init_shapes();
  setup_board();

  srandom(2); /* FIXME */
  
  _pos = _a_first*__b_cols + (__b_cols/2)-1;
  _nextshape = randshape();
  _curshape = randshape();  
  _score = 0;
  _game_over = false;
  _last_update = 0;
  _level = level;
  
  
  _fallrate = 500000 / _level;
}

CTetris::~CTetris()
{
  if (_board != NULL)
  {
    free(_board);
    _board = NULL;
  }
}

bool CTetris::loop()
{
  bool timeout_reached = false;
  bool redraw_screen = false;
  
  // Assume loop() is called more often than required, i.e. don't advance game state on each call.
  if ((micros() - _last_update) > _fallrate)
  {
    timeout_reached = true;
    _last_update = micros();
  }

  if (!timeout_reached)
    return false;

  if (_game_over)
    return false;  
  
  faster();
  place(_curshape, _pos, 1);
  update_display();
  place(_curshape, _pos, 0);

     
  if (fits_in(_curshape, _pos + __b_cols)) 
  {
    _pos += __b_cols;
    return true;
  }

  /*
   * Put up the current shape `permanently',
   * bump score, and elide any full rows.
   */
  place(_curshape, _pos, 1);
  _score++;
  elide();

  /*
   * Choose a new shape.  If it does not fit,
   * the game is over.
   */
  _curshape = _nextshape;
  _nextshape = randshape();
  _pos = _a_first*__b_cols + (__b_cols/2)-1;
  if (!fits_in(_curshape, _pos))
    _game_over = true;   
  
  return true;
}

void CTetris::left()
{
  if (fits_in(_curshape, _pos - 1))
   _pos--;  
}

void CTetris::right()
{
  if (fits_in(_curshape, _pos + 1))
   _pos++;  
}

void CTetris::rotate()
{
  /* turn */
  const struct shape *newshape = &shapes[_curshape->rot];

  if (fits_in(newshape, _pos))
    _curshape = newshape;  
}

void CTetris::drop()
{
  /* move to bottom */
  while (fits_in(_curshape, _pos + __b_cols)) 
  {
   _pos += __b_cols;
   _score++;  
  }
}

void CTetris::update_display()
{
  for (int y=0; y < __b_rows; y++)
    for (int x=0; x < __b_cols; x++)
      _set_xy(x, y, _board[(__b_cols*y)+x] ? 1 : 0);   
}

/*
 * Set up the initial board.  The bottom display row is completely set,
 * along with another (hidden) row underneath that.  Also, the left and
 * right edges are set.
 */
void CTetris::setup_board()
{
  int i;
  cell *p;

  p = _board;
  for (i = (__b_cols * __b_rows); i; i--)
    *p++ = i <= (2 * __b_cols) || (i % __b_cols) < 2;
}


/*
 * Elide any full active rows.
 */
void CTetris::elide()
{
  int i, j, base;
  cell *p;

  for (i = _a_first; i < _a_last; i++) {
    base = i * __b_cols + 1;
    p = _board+base;
    for (j = __b_cols - 2; *p++ != 0;) {
      if (--j <= 0) {
        /* this row is to be elided */
        memset(_board+base, 0, __b_cols - 2);
        update_display();
 //       tsleep(); FIXME
        while (--base != 0)
          _board[base + __b_cols] = _board[base];
        update_display(); 
 //       tsleep(); FIXME
        break;
      }
    }
  }
}

void CTetris::init_shapes()
{
  int tl = (-1*__b_cols) - 1; /* top left       */
  int tc = (-1*__b_cols);     /* top center     */
  int tr = (-1*__b_cols) + 1; /* top right      */
  int ml = -1;                /* middle left    */
  int mr = 1;                 /* middle right   */
  int bl = (__b_cols) - 1;    /* bottom left    */
  int bc = (__b_cols);        /* bottom center  */
  int br = (__b_cols) + 1;    /* bottom right   */
  
  shapes[ 0].rot =  7; shapes[ 0].off[0] = tl;  shapes[ 0].off[1] = tc;  shapes[ 0].off[2] = mr; 
  shapes[ 1].rot =  8; shapes[ 1].off[0] = tc;  shapes[ 1].off[1] = tr;  shapes[ 1].off[2] = ml; 
  shapes[ 2].rot =  9; shapes[ 2].off[0] = ml;  shapes[ 2].off[1] = mr;  shapes[ 2].off[2] = bc; 
  shapes[ 3].rot =  3; shapes[ 3].off[0] = tl;  shapes[ 3].off[1] = tc;  shapes[ 3].off[2] = ml; 
  shapes[ 4].rot = 12; shapes[ 4].off[0] = ml;  shapes[ 4].off[1] = bl;  shapes[ 4].off[2] = mr; 
  shapes[ 5].rot = 15; shapes[ 5].off[0] = ml;  shapes[ 5].off[1] = br;  shapes[ 5].off[2] = mr; 
  shapes[ 6].rot = 18; shapes[ 6].off[0] = ml;  shapes[ 6].off[1] = mr;  shapes[ 6].off[2] = 2 ;   /* sticks out */
  shapes[ 7].rot =  0; shapes[ 7].off[0] = tc;  shapes[ 7].off[1] = ml;  shapes[ 7].off[2] = bl; 
  shapes[ 8].rot =  1; shapes[ 8].off[0] = tc;  shapes[ 8].off[1] = mr;  shapes[ 8].off[2] = br; 
  shapes[ 9].rot = 10; shapes[ 9].off[0] = tc;  shapes[ 9].off[1] = mr;  shapes[ 9].off[2] = bc; 
  shapes[10].rot = 11; shapes[10].off[0] = tc;  shapes[10].off[1] = ml;  shapes[10].off[2] = mr; 
  shapes[11].rot =  2; shapes[11].off[0] = tc;  shapes[11].off[1] = ml;  shapes[11].off[2] = bc; 
  shapes[12].rot = 13; shapes[12].off[0] = tc;  shapes[12].off[1] = bc;  shapes[12].off[2] = br; 
  shapes[13].rot = 14; shapes[13].off[0] = tr;  shapes[13].off[1] = ml;  shapes[13].off[2] = mr; 
  shapes[14].rot =  4; shapes[14].off[0] = tl;  shapes[14].off[1] = tc;  shapes[14].off[2] = bc; 
  shapes[15].rot = 16; shapes[15].off[0] = tr;  shapes[15].off[1] = tc;  shapes[15].off[2] = bc; 
  shapes[16].rot = 17; shapes[16].off[0] = tl;  shapes[16].off[1] = mr;  shapes[16].off[2] = ml; 
  shapes[17].rot =  5; shapes[17].off[0] = tc;  shapes[17].off[1] = bc;  shapes[17].off[2] = bl; 
  shapes[18].rot =  6; shapes[18].off[0] = tc;  shapes[18].off[1] = bc;  shapes[18].off[2] = 2*__b_cols;  /* sticks out */  
}

/*
 * Return true iff the given shape fits in the given position,
 * taking the current board into account.
 */
int CTetris::fits_in(const struct shape *shape, int pos)
{
  const int *o = shape->off;

  if (_board[pos] || _board[pos + *o++] || _board[pos + *o++] ||
      _board[pos + *o])
    return 0;
  return 1;
}

/*
 * Write the given shape into the current board, turning it on
 * if `onoff' is 1, and off if `onoff' is 0.
 */
void CTetris::place(const struct shape *shape, int pos, int onoff)
{
  const int *o = shape->off;

  _board[pos] = onoff;
  _board[pos + *o++] = onoff;
  _board[pos + *o++] = onoff;
  _board[pos + *o] = onoff;
}

int CTetris::get_score()
{
  return _score;
}

void CTetris::faster()
{
  _fallrate -= _fallrate / 3000;
}
