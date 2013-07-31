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
 * AROMA UI: Progress Bar
 *
 */
#include "../aroma.h"

/***************************[ BUTTON ]**************************/
typedef struct {
  CANVAS    bg;
  CANVAS    control;
  float     value;
  byte      drawed;
  int       width_current;
  byte      hidden;
  byte      onwait;
  float     waitpos;
  byte      wait_thread;
} APROGD, * APROGDP;

dword acprog_oninput(void * x, int action, ATEV * atev) {
  return 0;
}
void acprog_ondraw(void * x) {
  ACONTROLP ctl = (ACONTROLP) x;
  APROGDP   d  = (APROGDP) ctl->d;
  CANVAS  * pc = &ctl->win->c;
  CANVAS  * c  = &d->control;
  
  if (d->hidden) {
    ag_draw_ex(pc, ctl->win->bg, ctl->x, ctl->y, ctl->x, ctl->y, ctl->w, ctl->h);
    return;
  }
  
  if (d->onwait) {
    int hp2    = floor(ctl->h / 2.0);
    int draw_w = floor(ctl->w / 1.618);
    int move_w = ctl->w - draw_w;
    float crps = d->waitpos;
    
    if (crps > 1.0) {
      crps = 2.0 - crps;
    }
    
    int move_x = round(move_w * crps);
    ag_draw(c, &d->bg, 0, 0);
    
    if (!atheme_draw("img.prograss.fill", c, move_x, 0, draw_w, ctl->h)) {
      dword hl1 = ag_calchighlight(acfg()->selectbg, acfg()->selectbg_g);
      ag_roundgrad(c, move_x, 0, draw_w, ctl->h, acfg()->selectbg_g, acfg()->selectbg, hp2);
      ag_roundgrad(c, move_x + 2, 2, draw_w - 4, ctl->h - 4, acfg()->selectbg, acfg()->selectbg_g, hp2 - 2);
      ag_roundgrad_ex(c, move_x + 3, 3, draw_w - 6, ceil((ctl->h - 6) / 2.0), LOWORD(hl1), HIWORD(hl1), hp2, 2, 2, 0, 0);
    }
  }
  else if (!d->drawed) {
    int issmall = -1;
    int draw_w  = d->width_current;
    int hp2     = floor(ctl->h / 2.0);
    
    if (d->width_current < ctl->h) {
      issmall   = d->width_current;
      draw_w    = ctl->h;
    }
    
    ag_draw(c, &d->bg, 0, 0);
    
    if (!atheme_draw("img.prograss.fill", c, 0, 0, draw_w, ctl->h)) {
      dword hl1 = ag_calchighlight(acfg()->selectbg, acfg()->selectbg_g);
      ag_roundgrad(c, 0, 0, draw_w, ctl->h, acfg()->selectbg_g, acfg()->selectbg, hp2);
      ag_roundgrad(c, 2, 2, draw_w - 4, ctl->h - 4, acfg()->selectbg, acfg()->selectbg_g, hp2 - 2);
      ag_roundgrad_ex(c, 3, 3, draw_w - 6, ceil((ctl->h - 6) / 2.0), LOWORD(hl1), HIWORD(hl1), hp2, 2, 2, 0, 0);
    }
    
    if (issmall >= 0) {
      ag_draw_ex(c, &d->bg, issmall, 0, issmall, 0, ctl->w - issmall, ctl->h);
    }
    
    d->drawed = 1;
  }
  
  ag_draw(pc, &d->control,  ctl->x, ctl->y);
}
static void * acprog_onwaitthread(void * cookie) {
  ACONTROLP ctl = (ACONTROLP) cookie;
  APROGDP   d  = (APROGDP) ctl->d;
  
  if (ctl->win->isActived) {
    ctl->win->threadnum++;
  }
  else {
    return NULL;
  }
  
  while (ctl->win->isActived && d->onwait) {
    d->waitpos += 0.05;
    
    if (d->waitpos >= 2.0) {
      d->waitpos = 0.0;
    }
    
    acprog_ondraw(ctl);
    aw_draw(ctl->win);
    usleep(8000);
  }
  
  d->wait_thread = 0;
  ctl->win->threadnum--;
  return NULL;
}
void acprog_ondestroy(void * x) {
  ACONTROLP ctl = (ACONTROLP) x;
  APROGDP   d   = (APROGDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->bg);
  free(ctl->d);
}
void acprog_onblur(void * x) {
}
float acprog_getvalue(ACONTROLP ctl) {
  APROGDP  d = (APROGDP) ctl->d;
  return d->value;
}
void acprog_setvalue(ACONTROLP ctl, float value, byte syncnow) {
  APROGDP  d      = (APROGDP) ctl->d;
  
  if (value > 1) {
    value = 1.0;
  }
  
  if (value < 0) {
    value = 0.0;
  }
  
  d->onwait       = 0;
  d->value        = value;
  d->width_current = ceil(d->value * ctl->w);
  d->drawed = 0;
  acprog_ondraw(ctl);
  
  if (syncnow) {
    aw_draw(ctl->win);
  }
}
void acprog_sethidden(ACONTROLP ctl, byte hidden, byte syncnow) {
  APROGDP  d      = (APROGDP) ctl->d;
  d->hidden       = hidden;
  d->drawed       = 0;
  acprog_ondraw(ctl);
  
  if (syncnow) {
    aw_draw(ctl->win);
  }
}
void acprog_setonwait(ACONTROLP ctl, byte onwait) {
  APROGDP  d      = (APROGDP) ctl->d;
  d->onwait       = onwait;
  d->waitpos      = 0.0;
  
  if ((!d->wait_thread) && (d->onwait)) {
    d->wait_thread = 1;
    pthread_t th;
    pthread_create(&th, NULL, acprog_onwaitthread, (void *) ctl);
    pthread_detach(th);
  }
}
ACONTROLP acprog(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  float value
) {
  //-- Validate Minimum Size
  if (h < agdp() * 5) {
    h = agdp() * 5;
  }
  
  if (w < agdp() * 10) {
    w = agdp() * 10;
  }
  
  //-- Initializing Button Data
  APROGDP d = (APROGDP) malloc(sizeof(APROGD));
  memset(d, 0, sizeof(APROGD));
  //-- Save Touch Message & Set Stats
  d->value        = value;
  d->drawed       = 0;
  d->width_current = ceil(value * w);
  d->hidden       = 0;
  d->onwait       = 0;
  d->waitpos      = 0.0;
  d->wait_thread  = 0;
  //-- Initializing Canvas
  ag_canvas(&d->control, w, h);
  ag_canvas(&d->bg, w, h);
  //-- Draw Background
  CANVAS * c = &d->bg;
  int hp2 = floor(h / 2.0);
  ag_draw_ex(c, win->bg, 0, 0, x, y, w, h);
  
  if (!atheme_draw("img.progress", c, 0, 0, w, h)) {
    dword hl1 = ag_calchighlight(acfg()->controlbg, acfg()->controlbg_g);
    ag_roundgrad(c, 0, 0, w, h, acfg()->border, acfg()->border_g, hp2);
    ag_roundgrad(c, 1, 1, w - 2, h - 2,
                 ag_calculatealpha(acfg()->controlbg,  0xffff, 180),
                 ag_calculatealpha(acfg()->controlbg_g, 0xffff, 160),
                 hp2 - 1);
    ag_roundgrad(c, 2, 2, w - 4, h - 4, acfg()->controlbg, acfg()->controlbg_g, hp2 - 2);
    ag_roundgrad_ex(c, 2, 2, w - 4, ceil((h - 4) / 2.0), LOWORD(hl1), HIWORD(hl1), hp2 - 2, 2, 2, 0, 0);
  }
  
  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &acprog_ondestroy;
  ctl->oninput  = &acprog_oninput;
  ctl->ondraw   = &acprog_ondraw;
  ctl->onblur   = &acprog_onblur;
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