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
 * AROMA UI: Main AROMA UI Window
 *
 */
#include <sched.h>
#include "../aroma.h"

/***************************[ GLOBAL VARIABLES ]**************************/
static AC_CONFIG acfg_var;
static byte      on_dialog_window = 0;

/***************************[ CONFIG FUNCTIONS ]**************************/
AC_CONFIG * acfg() {
  return &acfg_var;
}
void acfg_init_ex(byte themeonly) {
  acfg_var.winbg        = ag_rgb(0xf0, 0xf0, 0xf0);
  acfg_var.winbg_g      = ag_rgb(0xee, 0xee, 0xee);
  acfg_var.dialogbg     = acfg_var.winbg;
  acfg_var.dialogbg_g   = acfg_var.winbg_g;
  acfg_var.textbg       = ag_rgb(0xff, 0xff, 0xff);
  acfg_var.textfg       = ag_rgb(0x00, 0x00, 0x00);
  acfg_var.textfg_gray  = ag_rgb(0x88, 0x88, 0x88);
  acfg_var.winfg_gray   = acfg_var.textfg_gray;
  acfg_var.winfg        = acfg_var.textfg;
  acfg_var.dialogfg     = acfg_var.textfg;
  acfg_var.controlbg    = ag_rgb(0xf5, 0xf5, 0xf5);
  acfg_var.controlbg_g  = ag_rgb(0xdf, 0xdf, 0xdf);
  acfg_var.controlfg    = ag_rgb(0x44, 0x44, 0x44);
  acfg_var.selectbg     = ag_rgb(158, 228, 32);
  acfg_var.selectbg_g   = ag_rgb(106, 160, 44);
  acfg_var.selectfg     = ag_rgb(0xff, 0xff, 0xff);
  acfg_var.titlebg      = ag_rgb(0x22, 0x22, 0x22);
  acfg_var.titlebg_g    = ag_rgb(0x11, 0x11, 0x11);
  acfg_var.titlefg      = ag_rgb(0xff, 0xff, 0xff);
  acfg_var.dlgtitlebg   = acfg_var.titlebg;
  acfg_var.dlgtitlebg_g = acfg_var.titlebg_g;
  acfg_var.dlgtitlefg   = acfg_var.titlefg;
  acfg_var.navbg        = ag_rgb(0xf5, 0xf5, 0xf5);
  acfg_var.navbg_g      = ag_rgb(0xdf, 0xdf, 0xdf);
  acfg_var.scrollbar    = ag_rgb(0x66, 0x66, 0x66);
  acfg_var.border       = ag_rgb(0xcc, 0xcc, 0xcc);
  acfg_var.border_g     = ag_rgb(0x99, 0x99, 0x99);
  acfg_var.progressglow = acfg_var.selectbg;
  acfg_var.winroundsz   = 3;
  acfg_var.roundsz      = 2;
  acfg_var.btnroundsz   = 2;
  snprintf(acfg_var.themename, 64, "");
  
  if (themeonly == 0) {
    acfg_var.ckey_up      = 0;
    acfg_var.ckey_down    = 0;
    acfg_var.ckey_select  = 0;
    acfg_var.ckey_back    = 0;
    acfg_var.ckey_menu    = 0;
  }
  
  atheme_releaseall();
}
void acfg_init() {
  acfg_init_ex(0);
}

/***************************[ THEME ]**************************/
static char theme_name[AROMA_THEME_CNT][27] = {
  "img.background",
  "img.titlebar",
  "img.navbar",
  "img.dialog",
  "img.dialog.titlebar",
  "img.progress",
  "img.prograss.fill",
  "img.selection",
  "img.selection.push",
  "img.button",
  "img.button.focus",
  "img.button.push",
  "img.checkbox",
  "img.checkbox.focus",
  "img.checkbox.push",
  "img.checkbox.on",
  "img.checkbox.on.focus",
  "img.checkbox.on.push",
  "img.radio",
  "img.radio.focus",
  "img.radio.push",
  "img.radio.on",
  "img.radio.on.focus",
  "img.radio.on.push"
};
void atheme_releaseall() {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (acfg_var.theme[i] != NULL) {
      apng_close(acfg_var.theme[i]);
      free(acfg_var.theme[i]);
    }
    
    acfg_var.theme[i]   = NULL;
    acfg_var.theme_9p[i] = 0;
  }
}
void atheme_release(char * key) {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (strcmp(theme_name[i], key) == 0) {
      if (acfg_var.theme[i] != NULL) {
        apng_close(acfg_var.theme[i]);
        free(acfg_var.theme[i]);
        acfg_var.theme[i] = NULL;
        acfg_var.theme_9p[i] = 0;
      }
      
      return;
    }
  }
  
  return;
}
PNGCANVASP atheme_create(char * key, char * path) {
  int id = atheme_id(key);
  
  if (id != -1) {
    PNGCANVAS * ap = malloc(sizeof(PNGCANVAS));
    
    if (apng_load(ap, path)) {
      if (acfg_var.theme[id] != NULL) {
        apng_close(acfg_var.theme[id]);
        free(acfg_var.theme[id]);
        acfg_var.theme[id] = NULL;
        acfg_var.theme_9p[id] = 0;
      }
      
      acfg_var.theme[id]  = ap;
      int ln = strlen(path) - 1;
      acfg_var.theme_9p[id] = 0;
      
      if (ln > 2) {
        if ((path[ln] == '9') && (path[ln - 1] == '.')) {
          acfg_var.theme_9p[id] = 1;
        }
      }
      
      return ap;
    }
    
    free(ap);
  }
  
  return NULL;
}
byte atheme_draw(char * key, CANVAS * _b, int x, int y, int w, int h) {
  return atheme_id_draw(atheme_id(key), _b, x, y, w, h);
}
byte atheme_id_draw(int id, CANVAS * _b, int x, int y, int w, int h) {
  if (id < 0) {
    return 0;
  }
  
  if (id >= AROMA_THEME_CNT) {
    return 0;
  }
  
  if (acfg_var.theme[id] != NULL) {
    if (acfg_var.theme_9p[id]) {
      return apng9_draw(_b, acfg_var.theme[id], x, y, w, h, NULL, 1);
    }
    else {
      return apng_stretch(
               _b,
               acfg_var.theme[id],
               x, y, w, h,
               0, 0, acfg_var.theme[id]->w, acfg_var.theme[id]->h);
    }
  }
  
  return 0;
}
PNGCANVASP atheme(char * key) {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (strcmp(theme_name[i], key) == 0) {
      return acfg_var.theme[i];
    }
  }
  
  return NULL;
}
int atheme_id(char * key) {
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    if (strcmp(theme_name[i], key) == 0) {
      return i;
    }
  }
  
  return -1;
}
char * atheme_key(int id) {
  if (id < 0) {
    return NULL;
  }
  
  if (id >= AROMA_THEME_CNT) {
    return NULL;
  }
  
  return theme_name[id];
}


/***************************[ WINDOW FUNCTIONS ]**************************/
//-- CREATE WINDOW
AWINDOWP aw(CANVAS * bg) {
  ag_setbusy();
  //sleep(4);
  //-- Create Window
  AWINDOWP win = (AWINDOWP) malloc(sizeof(AWINDOW));
  
  if (win == NULL) {
    return NULL;
  }
  
  //-- Create Canvas & Draw BG
  ag_canvas(&win->c, agw(), agh());
  ag_draw(&win->c, bg, 0, 0);
  //-- Initializing Variables
  win->bg           = bg;
  win->controls     = NULL;
  win->controln     = 0;
  win->threadnum    = 0;
  win->focusIndex   = -1;
  win->touchIndex   = -1;
  win->isActived    = 0;
  //-- RETURN
  return win;
}
void aw_set_on_dialog(byte d) {
  on_dialog_window = d;
}
//-- DESTROY WINDOW
void aw_destroy(AWINDOWP win) {
  ag_setbusy();
  //-- Set To Unactive
  win->isActived = 0;
  //-- Wait Thread To Closed
  int threadwait_n = 0;
  
  while (win->threadnum > 0) {
    usleep(500);
    
    if (threadwait_n++ > 1000) {
      break;
    }
  }
  
  //-- Cleanup Controls
  if (win->controln > 0) {
    int i;
    ACONTROLP * controls = (ACONTROLP *) win->controls;
    
    for (i = win->controln - 1; i >= 0; i--) {
      controls[i]->ondestroy((void *) controls[i]);
      free(controls[i]);
    }
    
    free(win->controls);
  }
  
  //-- Cleanup Window
  ag_ccanvas(&win->c);
  free(win);
}

//-- Add Control Into Window
void aw_add(AWINDOWP win, ACONTROLP ctl) {
  if (win->controln > 0) {
    int i;
    void ** tmpctls   = win->controls;
    win->controls     = malloc( sizeof(ACONTROLP) * (win->controln + 1) );
    
    for (i = 0; i < win->controln; i++) {
      win->controls[i] = tmpctls[i];
    }
    
    win->controls[win->controln] = (void *) ctl;
    free(tmpctls);
  }
  else {
    win->controls    = malloc(sizeof(ACONTROLP));
    win->controls[0] = (void *) ctl;
  }
  
  win->controln++;
}

//-- Draw Window
void aw_draw(AWINDOWP win) {
  if (!win->isActived) {
    return;
  }
  
  ag_draw(NULL, &win->c, 0, 0);
  ag_sync();
}

//-- Redraw Window & Controls
void aw_redraw_ex(AWINDOWP win, byte syncnow) {
  if (!win->isActived) {
    return;
  }
  
  if (win->controln > 0) {
    int i;
    
    for (i = 0; i < win->controln; i++) {
      ACONTROLP ctl = (ACONTROLP) win->controls[i];
      
      if (ctl->ondraw != NULL) {
        ctl->ondraw(ctl);
      }
    }
  }
  
  if (syncnow) {
    ag_draw(NULL, &win->c, 0, 0);
  }
}

void aw_redraw(AWINDOWP win) {
  aw_redraw_ex(win, 1);
}

//-- Show Window
void aw_show_ex2(AWINDOWP win, byte anitype, int x, int pos, int w, int h, ACONTROLP firstFocus) {
  win->threadnum    = 0;
  win->isActived    = 1;
  
  //-- Find First Focus
  if (win->controln > 0) {
    int i;
    
    if (firstFocus != NULL) {
      for (i = 0; i < win->controln; i++) {
        ACONTROLP ctl = (ACONTROLP) win->controls[i];
        
        if (firstFocus == ctl) {
          if (ctl->onfocus != NULL) {
            if (ctl->onfocus(ctl)) {
              win->focusIndex = i;
              break;
            }
          }
        }
      }
    }
    
    if (win->focusIndex == -1) {
      for (i = 0; i < win->controln; i++) {
        ACONTROLP ctl = (ACONTROLP) win->controls[i];
        
        if (ctl->onfocus != NULL) {
          if (ctl->onfocus(ctl)) {
            win->focusIndex = i;
            break;
          }
        }
      }
    }
  }
  
  if (acfg()->fadeframes > 0) {
    if (anitype == 0) {
      //-- Fade
      aw_redraw(win);
      ag_sync_fade(acfg_var.fadeframes);
    }
    else if (anitype == 1) {
      //-- Bottom Top
      aw_redraw_ex(win, 0);
      int drawh  = agh() - pos;
      int anisz = floor(((float) drawh) / acfg()->fadeframes);
      int i;
      
      for (i = 1; i <= acfg()->fadeframes; i++) {
        ag_draw_ex(NULL, &win->c, 0, agh() - (anisz * i), 0, pos, agw(), drawh);
        ag_sync();
      }
      
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 2) {
      //-- Right Left
      aw_redraw_ex(win, 0);
      int drawh  = agh() - pos;
      int anisz = floor(((float) agw()) / acfg()->fadeframes);
      int i;
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      
      for (i = 1; i <= acfg()->fadeframes; i++) {
        ag_draw_ex(NULL, &cbg, 0 - (anisz * i), pos, 0, pos, agw(), drawh);
        ag_draw_ex(NULL, &win->c, agw() - (anisz * i), pos, 0, pos, agw(), drawh);
        ag_sync();
      }
      
      ag_ccanvas(&cbg);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 3) {
      //-- Left Right
      aw_redraw_ex(win, 0);
      int drawh  = agh() - pos;
      int anisz = floor(((float) agw()) / acfg()->fadeframes);
      int i;
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      
      for (i = 1; i <= acfg()->fadeframes; i++) {
        ag_draw_ex(NULL, &cbg, (anisz * i), pos, 0, pos, agw(), drawh);
        ag_draw_ex(NULL, &win->c, 0 - (agw() - (anisz * i)), pos, 0, pos, agw(), drawh);
        ag_sync();
      }
      
      ag_ccanvas(&cbg);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 4) {
      //-- Scale
      byte fadesz = floor(acfg()->fadeframes / 2);
      
      if (fadesz > 1) {
        ag_sync();
        aw_redraw_ex(win, 0);
        CANVAS cbg;
        ag_canvas(&cbg, agw(), agh());
        ag_draw(&cbg, agc(), 0, 0);
        int i;
        CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
        memset(tmpb, 0, sizeof(CANVAS)*fadesz);
        
        for (i = 1; i <= fadesz; i++) {
          /* Calculating Scale */
          byte scale  = (i * 0xff) / fadesz;
          ag_canvas(&tmpb[i - 1], w, h);
          ag_draw(&tmpb[i - 1], &cbg, 0, 0);
          ag_draw_opa(&tmpb[i - 1], &win->c, 0, 0, scale, 1);
        }
        
        ag_ccanvas(&cbg);
        
        for (i = 0; i < fadesz; i++) {
          ag_draw(NULL, &tmpb[i], x, pos);
          ag_ccanvas(&tmpb[i]);
          ag_sync();
        }
        
        free(tmpb);
        ag_draw(NULL, &win->c, 0, 0);
        ag_sync();
      }
      else {
        aw_redraw(win);
        ag_sync();
      }
    }
    else if (anitype == 5) {
      //-- Scale
      byte fadesz = acfg()->fadeframes;
      ag_sync();
      aw_redraw_ex(win, 0);
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc = w / 2;
      int yc = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        /* Calculating Scale */
        byte scale  = (i * 0xff) / fadesz;
        scale = (scale * (0x200 - scale)) >> 8;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_ex(&tmpb[i - 1], &cbg, 0, 0, x, pos, w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &win->c,
          xc - wtarget / 2, yc - htarget / 2, wtarget, htarget,
          x, pos, w, h, scale, 1
        );
      }
      
      ag_ccanvas(&cbg);
      
      for (i = 0; i < fadesz; i++) {
        ag_draw(NULL, &tmpb[i], x, pos);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 6) {
      //-- Stack Right to Left
      int fadesz = acfg()->fadeframes;
      ag_sync();
      aw_redraw_ex(win, 0);
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc = w / 2;
      int yc = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        byte scale  = (i * 0xff) / fadesz;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        int xtarget = (w * scale) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &win->c,
          w - wtarget, yc - htarget / 2, wtarget, htarget,
          x, pos, w, h, scale, 0
        );
        ag_draw_ex(&tmpb[i - 1], &cbg, 0, 0, x + xtarget, pos, w - xtarget, h);
      }
      
      ag_ccanvas(&cbg);
      
      for (i = 0; i < fadesz; i++) {
        ag_draw(NULL, &tmpb[i], x, pos);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else if (anitype == 7) {
      //-- Stack Right to Left
      int fadesz = acfg()->fadeframes;
      ag_sync();
      aw_redraw_ex(win, 0);
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc = w / 2;
      int yc = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        byte scale  = (i * 0xff) / fadesz;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        int xtarget = (w * scale) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &cbg,
          w - wtarget, yc - htarget / 2, wtarget, htarget,
          x, pos, w, h, scale, 0
        );
        ag_draw_ex(&tmpb[i - 1], &win->c, 0, 0, x + xtarget, pos, w - xtarget, h);
      }
      
      ag_ccanvas(&cbg);
      
      for (i = fadesz - 1; i >= 0; i--) {
        ag_draw(NULL, &tmpb[i], x, pos);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
      ag_draw(NULL, &win->c, 0, 0);
      ag_sync();
    }
    else {
      //-- No Effect
      aw_redraw(win);
      ag_sync();
    }
  }
  else {
    //-- No Effect
    aw_redraw(win);
    ag_sync();
  }
  
  ui_clear_key_queue_ex();
}
void aw_show_ex(AWINDOWP win, byte anitype, int pos, ACONTROLP firstFocus) {
  aw_show_ex2(win, anitype, 0, pos, agw(), agh(), firstFocus);
}
//-- Show Window
void aw_show(AWINDOWP win) {
  aw_show_ex(win, 0, 0, NULL);
}




//-- Post Message
void aw_post(dword msg) {
  atouch_send_message(msg);
}

//-- Check Mouse Event
byte aw_touchoncontrol(ACONTROLP ctl, int x, int y) {
  int wx  = ctl->x;
  int wx2 = wx + ctl->w;
  int wy  = ctl->y;
  int wy2 = wy + ctl->h;
  
  if ((x >= wx) && (x < wx2) && (y >= wy) && (y < wy2)) {
    return 1;
  }
  
  return 0;
}

//-- Set Focus
byte aw_setfocus(AWINDOWP win, ACONTROLP ctl) {
  if (!win->isActived) {
    return 0;
  }
  
  int i;
  
  for (i = 0; i < win->controln; i++) {
    ACONTROLP fctl = (ACONTROLP) win->controls[i];
    
    if (fctl == ctl) {
      if (fctl->onfocus != NULL) {
        if (fctl->onfocus(fctl)) {
          int pf = win->focusIndex;
          win->focusIndex = i;
          
          if ((pf != -1) && (pf != i)) {
            ACONTROLP pctl = (ACONTROLP) win->controls[pf];
            pctl->onblur(pctl);
          }
          
          aw_draw(win);
          return 1;
        }
      }
    }
  }
  
  return 0;
}

//-- Dispatch Messages
dword aw_dispatch_ex(AWINDOWP win, int miny) {
  dword msg;
  int i;
  byte prev_touch_out = 0;
  
  // ui_clear_key_queue();
  while (1) {
    //-- Wait For Event
    ATEV          atev;
    int action  = atouch_wait(&atev);
    //-- Reset Message Value
    msg         = aw_msg(0, 0, 0, 0);
    
    //-- Check an Action Value
    switch (action) {
      case ATEV_MESSAGE: {
          msg = atev.msg;
        }
        break;
        
      case ATEV_MENU: {
          if (!atev.d) {
            if (!on_dialog_window) {
              msg = aw_msg(3, 0, 0, 0);
            }
            else if (on_dialog_window == 2) {
              msg = aw_msg(5, 0, 0, 0);
            }
          }
        }
        break;
        
      case ATEV_BACK: {
          if (!atev.d) {
            if (!on_dialog_window) {
              msg = aw_msg(12, 0, 0, 0);
            }
            else if (on_dialog_window == 2) {
              msg = aw_msg(5, 0, 0, 0);
            }
          }
        }
        break;
        
      case ATEV_DOWN:
      case ATEV_RIGHT:
        if (!atev.d) {
          if (win->focusIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->focusIndex];
            
            if (ctl->oninput != NULL) {
              msg = ctl->oninput((void *)ctl, action, &atev);
            }
            
            if (aw_gl(msg) == 0) {
              for (i = win->focusIndex + 1; i < win->controln; i++) {
                ACONTROLP fctl = (ACONTROLP) win->controls[i];
                
                if (fctl->onfocus != NULL) {
                  if (fctl->onfocus(fctl)) {
                    win->focusIndex = i;
                    ctl->onblur(ctl);
                    aw_draw(win);
                    break;
                  }
                }
              }
            }
          }
        }
        
        break;
        
      case ATEV_UP:
      case ATEV_LEFT:
        if (!atev.d) {
          if (win->focusIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->focusIndex];
            
            if (ctl->oninput != NULL) {
              msg = ctl->oninput((void *)ctl, action, &atev);
            }
            
            if (aw_gl(msg) == 0) {
              for (i = win->focusIndex - 1; i >= 0; i--) {
                ACONTROLP fctl = (ACONTROLP) win->controls[i];
                
                if (fctl->onfocus != NULL) {
                  if (fctl->onfocus(fctl)) {
                    win->focusIndex = i;
                    ctl->onblur(ctl);
                    aw_draw(win);
                    break;
                  }
                }
              }
            }
          }
        }
        
        break;
        
      case ATEV_SELECT: {
          if (win->focusIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->focusIndex];
            
            if (ctl->oninput != NULL) {
              msg = ctl->oninput((void *)ctl, action, &atev);
            }
          }
        }
        break;
        
      case ATEV_MOUSEDN: {
          if (miny > atev.y) {
            prev_touch_out = 1;
          }
          else if (win->controln > 0) {
            int i;
            
            for (i = win->controln - 1; i >= 0; i--) {
              ACONTROLP ctl = (ACONTROLP) win->controls[i];
              
              if (aw_touchoncontrol(ctl, atev.x, atev.y)) {
                if (ctl->oninput != NULL) {
                  msg             = ctl->oninput((void *)ctl, action, &atev);
                  win->touchIndex = i;
                  break;
                }
              }
            }
          }
        }
        break;
        
      case ATEV_MOUSEUP: {
          if (prev_touch_out) {
            if (miny > atev.y) {
              msg = aw_msg(5, 0, 0, 0);
            }
          }
          else if (win->touchIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->touchIndex];
            
            if (ctl->oninput != NULL) {
              msg             = ctl->oninput((void *)ctl, action, &atev);
            }
            
            win->touchIndex   = -1;
          }
        }
        break;
        
      case ATEV_MOUSEMV: {
          if (win->touchIndex != -1) {
            ACONTROLP ctl = (ACONTROLP) win->controls[win->touchIndex];
            
            if (ctl->oninput != NULL) {
              msg             = ctl->oninput((void *)ctl, action, &atev);
            }
          }
        }
        break;
    }
    
    if (aw_gd(msg) == 1) {
      aw_draw(win);
    }
    
    if (aw_gm(msg) != 0) {
      return msg;
    }
  }
  
  return msg;
}
dword aw_dispatch(AWINDOWP win) {
  return aw_dispatch_ex(win, 0);
}
CANVAS * aw_muteparent(AWINDOWP win) {
  if (win == NULL) {
    //-- Set Temporary
    CANVAS * tmpbg = (CANVAS *) malloc(sizeof(CANVAS));
    ag_canvas(tmpbg, agw(), agh());
    ag_draw(tmpbg, agc(), 0, 0);
    return tmpbg;
  }
  else {
    win->isActived = 0;
    return NULL;
  }
}
CANVAS * aw_maskparent() {
  CANVAS * tmpbg = (CANVAS *) malloc(sizeof(CANVAS));
  ag_canvas(tmpbg, agw(), agh());
  ag_draw(tmpbg, agc(), 0, 0);
  ag_rectopa(tmpbg, 0, 0, agw(), agh(), 0x0000, 180);
  ag_draw(agc(), tmpbg, 0, 0);
  return tmpbg;
}
void aw_unmuteparent(AWINDOWP win, CANVAS * p) {
  if (win == NULL) {
    if (p != NULL) {
      ag_draw(NULL, p, 0, 0);
      //ag_sync_fade(acfg_var.fadeframes);
      ag_sync();
      ag_ccanvas(p);
      free(p);
    }
  }
  else {
    if (p != NULL) {
      ag_ccanvas(p);
      free(p);
    }
    
    win->isActived = 1;
    ag_draw(NULL, &win->c, 0, 0);
    //ag_sync_fade(acfg_var.fadeframes);
    ag_sync();
  }
}
void aw_unmaskparent(AWINDOWP win, CANVAS * p, CANVAS * maskc, int x, int y, int w, int h) {
  if (maskc != NULL) {
    CANVAS * wincanvas = NULL;
    
    if (win == NULL) {
      if (p != NULL) {
        wincanvas = p;
      }
      else {
        if (maskc != NULL) {
          ag_ccanvas(maskc);
          free(maskc);
        }
        
        aw_unmuteparent(win, p);
        return;
      }
    }
    else {
      wincanvas = &win->c;
    }
    
    int fadesz = acfg()->fadeframes;
    
    if (fadesz > 0) {
      //-- Current Canvas
      CANVAS cbg;
      ag_canvas(&cbg, agw(), agh());
      ag_draw(&cbg, agc(), 0, 0);
      int xc    = w / 2;
      int yc    = h / 2;
      int i;
      CANVAS * tmpb = (CANVAS *) malloc(sizeof(CANVAS) * fadesz);
      memset(tmpb, 0, sizeof(CANVAS)*fadesz);
      
      for (i = 1; i <= fadesz; i++) {
        /* Calculating Scale */
        byte scale  = (i * 0xff) / fadesz;
        scale = (scale * (0x200 - scale)) >> 8;
        byte scale2 = ((scale * 0x80) >> 8) + 0x80;
        int wtarget = (w * scale2) >> 8;
        int htarget = (h * scale2) >> 8;
        ag_canvas(&tmpb[i - 1], w, h);
        ag_draw_ex(&tmpb[i - 1], maskc, 0, 0, x, y, w, h);
        ag_draw_strecth_ex(
          &tmpb[i - 1],
          &cbg,
          xc - wtarget / 2, yc - htarget / 2, wtarget, htarget,
          x, y, w, h, scale, 1
        );
      }
      
      ag_ccanvas(&cbg);
      
      for (i = fadesz - 1; i >= 0; i--) {
        ag_draw(NULL, &tmpb[i], x, y);
        ag_ccanvas(&tmpb[i]);
        ag_sync();
      }
      
      free(tmpb);
    }
    
    ag_ccanvas(maskc);
    free(maskc);
    aw_unmuteparent(win, p);
  }
  else {
    aw_unmuteparent(win, p);
  }
}
void aw_textdialog(AWINDOWP parent, char * titlev, char * text, char * ok_text) {
  // actext(hWin,txtX,txtY,txtW,txtH,text,0);
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 1;
  //ag_rectopa(agc(),0,0,agw(),agh(),0x0000,180);
  CANVAS * maskc = aw_maskparent();
  ag_sync();
  char title[64];
  snprintf(title, 64, "%s", titlev);
  int pad   = agdp() * 4;
  int winW  = agw() - (pad * 2);
  int txtW  = winW - (pad * 2);
  int txtX  = pad * 2;
  int btnH  = agdp() * 20;
  int titW  = ag_txtwidth(title, 1);
  int titH  = ag_fontheight(1) + (pad * 2);
  PNGCANVASP winp = atheme("img.dialog");
  PNGCANVASP titp = atheme("img.dialog.titlebar");
  APNG9      winv;
  APNG9      titv;
  int vtitY = -1;
  int vpadB = pad;
  int vimgX = pad * 2;
  
  if (titp != NULL) {
    if (apng9_calc(titp, &titv, 1)) {
      int tmptitH = titH - (pad * 2);
      titH        = tmptitH + (titv.t + titv.b);
      vtitY       = titv.t;
    }
  }
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      txtW = winW - (winv.l + winv.r);
      txtX = pad  + (winv.l);
      vimgX = pad  + (winv.l);
      vpadB = winv.b;
    }
  }
  
  byte imgE = 0;
  int imgW = 0;
  int imgH = 0;
  int txtH    = agh() / 2;
  int infH    = txtH;
  //-- Calculate Window Size & Position
  int winH    = titH + infH + btnH + (pad * 2) + vpadB;
  int winX    = pad;
  int winY    = (agh() / 2) - (winH / 2);
  //-- Calculate Title Size & Position
  int titX    = (agw() / 2) - (titW / 2);
  int titY    = winY + pad;
  
  if (vtitY != -1) {
    titY = winY + vtitY;
  }
  
  //-- Calculate Text Size & Position
  int infY    = winY + titH + pad;
  int txtY    = infY;
  //-- Calculate Button Size & Position
  int btnW    = winW / 2;
  int btnY    = infY + infH + pad;
  int btnX    = (agw() / 2) - (btnW / 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  
  //-- Draw Window
  if (!atheme_draw("img.dialog", &alertbg, winX, winY, winW, winH)) {
    ag_roundgrad(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, (acfg_var.roundsz * agdp()) + 1);
    ag_roundgrad(&alertbg, winX, winY, winW, winH, acfg_var.dialogbg, acfg_var.dialogbg_g, acfg_var.roundsz * agdp());
  }
  
  //-- Draw Title
  if (!atheme_draw("img.dialog.titlebar", &alertbg, winX, winY, winW, titH)) {
    ag_roundgrad_ex(&alertbg, winX, winY, winW, titH, acfg_var.dlgtitlebg, acfg_var.dlgtitlebg_g, acfg_var.roundsz * agdp(), 1, 1, 0, 0);
  }
  
  ag_textf(&alertbg, titW, titX + 1, titY + 1, title, acfg_var.dlgtitlebg_g, 1);
  ag_text(&alertbg, titW, titX, titY, title, acfg_var.dlgtitlefg, 1);
  AWINDOWP hWin   = aw(&alertbg);
  actext(hWin, txtX, txtY, txtW, txtH, text, 0);
  ACONTROLP okbtn = acbutton(hWin, btnX, btnY, btnW, btnH, (ok_text == NULL ? alang_get("ok") : ok_text), 0, 5);
  /*aw_show(hWin);
  aw_setfocus(hWin,okbtn);*/
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, okbtn);
  byte ondispatch = 1;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 5:
        ondispatch = 0;
        break;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  //aw_unmuteparent(parent,tmpc);
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
}
void aw_alert(AWINDOWP parent, char * titlev, char * textv, PNGCANVASP ap, char * ok_text) {
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 1;
  //ag_rectopa(agc(),0,0,agw(),agh(),0x0000,180);
  CANVAS * maskc = aw_maskparent();
  ag_sync();
  char title[32];
  char text[512];
  snprintf(title, 32, "%s", titlev);
  snprintf(text, 512, "%s", textv);
  int pad   = agdp() * 4;
  int winW  = agw() - (pad * 2);
  int txtW  = winW - (pad * 2);
  int txtX  = pad * 2;
  int btnH  = agdp() * 20;
  int titW  = ag_txtwidth(title, 1);
  int titH  = ag_fontheight(1) + (pad * 2);
  PNGCANVASP winp = atheme("img.dialog");
  PNGCANVASP titp = atheme("img.dialog.titlebar");
  APNG9      winv;
  APNG9      titv;
  int vtitY = -1;
  int vpadB = -1;
  int vimgX = pad * 2;
  
  if (titp != NULL) {
    if (apng9_calc(titp, &titv, 1)) {
      int tmptitH = titH - (pad * 2);
      titH        = tmptitH + (titv.t + titv.b);
      vtitY       = titv.t;
    }
  }
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      txtW = winW - (winv.l + winv.r);
      txtX = pad  + (winv.l);
      vimgX = pad  + (winv.l);
      vpadB = winv.b;
    }
  }
  
  printf("PAD: %i, %i -- %i, %i\n", titv.t, titv.b, winv.l, winv.r);
  //-- Load Icon
  byte imgE = 0;
  int imgW = 0;
  int imgH = 0;
  
  if (ap != NULL) {
    imgE      = 1;
    /*imgW      = min(ap->w,agdp()*30);
    imgH      = min(ap->h,agdp()*30);*/
    imgW      = agdp() * 24;
    imgH      = agdp() * 24;
    int imgA  = pad + imgW;
    txtX     += imgA;
    txtW     -= imgA;
  }
  
  int txtH    = ag_txtheight(txtW, text, 0);
  int infH    = ((imgE) && (txtH < imgH)) ? imgH : txtH;
  //-- Calculate Window Size & Position
  int winH    = titH + infH + btnH + (pad * 3);
  
  if (vpadB != -1) {
    winH    = titH + infH + btnH + (pad * 2) + vpadB;
  }
  
  int winX    = pad;
  int winY    = (agh() / 2) - (winH / 2);
  //-- Calculate Title Size & Position
  int titX    = (agw() / 2) - (titW / 2);
  int titY    = winY + pad;
  
  if (vtitY != -1) {
    titY = winY + vtitY;
  }
  
  //-- Calculate Text Size & Position
  int infY    = winY + titH + pad;
  int txtY    = infY + ((infH - txtH) / 2);
  int imgY    = infY;
  //-- Calculate Button Size & Position
  int btnW    = winW / 2;
  int btnY    = infY + infH + pad;
  int btnX    = (agw() / 2) - (btnW / 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  
  //-- Draw Window
  if (!atheme_draw("img.dialog", &alertbg, winX, winY, winW, winH)) {
    ag_roundgrad(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, (acfg_var.roundsz * agdp()) + 1);
    ag_roundgrad(&alertbg, winX, winY, winW, winH, acfg_var.dialogbg, acfg_var.dialogbg_g, acfg_var.roundsz * agdp());
  }
  
  //-- Draw Title
  if (!atheme_draw("img.dialog.titlebar", &alertbg, winX, winY, winW, titH)) {
    ag_roundgrad_ex(&alertbg, winX, winY, winW, titH, acfg_var.dlgtitlebg, acfg_var.dlgtitlebg_g, acfg_var.roundsz * agdp(), 1, 1, 0, 0);
  }
  
  ag_textf(&alertbg, titW, titX + 1, titY + 1, title, acfg_var.dlgtitlebg_g, 1);
  ag_text(&alertbg, titW, titX, titY, title, acfg_var.dlgtitlefg, 1);
  
  //-- Draw Image
  if (imgE) {
    // apng_draw_ex(&alertbg,ap,vimgX,imgY,0,0,imgW,imgH);
    apng_stretch(&alertbg, ap, vimgX, imgY, imgW, imgH, 0, 0, ap->w, ap->h);
  }
  
  //-- Draw Text
  ag_textf(&alertbg, txtW, txtX + 1, txtY + 1, text, acfg_var.dialogbg, 0);
  ag_text(&alertbg, txtW, txtX, txtY, text, acfg_var.dialogfg, 0);
  AWINDOWP hWin   = aw(&alertbg);
  acbutton(hWin, btnX, btnY, btnW, btnH, (ok_text == NULL ? alang_get("ok") : ok_text), 0, 5);
  // aw_show(hWin);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  byte ondispatch = 1;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 5:
        ondispatch = 0;
        break;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  //aw_unmuteparent(parent,tmpc);
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
}
byte aw_confirm(AWINDOWP parent, char * titlev, char * textv, PNGCANVASP ap, char * yes_text, char * no_text) {
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 1;
  //ag_rectopa(agc(),0,0,agw(),agh(),0x0000,180);
  CANVAS * maskc = aw_maskparent();
  ag_sync();
  char title[64];
  char text[512];
  snprintf(title, 64, "%s", titlev);
  snprintf(text, 512, "%s", textv);
  int pad   = agdp() * 4;
  int winW  = agw() - (pad * 2);
  int txtW  = winW - (pad * 2);
  int txtX  = pad * 2;
  int btnH  = agdp() * 20;
  int titW  = ag_txtwidth(title, 1);
  int titH  = ag_fontheight(1) + (pad * 2);
  PNGCANVASP winp = atheme("img.dialog");
  PNGCANVASP titp = atheme("img.dialog.titlebar");
  APNG9      winv;
  APNG9      titv;
  int vtitY = -1;
  int vpadB = -1;
  int vimgX = pad * 2;
  
  if (titp != NULL) {
    if (apng9_calc(titp, &titv, 1)) {
      int tmptitH = titH - (pad * 2);
      titH        = tmptitH + (titv.t + titv.b);
      vtitY       = titv.t;
    }
  }
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      txtW = winW - (winv.l + winv.r);
      txtX = pad  + (winv.l);
      vimgX = pad  + (winv.l);
      vpadB = winv.b;
    }
  }
  
  //-- Load Icon
  byte imgE = 0;
  int imgW = 0;
  int imgH = 0;
  
  if (ap != NULL) {
    imgE      = 1;
    imgW      = agdp() * 24;
    imgH      = agdp() * 24;
    /*
    imgW      = min(ap->w,agdp()*30);
    imgH      = min(ap->h,agdp()*30);
    */
    int imgA  = pad + imgW;
    txtX     += imgA;
    txtW     -= imgA;
  }
  
  int txtH    = ag_txtheight(txtW, text, 0);
  int infH    = ((imgE) && (txtH < imgH)) ? imgH : txtH;
  //-- Calculate Window Size & Position
  int winH    = titH + infH + btnH + (pad * 3);
  
  if (vpadB != -1) {
    winH    = titH + infH + btnH + (pad * 2) + vpadB;
  }
  
  int winX    = pad;
  int winY    = (agh() / 2) - (winH / 2);
  //-- Calculate Title Size & Position
  int titX    = (agw() / 2) - (titW / 2);
  int titY    = winY + pad;
  
  if (vtitY != -1) {
    titY = winY + vtitY;
  }
  
  //-- Calculate Text Size & Position
  int infY    = winY + titH + pad;
  int txtY    = infY + ((infH - txtH) / 2);
  int imgY    = infY;
  //-- Calculate Button Size & Position
  int btnW    = (txtW / 2) - (pad / 2);
  int btnY    = infY + infH + pad;
  int btnX    = txtX;
  int btnX2   = txtX + (txtW / 2) + (pad / 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  
  //-- Draw Window
  if (!atheme_draw("img.dialog", &alertbg, winX, winY, winW, winH)) {
    ag_roundgrad(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, (acfg_var.roundsz * agdp()) + 1);
    ag_roundgrad(&alertbg, winX, winY, winW, winH, acfg_var.dialogbg, acfg_var.dialogbg_g, acfg_var.roundsz * agdp());
  }
  
  //-- Draw Title
  if (!atheme_draw("img.dialog.titlebar", &alertbg, winX, winY, winW, titH)) {
    ag_roundgrad_ex(&alertbg, winX, winY, winW, titH, acfg_var.dlgtitlebg, acfg_var.dlgtitlebg_g, acfg_var.roundsz * agdp(), 1, 1, 0, 0);
  }
  
  ag_textf(&alertbg, titW, titX + 1, titY + 1, title, acfg_var.dlgtitlebg_g, 1);
  ag_text(&alertbg, titW, titX, titY, title, acfg_var.dlgtitlefg, 1);
  
  //-- Draw Image
  if (imgE) {
    // apng_draw_ex(&alertbg,ap,vimgX,imgY,0,0,imgW,imgH);
    apng_stretch(&alertbg, ap, vimgX, imgY, imgW, imgH, 0, 0, ap->w, ap->h);
    // apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&alertbg, txtW, txtX + 1, txtY + 1, text, acfg_var.dialogbg, 0);
  ag_text(&alertbg, txtW, txtX, txtY, text, acfg_var.dialogfg, 0);
  AWINDOWP hWin   = aw(&alertbg);
  acbutton(hWin, btnX, btnY, btnW, btnH, (yes_text == NULL ? alang_get("yes") : yes_text), 0, 6);
  acbutton(hWin, btnX2, btnY, btnW, btnH, (no_text == NULL ? alang_get("no") : no_text), 0, 5);
  //aw_show(hWin);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  byte ondispatch = 1;
  byte res = 0;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 6:
        res = 1;
        ondispatch = 0;
        break;
        
      case 5:
        ondispatch = 0;
        break;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  //aw_unmuteparent(parent,tmpc);
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
  return res;
}
byte aw_calibdraw(CANVAS * c,
                  int id, int * xpos, int * ypos, int * xtch, int * ytch) {
  return 0;
}

byte aw_calibmatrix(AW_CALIBPOINTP displayPtr, AW_CALIBPOINTP screenPtr, AW_CALIBMATRIXP matrixPtr) {
  byte retValue = 1;
  matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
                       
  if ( matrixPtr->Divider == 0 ) {
    retValue = 0;
  }
  else {
    matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
    matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) -
                    ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;
    matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
    matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
    matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) -
                    ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
    matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;
  }
  
  return retValue;
} /* end of setCalibrationMatrix() */
byte aw_calibtools(AWINDOWP parent) {
  aw_alert(parent,
           "No Need Calibration",
           "This version don't need any calibration data...",
           aui_icons(37),
           NULL);
  return 1;
}
byte aw_calibtools_old(AWINDOWP parent) {
  int USE_HACK = aw_confirm(
                   parent,
                   "Use alternative touch",
                   "Do you want to use alternative touch?\n  Only use if the default method does not work.\n\nPress the volume keys to select Yes or No.",
                   aui_icons(39),
                   alang_get("no"),
                   alang_get("yes")
                 );
  int current_hack = atouch_gethack();
  int new_hack_val = 0;
  
  if (!USE_HACK) {
    atouch_sethack(20);
    new_hack_val = 20;
  }
  else {
    atouch_sethack(0);
  }
  
  //-- Set Mask
  CANVAS * tmpc = aw_muteparent(parent);
  on_dialog_window = 1;
  ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 220);
  ag_sync();
  byte isvalid = 0;
  //-- Initializing Canvas
  CANVAS ccv;
  ag_canvas(&ccv, agw(), agh());
  ag_blur(&ccv, agc(), agdp() * 2);
  atouch_plaincalibrate();
  int dp10    = agdp() * 20;
  int xpos[3] = {
    dp10, agw() - dp10, agw() / 2
  };
  int ypos[3] = {
    dp10, agh() / 2, agh() - dp10
  };
  int xtch[3] = { 0, 0, 0 };
  int ytch[3] = { 0, 0, 0 };
  AW_CALIBPOINT dPoint[3];
  AW_CALIBPOINT tPoint[3];
  int i;
  
  for (i = 0; i < 3; i++) {
    if (!aw_calibdraw(&ccv, i, xpos, ypos, xtch, ytch)) {
      goto doneit;
    }
    
    dPoint[i].x = ((float) xpos[i]) / 2;
    dPoint[i].y = ((float) ypos[i]) / 2;
    tPoint[i].x = ((float) xtch[i]) / 2;
    tPoint[i].y = ((float) ytch[i]) / 2;
  }
  
  AW_CALIBMATRIX matrix;
  
  if (aw_calibmatrix(dPoint, tPoint, &matrix)) {
    atouch_matrix_calibrate(&matrix);
    
    if (new_hack_val) {
      aw_calibdraw(&ccv, -2, &new_hack_val, ypos, xtch, ytch);
    }
    else {
      aw_calibdraw(&ccv, -1, xpos, ypos, xtch, ytch);
    }
    
    isvalid       = 1;
  }
  else {
    aw_alert(parent,
             "Calibrated Data",
             "Calibrated data not valid, please try again...",
             aui_icons(37),
             NULL);
  }
  
doneit:
  ag_ccanvas(&ccv);
  on_dialog_window = 0;
  aw_unmuteparent(parent, tmpc);
  byte dont_restore_caldata = 0;
  
  if (isvalid) {
    //char msg_calib[256];
    //snprintf(msg_calib,256,"%s\n\nDo you want to use the current calibrated data in the current process?",data_calib);
    dont_restore_caldata = 1;
    /*aw_confirm(
      parent,
      "Calibration Data",
      msg_calib,
      aui_icons(9),
      NULL,
      NULL
    );*/
  }
  
  if (!dont_restore_caldata) {
    atouch_sethack(current_hack);
    atouch_restorecalibrate();
  }
  
  return isvalid;
}
byte aw_menu(AWINDOWP parent, char * title, AWMENUITEMP mi, int n) {
  if (n < 1) {
    return 0;
  }
  
  CANVAS * tmpc = aw_muteparent(parent);
  on_dialog_window = 2;
  ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 180);
  ag_sync();
  int titH  = ag_fontheight(1) + (agdp() * 2);
  
  if (title == NULL) {
    titH = 0;
  }
  
  int pad   = agdp() * 2;
  int menuw = agw() - (pad * 2);
  int itemw = floor(menuw / 2);
  int itemh = agdp() * 22 + ag_fontheight(0);
  int winh  = (ceil(((float) n) / 2.0) * itemh) + (pad * 2);
  int wsph  = (agdp() * 4) + titH;
  int winy  = agh() - winh;
  int wspy  = winy - wsph;
  int itemy = winy + pad;
  int itemx1 = pad;
  int itemx2 = pad + itemw;
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  //-- Draw Win Separator
  ag_roundgrad_ex(&alertbg, 0, wspy, agw(), wsph, acfg()->navbg, acfg()->navbg_g, 0, 0, 0, 0, 0);
  
  if (title != NULL) {
    int title_w = ag_txtwidth(title, 1);
    int title_x = agw() / 2 - title_w / 2;
    ag_textf(&alertbg, agw(), title_x + 1, wspy + (agdp() * 3) + 1, title, acfg_var.navbg, 1);
    ag_text(&alertbg, agw(), title_x, wspy + (agdp() * 3), title, acfg_var.winfg, 1);
  }
  
  //-- Draw Main Window
  ag_roundgrad_ex(&alertbg, 0, winy, agw(), winh, acfg()->navbg, acfg()->navbg_g, 0, 0, 0, 0, 0);
  //-- Create Window
  AWINDOWP hWin   = aw(&alertbg);
  //-- Items
  int i;
  int div2 = (n % 2 == 0) ? 1 : 0;
  
  for (i = 0; i < n; i++) {
    if ((i == 0) && (!div2)) {
      imgbtn(hWin, itemx1, itemy, itemw * 2, itemh, mi[i].icon, mi[i].title, 1, 11 + i);
    }
    else {
      imgbtn(hWin, ((i % 2 != div2) ? itemx1 : itemx2), itemy, itemw, itemh, mi[i].icon, mi[i].title, 1, 11 + i);
    }
    
    if (i % 2 == div2) {
      itemy += itemh;
    }
  }
  
  // aw_show(hWin);
  aw_show_ex(hWin, 1, wspy, NULL);
  byte ondispatch = 1;
  byte res        = 0;
  
  while (ondispatch) {
    dword msg = aw_dispatch_ex(hWin, wspy);
    byte msgm = aw_gm(msg);
    
    if (msgm == 5) {
      ondispatch = 0;
    }
    else if (msgm > 10) {
      res = msgm - 10;
      ondispatch = 0;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  aw_unmuteparent(parent, tmpc);
  return res;
}
char * aw_ime(AWINDOWP parent, char * current_txt, char * title) {
  //-- Init Dialog Window
  CANVAS * tmpc     = aw_muteparent(parent);
  on_dialog_window  = 2;
  ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 180);
  ag_sync();
  //-- Calculate Sizes
  int vpad  = 1;
  int mpad  = 2;
  int pad   = agdp() * 2;
  int imeW  = agw();
  int keyH  = agdp() * 26;
  int keyW  = floor(agw() / 10);
  int imeH  = keyH * 4;
  int wimH  = imeH + pad;
  int wimY  = agh() - wimH;
  int keyY  = wimY + pad;
  int txtH  = agdp() * 36;
  int txtY  = wimY - txtH;
  int txtW  = agw() - ((agdp() * 48) + (pad * 3));
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  //-- Draw Ime Holder
  // ag_roundgrad_ex(&alertbg,0,wimY,agw(),wimH,acfg()->navbg,acfg()->navbg_g,0,0,0,0,0);
  // ag_rect(&alertbg,0,wimY,agw(),1,ag_calculatealpha(acfg()->border,acfg()->border_g,130));
  ag_roundgrad(&alertbg, 0, txtY, agw(), txtH, acfg()->navbg_g, acfg()->navbg, 0);
  ag_rect(&alertbg, 0, wimY - 1, agw(), 1, acfg()->navbg);
  ag_rect(&alertbg, 0, txtY, agw(), 1, ag_calculatealpha(acfg()->border, acfg()->border_g, 130));
  ag_roundgrad_ex(&alertbg, pad, txtY + pad, txtW, txtH - (pad * 2), acfg()->border, acfg()->border_g, pad, 1, 1, 1, 1);
  ag_roundgrad_ex(&alertbg, pad + 1, txtY + pad + 1, txtW - 2, txtH - ((pad * 2) + 2), acfg()->textbg, acfg()->textbg, pad - 1, 1, 1, 1, 1);
  //-- Draw Title
  int titH  = ag_fontheight(1) + (pad * 4);
  int titY  = txtY - titH;
  int titW  = ag_txtwidth(title, 1);
  int titX    = (agw() / 2) - (titW / 2);
  ag_roundgrad(&alertbg, 0, titY, agw(), titH, acfg_var.navbg, acfg_var.navbg_g, 0);
  ag_rect(&alertbg, 0, titY, agw(), 1, acfg()->border);
  ag_textf(&alertbg, titW, titX + 1, titY + (pad * 2) + 1, title, acfg_var.navbg, 1);
  ag_text(&alertbg, titW, titX, titY + (pad * 2), title, acfg_var.controlfg, 1);
  //-- Create Window
  AWINDOWP hWin   = aw(&alertbg);
  //-- IME Controls
  byte onShift        = 0;
  byte onChars        = 0;
  //-- Edit done & cancel button
  int btnv_s = agdp() * 24;
  int btnv_x = agw() - (btnv_s + pad);
  int btnv_h = agdp() * 32;
  imgbtn(hWin, btnv_x - (btnv_s + agdp()), wimY - (btnv_h + pad), btnv_s, btnv_h, aui_icons(0), alang_get("cancel"), 0, 5);
  imgbtn(hWin, btnv_x,                  wimY - (btnv_h + pad), btnv_s, btnv_h, aui_icons(33), alang_get("done"), 0,    96);
  //-- Edit Box
  ACONTROLP editbox = acedit(
                        hWin, (agdp() * 4), txtY + (agdp() * 4), txtW - (agdp() * 8), txtH - (agdp() * 8), current_txt, 1
                      );
  //-- IME
  ACONTROLP imebox = acime(
                       hWin,
                       0, wimY, agw(), wimH,
                       editbox
                     );
  //-- Show Window
  //aw_show(hWin);
  aw_show_ex(hWin, 1, titY, editbox);
  // aw_setfocus(hWin,editbox);
  aw_draw(hWin);
  byte ondispatch = 1;
  byte res        = 0;
  char * returntxt = NULL;
  
  //-- Dispatch Message
  while (ondispatch) {
    dword msg = aw_dispatch_ex(hWin, 0);
    byte msgm = aw_gm(msg);
    
    if (msgm == 5) {
      ondispatch = 0;
    }
    else if (msgm == 96) {
      char * ret = acedit_gettext(editbox);
      
      if (ret != NULL) {
        returntxt = strdup(ret);
      }
      
      ondispatch = 0;
    }
  }
  
  //-- Cleanup
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  aw_unmuteparent(parent, tmpc);
  ag_setcaret(0, 0, 0);
  return returntxt;
}
byte aw_multiconfirm(AWINDOWP parent, char * titlev, char * textv, PNGCANVASP ap, AWMENUITEMP mi, int n) {
  return aw_multiconfirm_ex(parent, titlev, textv, ap, mi, n, 1);
}
byte aw_multiconfirm_ex(AWINDOWP parent, char * titlev, char * textv, PNGCANVASP ap, AWMENUITEMP mi, int n, int first_sel) {
  CANVAS * tmpc = aw_muteparent(parent);
  //-- Set Mask
  on_dialog_window = 1;
  //ag_rectopa(agc(),0,0,agw(),agh(),0x0000,180);
  CANVAS * maskc = aw_maskparent();
  ag_sync();
  char title[64];
  char text[512];
  snprintf(title, 64, "%s", titlev);
  snprintf(text, 512, "%s", textv);
  int pad   = agdp() * 4;
  int winW  = agw() - (pad * 2);
  int txtW  = winW - (pad * 2);
  int txtX  = pad * 2;
  int titW  = ag_txtwidth(title, 1);
  int titH  = ag_fontheight(1) + (pad * 2);
  int btnHH = agdp() * 26;
  int btnH  = (ceil(((float) n) / 2.0) * btnHH);
  PNGCANVASP winp = atheme("img.dialog");
  PNGCANVASP titp = atheme("img.dialog.titlebar");
  APNG9      winv;
  APNG9      titv;
  int vtitY = -1;
  int vpadB = -1;
  int vimgX = pad * 2;
  
  if (titp != NULL) {
    if (apng9_calc(titp, &titv, 1)) {
      int tmptitH = titH - (pad * 2);
      titH        = tmptitH + (titv.t + titv.b);
      vtitY       = titv.t;
    }
  }
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      txtW = winW - (winv.l + winv.r);
      txtX = pad  + (winv.l);
      vimgX = pad  + (winv.l);
      vpadB = winv.b;
    }
  }
  
  //-- Load Icon
  byte imgE = 0;
  int imgW = 0;
  int imgH = 0;
  
  if (ap != NULL) {
    imgE      = 1;
    imgW      = agdp() * 24;
    imgH      = agdp() * 24;
    int imgA  = pad + imgW;
    txtX     += imgA;
    txtW     -= imgA;
  }
  
  int txtH    = ag_txtheight(txtW, text, 0);
  int infH    = ((imgE) && (txtH < imgH)) ? imgH : txtH;
  //-- Calculate Window Size & Position
  int winH    = titH + infH + btnH + (pad * 3);
  
  if (vpadB != -1) {
    winH    = titH + infH + btnH + (pad * 2) + vpadB;
  }
  
  int winX    = pad;
  int winY    = (agh() / 2) - (winH / 2);
  //-- Calculate Title Size & Position
  int titX    = (agw() / 2) - (titW / 2);
  int titY    = winY + pad;
  
  if (vtitY != -1) {
    titY = winY + vtitY;
  }
  
  //-- Calculate Text Size & Position
  int infY    = winY + titH + pad;
  int txtY    = infY + ((infH - txtH) / 2);
  int imgY    = infY;
  //-- Calculate Button Size & Position
  int dp1p2   = ceil(agdp() / 2.0);
  int btnW2   = winW - (pad * 2);
  int btnW    = (btnW2 / 2) - dp1p2;
  int btnY    = infY + infH + pad;
  int btnX    = pad * 2;
  int btnX2   = btnX + btnW + (dp1p2 * 2);
  //-- Initializing Canvas
  CANVAS alertbg;
  ag_canvas(&alertbg, agw(), agh());
  ag_draw(&alertbg, agc(), 0, 0);
  
  //-- Draw Window
  if (!atheme_draw("img.dialog", &alertbg, winX, winY, winW, winH)) {
    ag_roundgrad(&alertbg, winX - 1, winY - 1, winW + 2, winH + 2, acfg_var.border, acfg_var.border_g, (acfg_var.roundsz * agdp()) + 1);
    ag_roundgrad(&alertbg, winX, winY, winW, winH, acfg_var.dialogbg, acfg_var.dialogbg_g, acfg_var.roundsz * agdp());
  }
  
  //-- Draw Title
  if (!atheme_draw("img.dialog.titlebar", &alertbg, winX, winY, winW, titH)) {
    ag_roundgrad_ex(&alertbg, winX, winY, winW, titH, acfg_var.dlgtitlebg, acfg_var.dlgtitlebg_g, acfg_var.roundsz * agdp(), 1, 1, 0, 0);
  }
  
  ag_textf(&alertbg, titW, titX + 1, titY + 1, title, acfg_var.dlgtitlebg_g, 1);
  ag_text(&alertbg, titW, titX, titY, title, acfg_var.dlgtitlefg, 1);
  
  //-- Draw Image
  if (imgE) {
    // apng_draw_ex(&alertbg,ap,vimgX,imgY,0,0,imgW,imgH);
    apng_stretch(&alertbg, ap, vimgX, imgY, imgW, imgH, 0, 0, ap->w, ap->h);
    // apng_close(&ap);
  }
  
  //-- Draw Text
  ag_textf(&alertbg, txtW, txtX + 1, txtY + 1, text, acfg_var.dialogbg, 0);
  ag_text(&alertbg, txtW, txtX, txtY, text, acfg_var.dialogfg, 0);
  AWINDOWP hWin   = aw(&alertbg);
  //-- Items
  int i;
  int div2 = (n % 2 == 0) ? 1 : 0;
  int itemy = btnY + agdp();
  ACONTROLP focused_btn = NULL;
  
  for (i = 0; i < n; i++) {
    ACONTROLP curctl = NULL;
    
    if ((i == 0) && (!div2)) {
      curctl = imgbtn(hWin, btnX, itemy, txtW, btnHH - agdp(), mi[i].icon, mi[i].title, 3, 10 + i);
    }
    else {
      curctl = imgbtn(hWin, ((i % 2 != div2) ? btnX : btnX2), itemy, btnW, btnHH - agdp(), mi[i].icon, mi[i].title, 3, 10 + i);
    }
    
    if (first_sel == i) {
      focused_btn = curctl;
    }
    
    if (i % 2 == div2) {
      itemy += btnHH;
    }
  }
  
  if (focused_btn) {
    //aw_setfocus(hWin,focused_btn);
    //aw_show_ex(hWin,0,0,focused_btn);
    aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, focused_btn);
  }
  else {
    //aw_show(hWin);
    aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  }
  
  byte ondispatch = 1;
  byte res        = 0;
  
  while (ondispatch) {
    dword msg = aw_dispatch(hWin);
    byte msgm = aw_gm(msg);
    
    if (msgm >= 10) {
      res = msgm - 10;
      ondispatch = 0;
    }
  }
  
  aw_destroy(hWin);
  ag_ccanvas(&alertbg);
  on_dialog_window = 0;
  //aw_unmuteparent(parent,tmpc);
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
  return res;
}