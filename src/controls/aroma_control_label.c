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
 * AROMA UI: Label
 *
 */
#include "../aroma.h"

/***************************[ BUTTON ]**************************/
typedef struct {
  CANVAS    control;
  char   *  text;
  byte      drawed;
  byte      vpos;
  byte      isbig;
  byte      sigleAligment;
  color     cl;
} ACLABELD, * ACLABELDP;

dword aclabel_oninput(void * x, int action, ATEV * atev) {
  return 0;
}
void aclabel_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACLABELDP   d  = (ACLABELDP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  
  if (!d->drawed) {
    ag_draw_ex(&d->control, ctl->win->bg, 0, 0, ctl->x, ctl->y, ctl->w, ctl->h);
    int h = 0;
    int y = 0;
    int x = 0;
    
    if (d->vpos != 0) {
      if (d->sigleAligment == 0) {
        h = ag_txtheight(ctl->w, d->text, d->isbig);
      }
      else {
        h     = ag_fontheight(d->isbig);
        int w = ag_txtwidth(d->text, d->isbig);
        
        if (d->sigleAligment == 2) {
          //-- Center
          x = (ctl->w / 2) - (w / 2);
        }
        else if (d->sigleAligment == 3) {
          //-- Right
          x = ctl->w - w;
        }
      }
    }
    
    if (d->vpos == 1) {
      y = (ctl->h / 2) - (h / 2);
    }
    else if (d->vpos == 2) {
      y = ctl->h - h;
    }
    
    if (d->sigleAligment == 0) {
      ag_text(&d->control, ctl->w, x, y, d->text, d->cl, d->isbig);
    }
    else {
      ag_texts(&d->control, ctl->w, x, y, d->text, d->cl, d->isbig);
    }
    
    d->drawed = 1;
  }
  
  ag_draw(pc, &d->control,  ctl->x, ctl->y);
}
void aclabel_ondestroy(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACLABELDP  d    = (ACLABELDP) ctl->d;
  ag_ccanvas(&d->control);
  free(d->text);
  free(ctl->d);
}
void aclabel_onblur(void * x) {
}
void aclabel_settext(ACONTROLP ctl, char * text, byte syncnow) {
  ACLABELDP  d    = (ACLABELDP) ctl->d;
  free(d->text);
  d->text = strdup(text);
  d->drawed = 0;
  aclabel_ondraw(ctl);
  
  if (syncnow) {
    aw_draw(ctl->win);
  }
}
char * aclabel_gettext(ACONTROLP ctl) {
  ACLABELDP  d    = (ACLABELDP) ctl->d;
  return d->text;
}
void aclabel_setprop(ACONTROLP ctl, byte isbig, byte vpos, byte sigleAligment, color cl, byte syncnow) {
  ACLABELDP  d    = (ACLABELDP) ctl->d;
  d->isbig = isbig;
  d->vpos  = vpos;
  d->drawed = 0;
  d->cl    = cl;
  d->sigleAligment = sigleAligment;
  aclabel_ondraw(ctl);
  
  if (syncnow) {
    aw_draw(ctl->win);
  }
}

ACONTROLP aclabel(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig,
  byte vpos,
  byte sigleAligment,
  color cl
) {
  //-- Validate Minimum Size
  if (h < agdp() * 2) {
    h = agdp() * 2;
  }
  
  if (w < agdp() * 10) {
    w = agdp() * 10;
  }
  
  //-- Initializing Button Data
  ACLABELDP d = (ACLABELDP) malloc(sizeof(ACLABELD));
  memset(d, 0, sizeof(ACLABELD));
  //-- Save Touch Message & Set Stats
  d->text      = strdup(text);
  d->isbig     = isbig;
  d->vpos      = vpos;
  d->drawed    = 0;
  d->cl        = cl;
  d->sigleAligment = sigleAligment;
  //-- Initializing Canvas
  ag_canvas(&d->control, w, h);
  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &aclabel_ondestroy;
  ctl->oninput  = &aclabel_oninput;
  ctl->ondraw   = &aclabel_ondraw;
  ctl->onblur   = &aclabel_onblur;
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