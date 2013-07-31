/*
 * Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Descriptions:
 * -------------
 * AROMA UI: IME
 *
 */
#include "../aroma.h"

/***************************[ IME ]**************************/
#define ACIME2_BTNCNT  39
static char acime2_charlist[4][26] = {
  "1234567890#\%^&~`*{}[]<>()/",
  "1234567890!@$:;'\"*\\-_=+|?/",
  "QWERTYUIOPASDFGHJKLZXCVBNM",
  "qwertyuiopasdfghjklzxcvbnm"
};
typedef struct {
  CANVAS    control;
  CANVAS    control_rest;
  CANVAS    control_push;
  
  byte      inputMsg;
  int       btnH;
  byte      onShift;
  byte      on123;
  byte      onCTRL;
  
  int       keyX[ACIME2_BTNCNT]; //-- X Position
  int       keyW[ACIME2_BTNCNT]; //-- Width
  byte      keyD[ACIME2_BTNCNT]; //-- Drawed
  byte      pushedId;
  long      loopTick;
  
} ACIMED, * ACIMEDP;

void acime2_sendmsg(ACONTROLP ctl, byte a2, byte a3, byte a4) {
  ACIMEDP  d      = (ACIMEDP) ctl->d;
  dword msg = aw_msg(d->inputMsg, a2, (d->onCTRL ? 1 : 0), a4);
  
  if (d->onCTRL) {
    d->keyD[34] = 0;
    d->onCTRL = 0;
    aw_draw(ctl->win);
  }
  
  atouch_send_message(msg);
}
static void * acime2_loopthread(void * cookie) {
  ACONTROLP ctl   = (ACONTROLP) cookie;
  ACIMEDP  d      = (ACIMEDP) ctl->d;
  long lt         = d->loopTick;
  byte pd         = d->pushedId;
  byte ft         = 0;
  
  while (1) {
    usleep(40000);
    
    if ((lt == d->loopTick) && (pd == d->pushedId)) {
      if (ft > 15) {
        if (pd == 30) {
          //-- SPACE
          acime2_sendmsg(ctl, 32, 0, 0);
        }
        else if (pd == 27) {
          //-- Backspace
          acime2_sendmsg(ctl, 8, 0, 0);
        }
        else if (pd == 35) {
          acime2_sendmsg(ctl, 37, 1, 1);  /* left */
        }
        else if (pd == 36) {
          acime2_sendmsg(ctl, 38, 1, 1);  /* up */
        }
        else if (pd == 37) {
          acime2_sendmsg(ctl, 40, 1, 1);  /* down */
        }
        else if (pd == 38) {
          acime2_sendmsg(ctl, 39, 1, 1);  /* right */
        }
        else if (!d->on123) {
          byte keyID = d->pushedId;
          char c = 0;
          
          if ((pd < 27) && (pd != 19)) {
            d->pushedId    = 254;
            int n = pd;
            
            if (n > 19) {
              n--;
            }
            
            int np = (d->onShift) ? 0 : 1;
            c = acime2_charlist[np][n];
            d->keyD[keyID] = 0;
            vibrate(30);
            acime2_sendmsg(ctl, (byte) c, 0, 0);
            
            if (d->onShift == 1) {
              d->onShift = 0;
              int i;
              
              for (i = 0; i < ACIME2_BTNCNT; i++) {
                d->keyD[i] = 0;
              }
            }
            
            ctl->ondraw(ctl);
            aw_draw(ctl->win);
          }
          
          break;
        }
      }
      
      if (ft <= 15) {
        ft++;
      }
    }
    else {
      break;
    }
  }
  
  return NULL;
}
void acime2_regThread(ACONTROLP ctl) {
  ACIMEDP  d      = (ACIMEDP) ctl->d;
  d->loopTick = alib_tick();
  pthread_t loopThread;
  pthread_create(&loopThread, NULL, acime2_loopthread, (void *) ctl);
  pthread_detach(loopThread);
}

void  acime2_action(ACONTROLP ctl, int keyID, byte isUp) {
  ACIMEDP  d      = (ACIMEDP) ctl->d;
  byte rb         = 0;
  char c          = 0;
  byte doThread   = 0;
  byte iscursor   = 0;
  
  if ((keyID < 27) && (keyID != 19)) {
    if (isUp) {
      int n = keyID;
      
      if (n > 19) {
        n--;
      }
      
      int np = 3;
      
      if (d->on123) {
        np = (d->onShift) ? 0 : 1;
      }
      else if (d->onShift) {
        np = 2;
      }
      
      c = acime2_charlist[np][n];
    }
  }
  else if (keyID == 19) {
    if (!isUp) {
      rb = 1;
      
      if (d->onShift == 1) {
        d->onShift = 2;
        rb = 0;
        d->keyD[19] = 0;
      }
      else if (d->onShift == 0) {
        d->onShift = 1;
      }
      else if (d->onShift == 2) {
        d->onShift = 0;
      }
    }
  }
  else if (keyID == 28) {
    if (!isUp) {
      rb = 1;
      d->on123 = d->on123 ? 0 : 1;
    }
  }
  else if (keyID == 34) {
    /* ctrl */
    if (!isUp) {
      d->keyD[34] = 0;
      d->onCTRL = d->onCTRL ? 0 : 1;
    }
  }
  else if (keyID == 30) {
    if (!isUp) {
      c = ' ';
      doThread = 1;
    }
  }
  else if (keyID == 29) {
    if (isUp) {
      c = ',';
    }
  }
  else if (keyID == 31) {
    if (isUp) {
      c = '.';
    }
  }
  else if (keyID == 33) {
    if (!isUp) {
      c = '\t';
    }
  }
  else if (keyID >= 35) {
    iscursor = 1;
    
    if (!isUp) {
      if (keyID == 35) {
        c = 37;  /* left */
      }
      else if (keyID == 36) {
        c = 38;  /* up */
      }
      else if (keyID == 37) {
        c = 40;  /* down */
      }
      else if (keyID == 38) {
        c = 39;  /* right */
      }
      
      doThread = 1;
    }
  }
  else if (keyID == 32) {
    if (!isUp) {
      c = '\n';
    }
  }
  else if (keyID == 27) {
    if (!isUp) {
      c = 8;
      doThread = 1;
    }
  }
  
  if (c != 0) {
    acime2_sendmsg(ctl, (byte) c, iscursor, iscursor);
  }
  
  if ((!isUp) && ((doThread) || (!d->on123))) {
    acime2_regThread(ctl);
  }
  
  if (rb) {
    int i;
    
    for (i = 0; i < ACIME2_BTNCNT; i++) {
      d->keyD[i] = 0;
    }
  }
}
dword acime2_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl   = (ACONTROLP) x;
  ACIMEDP  d      = (ACIMEDP) ctl->d;
  dword msg = 0;
  
  switch (action) {
    case ATEV_MOUSEUP: {
        if (d->pushedId < ACIME2_BTNCNT) {
          int keyID = d->pushedId;
          d->keyD[keyID]  = 0;
          d->pushedId     = 255;
          acime2_action(ctl, keyID, 1);
          
          if (((keyID < 27) && (keyID != 19)) || (keyID == 30) || (keyID == 29) || (keyID == 31)) {
            if (d->onShift == 1) {
              d->onShift = 0;
              int i;
              
              for (i = 0; i < ACIME2_BTNCNT; i++) {
                d->keyD[i] = 0;
              }
            }
          }
          
          ctl->ondraw(ctl);
          msg = aw_msg(0, 1, 0, 0);
        }
        else {
          d->pushedId = 255;
        }
      }
      break;
      
    case ATEV_MOUSEDN:
    case ATEV_MOUSEMV: {
        if ((d->pushedId != 200) && (d->pushedId != 254)) {
          int clientX = atev->x - ctl->x;
          int clientY = atev->y - ctl->y;
          
          if (clientY >= 0) {
            int i;
            
            for (i = 1; i < 5; i++) {
              if (clientY < (i * d->btnH)) {
                break;
              }
            }
            
            int r = i;
            byte idpos[5][2] = {
              {0,  9},
              {10, 18},
              {19, 27},
              {28, 32},
              {33, 38}
            };
            
            for (i = idpos[r - 1][0]; i < idpos[r - 1][1]; i++) {
              if (clientX < (d->keyX[i] + d->keyW[i])) {
                break;
              }
            }
            
            if (d->pushedId != i) {
              msg = aw_msg(0, 1, 0, 0);
              
              if (d->pushedId < ACIME2_BTNCNT) {
                d->keyD[d->pushedId]  = 0;
              }
              
              d->pushedId = i;
              acime2_action(ctl, i, 0);
              d->keyD[i]  = 0;
              ctl->ondraw(ctl);
              
              if (action == ATEV_MOUSEDN) {
                vibrate(30);
              }
            }
          }
        }
      }
      break;
  }
  
  return msg;
}
void acime2_drawbtn(ACONTROLP ctl, int keyID, int y) {
  ACIMEDP  d      = (ACIMEDP) ctl->d;
  
  if (((keyID < 27) && (keyID != 19)) || (keyID == 30) || (keyID == 31) || (keyID == 29) || (keyID == 34)) {
    char c[8];
    char c2 = 0;
    c[1] = 0;
    
    if (keyID == 30) {
      c[0] = ' ';
    }
    else if (keyID == 34) {
      snprintf(c, 8, "ctrl");
      
      if (d->onCTRL) {
        c2 = '*';
      }
    }
    else if (keyID == 29) {
      c[0] = ',';
    }
    else if (keyID == 31) {
      c[0] = '.';
    }
    else {
      int n = keyID;
      
      if (n > 19) {
        n--;
      }
      
      int np = 3;
      
      if (d->on123) {
        np = (d->onShift) ? 0 : 1;
      }
      else {
        if (d->onShift) {
          np = 2;
        }
        
        int np2 = (d->onShift) ? 0 : 1;
        c2 = acime2_charlist[np2][n];
      }
      
      c[0] = acime2_charlist[np][n];
    }
    
    color cl = (d->pushedId == keyID) ? acfg()->selectfg : acfg()->controlfg;
    int y1 = y;
    y     += (d->btnH / 2) - (ag_fontheight(1) / 2);
    int x  = (d->keyW[keyID] / 2) - (ag_txtwidth(c, 1) / 2);
    ag_textf(
      &d->control,
      d->keyW[keyID],
      d->keyX[keyID] + x,
      y,
      c,
      cl,
      1
    );
    
    if (c2) {
      cl = (d->pushedId == keyID) ? acfg()->selectfg : acfg()->textfg_gray;
      char c2s[2];
      c2s[0] = c2;
      c2s[1] = 0;
      ag_textf(
        &d->control,
        d->keyW[keyID],
        d->keyX[keyID] + d->keyW[keyID] - ag_txtwidth(c2s, 0) - agdp(),
        y1 + agdp(),
        c2s,
        cl,
        0
      );
    }
  }
  else {
    int icoid = 0;
    
    if (keyID == 19) {
      icoid = (!d->onShift ? 27 : (d->onShift == 2 ? 29 : 28));
    }
    else if (keyID == 28) {
      icoid = d->on123 ? 31 : 30;
    }
    else if (keyID == 27) {
      icoid = 32;  // bkspace;
    }
    else if (keyID == 33) {
      icoid = 42;  // tab;
    }
    else if (keyID == 35) {
      icoid = 34;  // left;
    }
    else if (keyID == 36) {
      icoid = 43;  // up;
    }
    else if (keyID == 37) {
      icoid = 44;  // down;
    }
    else if (keyID == 38) {
      icoid = 35;  // right;
    }
    else if (keyID == 32) {
      icoid = 41;  // enter;
    }
    
    PNGCANVAS * ap = aui_icons(icoid);
    
    if (ap != NULL) {
      int isz = agdp() * 16;
      y     += (d->btnH / 2) - (isz / 2);
      int x  = (d->keyW[keyID] / 2) - (isz / 2);
      apng_stretch(
        &d->control, ap, d->keyX[keyID] + x, y, isz, isz, 0, 0, ap->w, ap->h);
    }
  }
}
void acime2_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACIMEDP     d  = (ACIMEDP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  //-- Refresh undrawed items
  int i = 0;
  
  for (i = 0; i < ACIME2_BTNCNT; i++) {
    if (d->keyD[i] == 0) {
      int y = 4;
      
      if (i < 10) {
        y = 0;
      }
      else if (i < 19) {
        y = 1;
      }
      else if (i < 28) {
        y = 2;
      }
      else if (i < 33) {
        y = 3;
      }
      
      y *= d->btnH;
      ag_draw_ex(
        &d->control,
        ((d->pushedId == i) ? &d->control_push : &d->control_rest),
        d->keyX[i],
        y,
        d->keyX[i],
        y,
        d->keyW[i],
        d->btnH);
      acime2_drawbtn(ctl, i, y);
      d->keyD[i] = 1;
    }
  }
  
  ag_draw(pc, &d->control, ctl->x, ctl->y);
}
void acime2_ondestroy(void * x) {
  ACONTROLP ctl = (ACONTROLP) x;
  ACIMEDP   d   = (ACIMEDP) ctl->d;
  ag_ccanvas(&d->control_push);
  ag_ccanvas(&d->control_rest);
  ag_ccanvas(&d->control);
  free(ctl->d);
}
void acime2_drawbutton(ACIMEDP d, int x, int y, int w, int h, int id) {
  d->keyX[id] = x;
  d->keyW[id] = w;
  d->keyD[id] = 0;
  x++;
  y++;
  w -= 2;
  h -= 2;
  
  if (!atheme_draw("img.button", &d->control_rest, x, y, w, h)) {
    ag_roundgrad(&d->control_rest, x, y, w, h,          acfg()->border,     acfg()->border_g,    acfg()->roundsz);
    ag_roundgrad(&d->control_rest, x + 1, y + 1, w - 2, h - 2,  acfg()->controlbg,  acfg()->controlbg_g, acfg()->roundsz - 1);
  }
  
  if (!atheme_draw("img.button.push", &d->control_push, x, y, w, h)) {
    ag_roundgrad(&d->control_push, x, y, w, h,          acfg()->selectbg_g,     acfg()->selectbg,    acfg()->roundsz);
    ag_roundgrad(&d->control_push, x + 1, y + 1, w - 2, h - 2,  acfg()->selectbg,       acfg()->selectbg_g,  acfg()->roundsz - 1);
  }
}
void acime2_onblur(void * x) {}
ACONTROLP acime2(
  AWINDOWP win,
  int x, int y, int w, int h,
  byte inputMsg
) {
  //-- Initializing Button Data
  ACIMEDP d = (ACIMEDP) malloc(sizeof(ACIMED));
  memset(d, 0, sizeof(ACIMED));
  //-- Set Data
  d->inputMsg = inputMsg;
  d->onShift = 0;
  d->on123   = 0;
  d->onCTRL  = 0;
  d->pushedId = 255;
  //-- Init Canvas
  ag_canvas(&d->control, w, h);
  ag_canvas(&d->control_push, w, h);
  ag_canvas(&d->control_rest, w, h);
  //-- Drawings
  ag_roundgrad(&d->control_rest, 0, 0, w, h, acfg()->navbg, acfg()->navbg_g, 0);
  ag_rect(&d->control_rest, 0, 0, w, 1, acfg()->navbg);
  ag_draw(&d->control_push, &d->control_rest, 0, 0);
  //-- Calculate Size
  int btnW = floor(w / 10);
  d->btnH = floor(h / 5);
  //-- Draw Buttons
  int i     = 0;
  int bY    = 0;
  int w1p2  = (btnW / 2);
  int w3p2  = ((btnW * 3) / 2);
  
  for (i = 0; i < 10; i++) {
    acime2_drawbutton(d, i * btnW, bY, btnW, d->btnH, i);
  }
  
  bY += d->btnH;
  
  for (i = 0; i < 9; i++) {
    acime2_drawbutton(d, w1p2 + (i * btnW), bY, btnW, d->btnH, i + 10);
  }
  
  bY += d->btnH;
  acime2_drawbutton(d, 0, bY, w3p2, d->btnH, 19);            //-- SHIFT
  
  for (i = 0; i < 7; i++) {
    acime2_drawbutton(d, w3p2 + (i * btnW), bY, btnW, d->btnH, i + 20);
  }
  
  acime2_drawbutton(d, 8.5 * btnW, bY, w3p2, d->btnH, 27);   //-- BACKSPACE
  bY += d->btnH;
  acime2_drawbutton(d, 0,       bY, w3p2,    d->btnH, 28); //-- CHANGE 123-ABC
  acime2_drawbutton(d, w3p2,    bY, w3p2,    d->btnH, 29); //-- COMMA
  acime2_drawbutton(d, w3p2 * 2,  bY, btnW * 4,  d->btnH, 30); //-- SPACE
  acime2_drawbutton(d, 7 * btnW,  bY, w3p2,    d->btnH, 31); //-- DOT
  acime2_drawbutton(d, 8.5 * btnW, bY, w3p2,    d->btnH, 32); //-- ENTER
  bY += d->btnH;
  acime2_drawbutton(d, 0,         bY, btnW * 2,    d->btnH, 33); //-- TAB
  acime2_drawbutton(d, btnW * 2,      bY, btnW * 2,    d->btnH, 34); //-- CTRL
  acime2_drawbutton(d, 4 * btnW,    bY, w3p2,    d->btnH, 35); //-- LEFT
  acime2_drawbutton(d, 5.5 * btnW,  bY, w3p2,    d->btnH, 36); //-- UP
  acime2_drawbutton(d, 7 * btnW,    bY, w3p2,    d->btnH, 37); //-- DOWN
  acime2_drawbutton(d, 8.5 * btnW,  bY, w3p2,    d->btnH, 38); //-- RIGHT
  ag_draw(&d->control, &d->control_rest, 0, 0);
  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &acime2_ondestroy;
  ctl->oninput  = &acime2_oninput;
  ctl->ondraw   = &acime2_ondraw;
  ctl->onblur   = &acime2_onblur;
  ctl->onfocus  = NULL;
  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
  ctl->d        = (void *) d;
  aw_add(win, ctl);
  return ctl;
}