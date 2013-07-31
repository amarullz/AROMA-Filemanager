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
 * AROMA UI: CONSOLE
 *
 */
#include "../aroma.h"
/* Internal representation of the screen */

#define ESC_BUF_SIZ   256
#define ESC_ARG_SIZ   16
#define BETWEEN(x, a, b)  ((a) <= (x) && (x) <= (b))
#define DEFAULT(a, b)     (a) = (a) ? (a) : (b)
#define LIMIT(x, a, b)    (x) = (x) < (a) ? (a) : (x) > (b) ? (b) : (x)

enum escape_state {
  ESC_START      = 1,
  ESC_CSI        = 2,
  ESC_OSC        = 4,
  ESC_TITLE      = 8,
  ESC_ALTCHARSET = 16
};

typedef struct {
  char  buf[ESC_BUF_SIZ];   /* raw string */
  int   len;                /* raw string length */
  char  priv;
  int   arg[ESC_ARG_SIZ];
  int   narg;               /* nb of args */
  char  mode;
} AESCAPE, * AESCAPEP;

typedef struct {
  CANVAS    cv;
  char  **  history;
  byte      active;
  byte      iscursor;
  
  int       cv_x;
  int       cv_y;
  
  int       fw;     /* Font Width */
  int       fh;     /* Font Height */
  
  int       col;    /* Column Size */
  int       row;    /* Line Size */
  
  int       cx;
  int       cy;
  
  
  /* CONSOLE DATA */
  AESCAPE   escseq;
  int       esc;
  int       savex;  /* Save X */
  int       savey;  /* Save X */
  int       bg;     /* Background */
  int       fg;     /* Foreground */
  int       def_bg;
  int       def_fg;
  byte      bold;
  byte      underline;
  byte      reserve;
  byte      nextwrap;
  byte      clrf;
  word      colorset[256];
  byte      beep;
} ACONSOLE, * ACONSOLEP;

/*
  d->bg=d->def_bg;
  d->fg=d->def_fg;
  d->bold=0;
  d->underline=0;
*/
/** CONSOLE HANDLER **/
int aconsole_isescape(ACONTROLP ctl) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  return d->esc;
}
void aconsole_initcolors(ACONSOLEP d) {
  int i, r, g, b;
  byte red, green, blue;
  /* 16colorset */
  d->colorset[0] = 0x0000;
  d->colorset[1] = ag_rgb(0xFF, 0, 0);
  d->colorset[2] = ag_rgb(0, 0xFF, 0);
  d->colorset[3] = ag_rgb(0xFF, 0xFF, 0);
  d->colorset[4] = ag_rgb(0, 0, 0xFF);
  d->colorset[5] = ag_rgb(0xFF, 0, 0xFF);
  d->colorset[6] = ag_rgb(0, 0xFF, 0xFF);
  d->colorset[7] = ag_rgb(0xCC, 0xCC, 0xCC);
  d->colorset[8] = ag_rgb(0x66, 0x66, 0x66);
  d->colorset[9] = ag_rgb(0xFF, 0x5C, 0x5C);
  d->colorset[10] = ag_rgb(0x5C, 0xFF, 0x5C);
  d->colorset[11] = ag_rgb(0xFF, 0xFF, 0x5C);
  d->colorset[12] = ag_rgb(0x5C, 0x5C, 0xFF);
  d->colorset[13] = ag_rgb(0xFF, 0x5C, 0xFF);
  d->colorset[14] = ag_rgb(0x5C, 0xFF, 0xFF);
  d->colorset[15] = 0xffff;
  
  /* load colors [16-255] ; same colors as xterm */
  for (i = 16, r = 0; r < 6; r++) {
    for (g = 0; g < 6; g++) {
      for (b = 0; b < 6; b++) {
        red   = 42 * r;
        green = 42 * g;
        blue  = 42 * b;
        d->colorset[i] = ag_rgb(red, green, blue);
        i++;
      }
    }
  }
  
  for (r = 0; r < 24; r++, i++) {
    red = green = blue = 10 * r;
    d->colorset[i] = ag_rgb(red, green, blue);
  }
}
word getbg(ACONSOLEP d) {
  return d->colorset[(d->reserve ? d->fg : d->bg)];
}
word getfg(ACONSOLEP d) {
  return d->colorset[(d->reserve ? d->bg : d->fg)];
}
void csireset(ACONTROLP ctl) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  memset(&d->escseq, 0, sizeof(d->escseq));
}
void tclearregion(ACONTROLP ctl, int x1, int y1, int x2, int y2) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  int x, y, temp;
  
  if (x1 > x2) {
    temp = x1, x1 = x2, x2 = temp;
  }
  
  if (y1 > y2) {
    temp = y1, y1 = y2, y2 = temp;
  }
  
  LIMIT(x1, 0, d->col - 1);
  LIMIT(x2, 0, d->col - 1);
  LIMIT(y1, 0, d->row - 1);
  LIMIT(y2, 0, d->row - 1);
  /* Draw */
  ag_rect(
    &d->cv,
    x1 * d->fw,       y1 * d->fh,
    ((x2 - x1) + 1) * d->fw,  ((y2 - y1) + 1) * d->fh,
    getbg(d)
  );
}
void treset(ACONTROLP ctl) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  d->bg = d->def_bg;
  d->fg = d->def_fg;
  d->bold = 0;
  d->underline = 0;
  d->reserve = 0;
  d->cx = 0;
  d->cy = 0;
  d->clrf = 0;
  tclearregion(ctl, 0, 0, d->col - 1, d->row - 1);
}
void tscrolldown(ACONTROLP ctl, int orig, int n) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  LIMIT(n, 0, d->row - orig);
  int height = d->fh * n;
  ag_draw(&d->cv, &d->cv, 0, height);
  tclearregion(ctl, 0, orig, d->col - 1, orig + n - 1);
  //printf("SCROLLDN\n");
}
void tscrollup(ACONTROLP ctl, int orig, int n) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  LIMIT(n, 0, d->row - orig);
  int height = d->fh * n;
  ag_draw(&d->cv, &d->cv, 0, -height);
  tclearregion(ctl, 0, d->row - n, d->col - 1, d->row - 1);
  //printf("SCROLLUP\n");
}
void tmoveto(ACONTROLP ctl, int x, int y) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  LIMIT(x, 0, d->col - 1);
  LIMIT(y, 0, d->row - 1);
  d->cx = x;
  d->cy = y;
  d->nextwrap = 0;
}
void tnewline(ACONTROLP ctl, int first_col) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  int y = d->cy;
  
  if (y == d->row - 1) {
    tscrollup(ctl, 0, 1);
  }
  else {
    y++;
  }
  
  tmoveto(ctl, first_col ? 0 : d->cx, y);
}
void tsetchar(ACONTROLP ctl, char c) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  tclearregion(ctl, d->cx, d->cy, d->cx, d->cy);
  
  if (c != ' ') {
    ag_drawchar_ex2(
      &d->cv,
      d->cx * d->fw,
      d->cy * d->fh,
      c,
      getfg(d),
      2,
      d->underline,
      d->bold, 0, 0
    );
  }
}
void tdeletechar(ACONTROLP ctl, int n) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  int src   = d->cx + n;
  int dst   = d->cx;
  int size  = d->col - src;
  
  if (src >= d->col) {
    tclearregion(ctl, d->cx, d->cy, d->col - 1, d->cy);
    return;
  }
  
  ag_draw_ex(&d->cv, &d->cv,
             dst * d->fw,
             d->cy * d->fh,
             src * d->fw,
             d->cy * d->fh,
             size * d->fw,
             d->fh
            );
  tclearregion(ctl, d->col - n, d->cy, d->col - 1, d->cy);
  //printf("tdeletechar : %i, %i, %i, %i\n", d->col-n, d->cy, d->col-1, d->cy);
}
void tinsertblank(ACONTROLP ctl, int n) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  int src = d->cx;
  int dst = src + n;
  int size = d->col - dst;
  
  if (dst >= d->col) {
    tclearregion(ctl, d->cx, d->cy, d->col - 1, d->cy);
    return;
  }
  
  /* Move Create Temporary Canvas */
  CANVAS tmpcanvas;
  ag_canvas(&tmpcanvas, size * d->fw, d->fh);
  ag_draw_ex(
    &tmpcanvas, &d->cv,
    0, 0,
    src * d->fw, d->cy * d->fh,
    size * d->fw, d->fh
  );
  ag_draw(
    &d->cv, &tmpcanvas,
    dst * d->fw, d->cy * d->fh
  );
  ag_ccanvas(&tmpcanvas);
  /*
  ag_draw_ex(
    &d->cv,&d->cv,
    dst*d->fw,
    d->cy*d->fh,
    src*d->fw,
    d->cy*d->fh,
    size*d->fw,
    d->fh
  );
  */
  tclearregion(ctl, src, d->cy, dst - 1, d->cy);
  //printf("INSERT BLANK : %i, %i, %i, %i\n", src, d->cy, dst - 1, d->cy);
}
void tinsertblankline(ACONTROLP ctl, int n) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  
  if ((d->cy < 0) || (d->cy >= d->row)) {
    return;
  }
  
  tscrolldown(ctl, d->cy, n);
}
void tdeleteline(ACONTROLP ctl, int n) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  
  if ((d->cy < 0) || (d->cy >= d->row)) {
    return;
  }
  
  tscrollup(ctl, d->cy, n);
}
void tsetattr(ACONTROLP ctl, int * attr, int l) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  int i;
  
  for (i = 0; i < l; i++) {
    switch (attr[i]) {
      case 0: {
          d->bg = d->def_bg;
          d->fg = d->def_fg;
          d->bold = 0;
          d->underline = 0;
          d->reserve = 0;
        }
        break;
        
      case 1:
        d->bold = 1;
        break;
        
      case 4:
        d->underline = 1;
        break;
        
      case 7:
        d->reserve = 1;
        break;
        
      case 22:
        d->bold = 0;
        break;
        
      case 24:
        d->underline = 0;
        break;
        
      case 27:
        d->reserve = 0;
        break;
        
      case 38: {
          if ((i + 2 < l) && (attr[i + 1] == 5)) {
            i += 2;
            
            if (BETWEEN(attr[i], 0, 255)) {
              d->fg = attr[i];
            }
          }
        }
        break;
        
      case 39:
        d->fg = d->def_fg;
        break;
        
      case 48: {
          if (i + 2 < l && attr[i + 1] == 5) {
            i += 2;
            
            if (BETWEEN(attr[i], 0, 255)) {
              d->bg = attr[i];
            }
          }
        }
        break;
        
      case 49:
        d->bg = d->def_bg;
        break;
        
      default: {
          if (BETWEEN(attr[i], 30, 37)) {
            d->fg = attr[i] - 30;
          }
          else if (BETWEEN(attr[i], 40, 47)) {
            d->bg = attr[i] - 40;
          }
          else if (BETWEEN(attr[i], 90, 97)) {
            d->fg = attr[i] - 90 + 8;
          }
          else if (BETWEEN(attr[i], 100, 107)) {
            d->fg = attr[i] - 100 + 8;
          }
        }
        break;
    }
  }
}
void tputtab(ACONTROLP  ctl) {
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  int space = 8 - d->cx % 8;
  tmoveto(ctl, d->cx + space, d->cy);
}
void csiparse(ACONTROLP  ctl) {
  ACONSOLEP  d = (ACONSOLEP) ctl->d;
  char * p = d->escseq.buf;
  d->escseq.narg = 0;
  
  if (*p == '?') {
    d->escseq.priv = 1, p++;
  }
  
  while (p < d->escseq.buf + d->escseq.len) {
    while (isdigit(*p)) {
      d->escseq.arg[d->escseq.narg] *= 10;
      d->escseq.arg[d->escseq.narg] += *p++ - '0'/*, noarg = 0 */;
    }
    
    if (*p == ';' && d->escseq.narg + 1 < ESC_ARG_SIZ) {
      d->escseq.narg++, p++;
    }
    else {
      d->escseq.mode = *p;
      d->escseq.narg++;
      return;
    }
  }
}
void csihandle(ACONTROLP  ctl) {
  ACONSOLEP  d = (ACONSOLEP) ctl->d;
  
  switch (d->escseq.mode) {
    case '@': /* ICH -- Insert <n> blank char */
      DEFAULT(d->escseq.arg[0], 1);
      tinsertblank(ctl, d->escseq.arg[0]);
      break;
      
    case 'A': /* CUU -- Cursor <n> Up */
    case 'e':
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, d->cx, d->cy - d->escseq.arg[0]);
      break;
      
    case 'B': /* CUD -- Cursor <n> Down */
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, d->cx, d->cy + d->escseq.arg[0]);
      break;
      
    case 'C': /* CUF -- Cursor <n> Forward */
    case 'a':
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, d->cx + d->escseq.arg[0], d->cy);
      break;
      
    case 'D': /* CUB -- Cursor <n> Backward */
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, d->cx - d->escseq.arg[0], d->cy);
      break;
      
    case 'E': /* CNL -- Cursor <n> Down and first col */
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, 0, d->cy + d->escseq.arg[0]);
      break;
      
    case 'F': /* CPL -- Cursor <n> Up and first col */
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, 0, d->cy - d->escseq.arg[0]);
      break;
      
    case 'G': /* CHA -- Move to <col> */
    case '`': /* XXX: HPA -- same? */
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, d->escseq.arg[0] - 1, d->cy);
      break;
      
    case 'H': /* CUP -- Move to <row> <col> */
    case 'f': /* XXX: HVP -- same? */
      DEFAULT(d->escseq.arg[0], 1);
      DEFAULT(d->escseq.arg[1], 1);
      tmoveto(ctl, d->escseq.arg[1] - 1, d->escseq.arg[0] - 1);
      break;
      
      /* XXX: (CSI n I) CHT -- Cursor Forward Tabulation <n> tab stops */
    case 'J': { /* ED -- Clear screen */
        switch (d->escseq.arg[0]) {
          case 0: { /* below */
              tclearregion(ctl, d->cx, d->cy, d->col - 1, d->cy);
              
              if (d->cy < d->row - 1) {
                tclearregion(ctl, 0, d->cy + 1, d->col - 1, d->row - 1);
              }
            }
            break;
            
          case 1: { /* above */
              if (d->cy > 1) {
                tclearregion(ctl, 0, 0, d->col - 1, d->cy - 1);
              }
              
              tclearregion(ctl, 0, d->cy, d->cx, d->cy);
            }
            break;
            
          case 2: /* all */
            tclearregion(ctl, 0, 0, d->col - 1, d->row - 1);
            break;
        }
      }
      break;
      
    case 'K': { /* EL -- Clear line */
        switch (d->escseq.arg[0]) {
          case 0: /* right */
            tclearregion(ctl, d->cx, d->cy, d->col - 1, d->cy);
            break;
            
          case 1: /* left */
            tclearregion(ctl, 0, d->cy, d->cx, d->cy);
            break;
            
          case 2: /* all */
            tclearregion(ctl, 0, d->cy, d->col - 1, d->cy);
            break;
        }
      }
      break;
      
    case 'S': { /* SU -- Scroll <n> line up */
        DEFAULT(d->escseq.arg[0], 1);
        tscrollup(ctl, 0, d->escseq.arg[0]);
      }
      break;
      
    case 'T': { /* SD -- Scroll <n> line down */
        DEFAULT(d->escseq.arg[0], 1);
        tscrolldown(ctl, 0, d->escseq.arg[0]);
      }
      break;
      
    case 'L': { /* IL -- Insert <n> blank lines */
        DEFAULT(d->escseq.arg[0], 1);
        tinsertblankline(ctl, d->escseq.arg[0]);
      }
      break;
      
    case 'l': { /* RM -- Reset Mode */
        if (d->escseq.priv) {
          switch (d->escseq.arg[0]) {
            case 5: /* DECSCNM -- Remove reverse video */
              d->reserve = 0;
              break;
              
            case 20:
              //printf("CLRF OFF\n");
              d->clrf = 0;
              break;
              
            case 1048: {
                d->cx = d->savex;
                d->cy = d->savey;
              }
              break;
          }
        }
      }
      break;
      
    case 'M': /* DL -- Delete <n> lines */
      DEFAULT(d->escseq.arg[0], 1);
      tdeleteline(ctl, d->escseq.arg[0]);
      break;
      
    case 'X': /* ECH -- Erase <n> char */
      DEFAULT(d->escseq.arg[0], 1);
      tclearregion(ctl, d->cx, d->cy, d->cx + d->escseq.arg[0], d->cy);
      break;
      
    case 'P': /* DCH -- Delete <n> char */
      DEFAULT(d->escseq.arg[0], 1);
      tdeletechar(ctl, d->escseq.arg[0]);
      break;
      
      /* XXX: (CSI n Z) CBT -- Cursor Backward Tabulation <n> tab stops */
    case 'd': /* VPA -- Move to <row> */
      DEFAULT(d->escseq.arg[0], 1);
      tmoveto(ctl, d->cx, d->escseq.arg[0] - 1);
      break;
      
    case 'h': { /* SM -- Set terminal mode */
        if (d->escseq.priv) {
          switch (d->escseq.arg[0]) {
            case 5: /* DECSCNM -- Reverve video */
              d->reserve = 1;
              break;
              
            case 20:
              //printf("CLRF ON\n");
              d->clrf = 1;
              break;
              
            case 1048:
              d->savex = d->cx;
              d->savey = d->cy;
              break;
          }
        }
      }
      break;
      
    case 'm': /* SGR -- Terminal attribute (color) */
      tsetattr(ctl, d->escseq.arg, d->escseq.narg);
      break;
      
    case 's': /* DECSC -- Save cursor position (ANSI.SYS) */
      d->savex = d->cx;
      d->savey = d->cy;
      break;
      
    case 'u': /* DECRC -- Restore cursor position (ANSI.SYS) */
      d->cx = d->savex;
      d->cy = d->savey;
      break;
  }
}


/** CONTROLS **/
void aconsole_setwindowsize(void * x, int fd) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  struct winsize w;
  w.ws_row = d->row;
  w.ws_col = d->col;
  w.ws_xpixel = w.ws_ypixel = 0;
  
  if (ioctl(fd, TIOCSWINSZ, &w) < 0) {
    //printf("\n\n::: CANNOT RESIZE WINDOW :::\n\n");
  }
}
dword aconsole_oninput(void * x, int action, ATEV * atev) {
  return 0;
}
void aconsole_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  ag_rect(pc,
          ctl->x, ctl->y, ctl->w, ctl->h, getbg(d)
         );
  ag_draw(pc, &d->cv, ctl->x + d->cv_x, ctl->y + d->cv_y);
  
  if (d->iscursor) {
    int x, y;
    int xp = d->cx * d->fw + ctl->x + d->cv_x;
    int yp = d->cy * d->fh + ctl->y + d->cv_y;
    word fgcl = (d->beep) ? ag_rgb(255, 50, 50) : getfg(d);
    word bgcl = getbg(d);
    byte ir = ag_r(fgcl);
    byte ig = ag_g(fgcl);
    byte ib = ag_b(fgcl);
    byte gr = 255 - ag_r(bgcl);
    byte gg = 255 - ag_r(bgcl);
    byte gb = 255 - ag_r(bgcl);
    
    for (y = 0; y < d->fh; y++) {
      for (x = 0; x < d->fw; x++) {
        word * p = agxy(pc, xp + x, yp + y);
        
        if (p) {
          byte r = (byte) (((int) (gr - ag_r(*p)) * ir) >> 8);
          byte g = (byte) (((int) (gg - ag_g(*p)) * ig) >> 8);
          byte b = (byte) (((int) (gb - ag_b(*p)) * ib) >> 8);
          *p = ag_rgb(r, g, b);
        }
      }
    }
    
    /*
    ag_rect(pc,
      d->cx*d->fw + ctl->x + d->cv_x,
      d->cy*d->fh + ctl->y + d->cv_y,
      d->fw,d->fh,
      getfg(d)
    );
    */
  }
  
  aw_draw(ctl->win);
}
static void * aconsole_drawer(void * cookie) {
  ACONTROLP   ctl = (ACONTROLP) cookie;
  ACONSOLEP   d  = (ACONSOLEP) ctl->d;
  
  while (d->active) {
    d->iscursor = d->iscursor ? 0 : 1;
    aconsole_ondraw(ctl);
    usleep(250000);
  }
  
  return NULL;
}
void aconsole_ondestroy(void * x) {
  ACONTROLP  ctl  = (ACONTROLP) x;
  ACONSOLEP  d    = (ACONSOLEP) ctl->d;
  d->active = 0;
  usleep(600000);
  ag_ccanvas(&d->cv);
  free(ctl->d);
}
void aconsole_onblur(void * x) {
}
byte aconsole_onfocus(void * x) {
  return 1;
}
void aconsole_escape_handler(ACONTROLP  ctl, char ascii) {
  ACONSOLEP  d    = (ACONSOLEP) ctl->d;
  
  if (d->esc & ESC_CSI) {
    d->escseq.buf[d->escseq.len++] = ascii;
    
    if (BETWEEN(ascii, 0x40, 0x7E) || d->escseq.len >= ESC_BUF_SIZ) {
      d->esc = 0;
      csiparse(ctl);
      csihandle(ctl);
    }
  }
  else if (d->esc & ESC_OSC) {
    if (ascii == ';') {
      d->esc = ESC_START | ESC_TITLE;
    }
  }
  else if (d->esc & ESC_TITLE) {
    if (ascii == '\a') {
      d->esc = 0;
    }
  }
  else if (d->esc & ESC_ALTCHARSET) {
    d->esc = 0;
  }
  else {
    switch (ascii) {
      case '[':
        d->esc |= ESC_CSI;
        break;
        
      case ']':
        d->esc |= ESC_OSC;
        break;
        
      case '(':
        d->esc |= ESC_ALTCHARSET;
        break;
        
      case 'D': {
          if (d->cy == d->row - 1) {
            tscrollup(ctl, 0, 1);
          }
          else {
            tmoveto(ctl, d->cx, d->cy + 1);
          }
          
          d->esc = 0;
        }
        break;
        
      case 'E': {
          tnewline(ctl, 1);
          d->esc = 0;
        }
        break;
        
      case 'M': {
          if (d->cy == 0) {
            tscrolldown(ctl, 0, 1);
          }
          else {
            tmoveto(ctl, d->cx, d->cy - 1);
          }
          
          d->esc = 0;
        }
        break;
        
      case 'c': { /* RIS -- Reset to inital state */
          treset(ctl);
          d->esc = 0;
        }
        break;
        
      case '7': /* DECSC -- Save Cursor */
        d->savex = d->cx;
        d->savey = d->cy;
        d->esc = 0;
        break;
        
      case '8': /* DECRC -- Restore Cursor */
        d->cx = d->savex;
        d->cy = d->savey;
        d->esc = 0;
        break;
        
      default:
        d->esc = 0;
    }
  }
}
void aconsole_add(void * x, int c) {
  ACONTROLP  ctl  = (ACONTROLP) x;
  ACONSOLEP  d    = (ACONSOLEP) ctl->d;
  
  if (d->esc & ESC_START) {
    aconsole_escape_handler(ctl, c);
    return;
  }
  else if (c == '\033') {
    /* ESCAPE */
    csireset(ctl);
    d->esc = ESC_START;
    return;
  }
  
  byte redrawit = 0;
  
  if (c == 7) {
    /* BEEP */
    d->beep = 1;
    vibrate(50);
    aconsole_ondraw(ctl);
    return;
  }
  else if (d->beep) {
    d->beep = 0;
    redrawit = 1;
  }
  
  /* Normal Char Handler */
  int  chr     = 0;
  
  switch (c) {
    case '\t':
      tputtab(ctl);
      break;
      
    case '\b':
      tmoveto(ctl, d->cx - 1, d->cy);
      break;
      
    case '\r':
      tmoveto(ctl, 0, d->cy);
      break;
      
    case '\f':
    case '\v':
    case '\n':
      tnewline(ctl, d->clrf);
      break;
      
    default: {
        if (d->nextwrap) {
          tnewline(ctl, 1);
        }
        
        tsetchar(ctl, c);
        
        if (d->cx + 1 < d->col) {
          tmoveto(ctl, d->cx + 1, d->cy);
        }
        else {
          d->nextwrap = 1;
        }
      }
  }
  
  if (redrawit) {
    aconsole_ondraw(ctl);
  }
}
byte aconsole_isclrf(ACONTROLP  ctl) {
  ACONSOLEP  d    = (ACONSOLEP) ctl->d;
  return d->clrf;
}
ACONTROLP aconsole(AWINDOWP win, int x, int y, int w, int h) {
  //-- Console Data
  ACONSOLEP d = (ACONSOLEP) malloc(sizeof(ACONSOLE));
  memset(d, 0, sizeof(ACONSOLE));
  //-- Set Data
  // d->history  = aStack();
  d->fw       = ag_fontwidth('A', 2);
  d->fh       = ag_fontheight(2);
  d->col      = w / d->fw;
  d->row      = h / d->fh;
  d->cx       = 0;
  d->cy       = 0;
  d->active   = 1;
  d->iscursor = 0;
  d->clrf     = 0;
  //-- Console Info
  d->def_bg = 0;
  d->def_fg = 7;
  d->bg = d->def_bg;
  d->fg = d->def_fg;
  d->bold = 0;
  d->underline = 0;
  d->reserve = 0;
  d->nextwrap = 0;
  d->beep = 0;
  int cvw = d->col * d->fw;
  int cvh = d->row * d->fh;
  d->cv_x = (w / 2) - (cvw / 2);
  d->cv_y = (h / 2) - (cvh / 2);
  //-- Init Colorset
  aconsole_initcolors(d);
  //-- Initializing Canvas
  ag_canvas(&d->cv, cvw, cvh);
  ag_rect(&d->cv, 0, 0, w, h, getbg(d));
  //-- Initializing Control
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &aconsole_ondestroy;
  ctl->oninput  = &aconsole_oninput;
  ctl->ondraw   = &aconsole_ondraw;
  ctl->onblur   = &aconsole_onblur;
  ctl->onfocus  = &aconsole_onfocus;
  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
  ctl->d        = (void *) d;
  aw_add(win, ctl);
  pthread_t rth;
  pthread_create(&rth, NULL, aconsole_drawer, (void *) ctl);
  pthread_detach(rth);
  return ctl;
}