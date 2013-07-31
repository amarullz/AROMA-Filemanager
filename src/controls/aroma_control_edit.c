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
 * AROMA UI: Button Window Control
 *
 */
#include "../aroma.h"

/***************************[ BUTTON ]**************************/
#define ACEDIT_MAXLEN 256
typedef struct {
  CANVAS    control;
  char      text[ACEDIT_MAXLEN];
  int       text_len;
  int       caret_pos;
  int       drawY;
  byte      drawed;
  byte      isBig;
} ACEDITD, * ACEDITDP;

dword acedit_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl  = (ACONTROLP) x;
  ACEDITDP  d  = (ACEDITDP) ctl->d;
  dword msg = 0;
  
  switch (action) {
    case ATEV_MOUSEDN:
    case ATEV_MOUSEMV:
      //case ATEV_MOUSEUP:
      {
        if ((ATEV_MOUSEMV == action) && (d->drawY < 0)) {
        }
        else {
          int clientX = atev->x - ctl->x;
          int clientY = ((atev->y - ctl->y) - d->drawY) - (ag_fontheight(d->isBig) / 2);
          int i, caretPos  = 0, lastY = 0;
          
          for (i = 0; i <= d->text_len; i++) {
            int crX = 0;
            int crY = 0;
            ag_txtxy(&crX, &crY, ctl->w, d->text, d->isBig, i);
            
            if (lastY < crY) {
              caretPos = i;
              lastY = crY;
            }
            
            if (crY >= clientY) {
              break;
            }
          }
          
          for (i = caretPos; i <= d->text_len; i++) {
            int crX = 0;
            int crY = 0;
            caretPos = i;
            ag_txtxy(&crX, &crY, ctl->w, d->text, d->isBig, i);
            
            if (crX >= clientX) {
              break;
            }
          }
          
          d->caret_pos = caretPos;
          d->drawed = 0;
          msg = aw_msg(0, 1, 0, 0);
          ctl->ondraw(ctl);
        }
      }
      break;
      
    case ATEV_DOWN:
    case ATEV_RIGHT: {
        acedit_movecaret(ctl, 1);
        msg = aw_msg(0, 1, 1, 0);
      }
      break;
      
    case ATEV_UP:
    case ATEV_LEFT: {
        acedit_movecaret(ctl, 0);
        msg = aw_msg(0, 1, 1, 0);
      }
      break;
  }
  
  return msg;
}
void acedit_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACEDITDP    d  = (ACEDITDP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  CANVAS   *  cc = &d->control;
  
  if (!d->drawed) {
    //-- Calculate Sizes
    int txtH    = ag_txtheight(ctl->w, d->text, d->isBig);
    int lineH   = ag_fontheight(d->isBig);
    
    if (txtH == 0) {
      txtH = lineH;
    }
    
    int addY    = (ctl->h / 2) - (txtH / 2);
    int txtX    = 0;
    int txtY    = addY;
    //-- Find caret position
    int crX = 0;
    int crY = 0;
    ag_txtxy(&crX, &crY, ctl->w, d->text, d->isBig, d->caret_pos);
    
    //-- Reposition text location depend by caret
    if (txtY < 0) {
      txtY = 0 - (crY - round((ctl->h / 2.0) - (lineH / 2.0)));
      
      if (txtY > 0) {
        txtY = 0;
      }
      else if (txtY < (ctl->h - txtH)) {
        txtY = (ctl->h - txtH);
      }
    }
    
    //-- Set drawedY Value
    d->drawY = txtY;
    //-- Set Absolute caret coordinate
    crX += txtX + ctl->x;
    crY += txtY + ctl->y;
    //-- Set Global Caret Location
    ag_setcaret(crX, crY, ag_fontheight(d->isBig));
    //-- Draw Background
    ag_rect(cc, 0, 0, ctl->w, ctl->h, acfg()->textbg);
    //-- Draw Text
    ag_text(cc, ctl->w, txtX, txtY, d->text, acfg()->textfg, d->isBig);
    //-- Set drawed
    d->drawed = 1;
  }
  
  ag_draw(pc, cc, ctl->x, ctl->y);
}
void acedit_ondestroy(void * x) {
  ACONTROLP ctl = (ACONTROLP) x;
  ACEDITDP  d   = (ACEDITDP) ctl->d;
  ag_ccanvas(&d->control);
  free(ctl->d);
  ag_setcaret(0, 0, 0);
}
byte acedit_onfocus(void * x) {
  return 1;
}
void acedit_onblur(void * x) {}
ACONTROLP acedit(
  AWINDOWP win,
  int x, int y, int w, int h,
  char * text, byte isBig
) {
  //-- Initializing Button Data
  ACEDITDP d = (ACEDITDP) malloc(sizeof(ACEDITD));
  memset(d, 0, sizeof(ACEDITD));
  //-- Set Data
  snprintf(d->text, ACEDIT_MAXLEN, "%s", text);
  d->text_len = strlen(d->text);
  d->caret_pos = d->text_len;
  d->drawY    = 0;
  d->drawed   = 0;
  d->isBig    = isBig;
  //-- Init Canvas
  ag_canvas(&d->control, w, h);
  ag_rect(&d->control, 0, 0, w, h, acfg()->textbg);
  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &acedit_ondestroy;
  ctl->oninput  = &acedit_oninput;
  ctl->ondraw   = &acedit_ondraw;
  ctl->onblur   = &acedit_onblur;
  ctl->onfocus  = &acedit_onfocus;
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
char * acedit_gettext(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACEDITDP    d  = (ACEDITDP) ctl->d;
  return d->text;
}
void acedit_addchar(void * x, char c) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACEDITDP    d  = (ACEDITDP) ctl->d;
  
  if (d->text_len < 255) {
    d->text_len++;
    int i;
    
    for (i = d->text_len - 1; i > d->caret_pos; i--) {
      d->text[i] = d->text[i - 1];
    }
    
    d->text[d->caret_pos] = c;
    d->caret_pos++;
    d->text[d->text_len] = 0;
    //-- Draw
    d->drawed = 0;
    ctl->ondraw(ctl);
    aw_draw(ctl->win);
  }
}
void acedit_rchar(void * x, char c) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACEDITDP    d  = (ACEDITDP) ctl->d;
  
  if (d->caret_pos > 0) {
    d->text[d->caret_pos - 1] = c;
    //-- Draw
    d->drawed = 0;
    ctl->ondraw(ctl);
    aw_draw(ctl->win);
  }
}
void acedit_backspace(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACEDITDP    d  = (ACEDITDP) ctl->d;
  
  if (d->caret_pos > 0) {
    d->caret_pos--;
    int i;
    
    for (i = d->caret_pos; i < d->text_len; i++) {
      d->text[i] = d->text[i + 1];
    }
    
    d->text_len--;
    d->text[d->text_len] = 0;
    //-- Draw
    d->drawed = 0;
    ctl->ondraw(ctl);
    aw_draw(ctl->win);
  }
}
void acedit_movecaret(void * x, byte isright) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACEDITDP    d  = (ACEDITDP) ctl->d;
  
  if (!isright) {
    if (d->caret_pos > 0) {
      d->caret_pos--;
      //-- Draw
      d->drawed = 0;
      ctl->ondraw(ctl);
      aw_draw(ctl->win);
    }
  }
  else {
    if (d->caret_pos < d->text_len) {
      d->caret_pos++;
      //-- Draw
      d->drawed = 0;
      ctl->ondraw(ctl);
      aw_draw(ctl->win);
    }
  }
}