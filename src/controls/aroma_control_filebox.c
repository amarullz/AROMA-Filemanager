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
 * AROMA UI: Checkbox List Window Control
 *
 */
#include "../aroma.h"

#define ACCHK_MAX_GROUP   64
void afbox_redrawitem_ex(ACONTROLP ctl, int index);

/***************************[ CHECKBOX ]**************************/
typedef struct {
  char title[256];
  char desc[256];
  PNGCANVAS * img;
  byte checked;
  int  id;
  int  h;
  int  y;
  byte drawed;
  
  /* Title & Desc Size/Pos */
  int  th;
  int  dh;
  int  ty;
  int  dy;
  
  /* Type */
  byte isTitle;
  int  group;
  int  groupid;
  
  byte d_type;
  char d_perm[10];
  dword d_data;
  
} AFBOXI, * AFBOXIP;
typedef struct {
  byte      acheck_signature;
  CANVAS    client;
  CANVAS    control;
  CANVAS    control_focused;
  AKINETIC  akin;
  int       scrollY;
  int       maxScrollY;
  int       prevTouchY;
  int       invalidDrawItem;
  
  /* Client Size */
  int clientWidth;
  int clientTextW;
  int clientTextX;
  int nextY;
  
  /* Items */
  AFBOXIP * items;
  int       itemn;
  int       touchedItem;
  int       focusedItem;
  int       draweditemn;
  
  int       groupCounts;
  int       groupCurrId;
  int       check_n;
  
  /* Focus */
  byte      focused;
  byte      touchmsg;
  byte      holdmsg;
  byte      changemsg;
  byte      boxtype;
  long      lasttouch;
  int       selectedId;
} AFBOXD, * AFBOXDP;
void afbox_ondestroy(void * x) {
  ACONTROLP ctl = (ACONTROLP) x;
  AFBOXDP d  = (AFBOXDP) ctl->d;
  ag_ccanvas(&d->control);
  ag_ccanvas(&d->control_focused);
  
  if (d->itemn > 0) {
    int i;
    
    for (i = 0; i < d->itemn; i++) {
      free(d->items[i]);
    }
    
    free(d->items);
    ag_ccanvas(&d->client);
  }
  
  free(ctl->d);
}
int afbox_itemcount(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return -1;
  }
  
  return d->itemn;
}
int afbox_checkcount(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return -1;
  }
  
  return d->check_n;
}
byte afbox_ischecked(ACONTROLP ctl, int index) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  if (index < d->itemn) {
    return d->items[index]->checked;
  }
  
  return 0;
}
char * afbox_getcfile(ACONTROLP ctl, int index) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  if (index < d->itemn) {
    if (d->items[index]->checked) {
      return d->items[index]->title;
    }
  }
  
  return NULL;
}
byte afbox_isgroup(ACONTROLP ctl, int index) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  return d->items[index]->isTitle;
}
byte afbox_dtype(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  if (d->selectedId == -1) {
    return 0;
  }
  
  return d->items[d->selectedId]->d_type;
}
char * afbox_dperm(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  if (d->selectedId == -1) {
    return 0;
  }
  
  return d->items[d->selectedId]->d_perm;
}
dword afbox_ddata(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  if (d->selectedId == -1) {
    return 0;
  }
  
  return d->items[d->selectedId]->d_data;
}
char * afbox_getselectedfile(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return NULL;
  }
  
  if (d->selectedId == -1) {
    return NULL;
  }
  
  return d->items[d->selectedId]->title;
}
char * afbox_getselecteddesc(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return NULL;
  }
  
  if (d->selectedId == -1) {
    return NULL;
  }
  
  return d->items[d->selectedId]->desc;
}
int afbox_getgroup(ACONTROLP ctl, int index) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  return d->items[index]->group;
}
int afbox_getgroupid(ACONTROLP ctl, int index) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;
  }
  
  return d->items[index]->groupid;
}
void afbox_redrawitem(ACONTROLP ctl, int index) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return;  //-- Not Valid Signature
  }
  
  if ((index >= d->itemn) || (index < 0)) {
    return;  //-- Not Valid Index
  }
  
  AFBOXIP p = d->items[index];
  p->drawed = 0;
}
void afbox_redrawitem_ex(ACONTROLP ctl, int index) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return;  //-- Not Valid Signature
  }
  
  if ((index >= d->itemn) || (index < 0)) {
    return;  //-- Not Valid Index
  }
  
  AFBOXIP p = d->items[index];
  CANVAS  * c = &d->client;
  //-- Cleanup Background
  ag_rect(c, 0, p->y, d->clientWidth, p->h, acfg()->textbg);
  
  if (p->isTitle) {
    ag_roundgrad(c, 0, p->y, d->clientWidth, p->h, acfg()->titlebg, acfg()->titlebg_g, 0);
    ag_textf(c, d->clientTextW + (agdp() * 14), (d->clientTextX - (agdp() * 14)) + 1, p->y + p->ty, p->title, acfg()->titlebg_g, 0);
    ag_text(c, d->clientTextW + (agdp() * 14), d->clientTextX - (agdp() * 14), p->y + p->ty - 1, p->title, acfg()->titlefg, 0);
  }
  else {
    color txtcolor = acfg()->textfg;
    color graycolor = acfg()->textfg_gray;
    byte isselectcolor = 0;
    
    if (index == d->touchedItem) {
      if (!atheme_draw("img.selection.push", c, 0, p->y + 1, d->clientWidth, p->h - 2)) {
        color pshad = ag_calpushad(acfg()->selectbg_g);
        dword hl1 = ag_calcpushlight(acfg()->selectbg, pshad);
        ag_roundgrad(c, 0, p->y + 1, d->clientWidth, p->h - 3, acfg()->selectbg, pshad, (agdp() * 2));
        ag_roundgrad(c, 0, p->y + 1, d->clientWidth, (p->h - 3) / 2, LOWORD(hl1), HIWORD(hl1), (agdp() * 2));
      }
      
      graycolor = txtcolor = acfg()->selectfg;
      isselectcolor = 1;
    }
    else if ((index == d->focusedItem) && (d->focused)) {
      if (!atheme_draw("img.selection", c, 0, p->y + 1, d->clientWidth, p->h - 2)) {
        dword hl1 = ag_calchighlight(acfg()->selectbg, acfg()->selectbg_g);
        ag_roundgrad(c, 0, p->y + 1, d->clientWidth, p->h - 3, acfg()->selectbg, acfg()->selectbg_g, (agdp() * 2));
        ag_roundgrad(c, 0, p->y + 1, d->clientWidth, (p->h - 3) / 2, LOWORD(hl1), HIWORD(hl1), (agdp() * 2));
      }
      
      graycolor = txtcolor = acfg()->selectfg;
      isselectcolor = 1;
    }
    
    if (index < d->itemn - 1) {
      //-- Not Last... Add Separator
      color sepcl = ag_calculatealpha(acfg()->textbg, acfg()->textfg_gray, 80);
      ag_rect(c, 0, p->y + p->h - 1, d->clientWidth, 1, sepcl);
    }
    
    //-- Now Draw The Checkbox
    int imgS = agdp() * 24;
    
    if (p->img != NULL) {
      apng_stretch(c, p->img, agdp(), p->y + agdp(), imgS, imgS, 0, 0, p->img->w, p->img->h);
    }
    
    int txt_h = p->th + p->dh;
    int tit_y = (p->h / 2) - (txt_h / 2);
    int des_y = tit_y + p->th;
    int des_add = 0;
    
    if (d->boxtype != 0) {
      des_add = (agdp() * 16);
    }
    
    char permstr[64];
    snprintf(permstr, 64, "%s", p->d_perm);
    
    //-- Now Draw The Text
    if (isselectcolor) {
      ag_textf(c, d->clientTextW, d->clientTextX, p->y + tit_y, p->title, acfg()->selectbg_g, 1);
      ag_textf(c, d->clientTextW + des_add, d->clientTextX, p->y + des_y, p->desc, acfg()->selectbg_g, 0);
      ag_textf(c, d->clientTextW + des_add, d->clientTextX - 1, p->y + des_y - 1, p->desc, graycolor, 0);
      ag_textf(c, d->clientTextW + des_add, d->clientTextX, p->y + des_y, permstr, acfg()->selectbg_g, 0);
      ag_textf(c, d->clientTextW + des_add, d->clientTextX - 1, p->y + des_y - 1, permstr, graycolor, 0);
    }
    else {
      ag_text(c, d->clientTextW + des_add, d->clientTextX - 1, p->y + des_y - 1, p->desc, graycolor, 0);
      ag_text(c, d->clientTextW + des_add, d->clientTextX - 1, p->y + des_y - 1, permstr, graycolor, 0);
    }
    
    ag_textf(c, d->clientTextW, d->clientTextX - 1, p->y + tit_y - 1, p->title, txtcolor, 1);
    // img
    //-- Now Draw The Checkbox
    int halfdp   = ceil(((float) agdp()) / 2);
    int halfdp2  = halfdp * 2;
    int chkbox_s = (agdp() * 10);
    int chkbox_x = (d->clientTextX + d->clientTextW + (agdp() * 9)) - ((chkbox_s + 2) / 2);
    int chkbox_y = p->y + round((p->h / 2) - (chkbox_s / 2));
    byte drawed = 0;
    int minpad = 3 * agdp();
    int addpad = 6 * agdp();
    
    if (d->boxtype == 0) {
      if (p->checked) {
        if (index == d->touchedItem) {
          drawed = atheme_draw("img.checkbox.on.push", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else if ((index == d->focusedItem) && (d->focused)) {
          drawed = atheme_draw("img.checkbox.on.focus", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else {
          drawed = atheme_draw("img.checkbox.on", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
      }
      else {
        if (index == d->touchedItem) {
          drawed = atheme_draw("img.checkbox.push", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else if ((index == d->focusedItem) && (d->focused)) {
          drawed = atheme_draw("img.checkbox.focus", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
        else {
          drawed = atheme_draw("img.checkbox", c, chkbox_x - minpad, chkbox_y - minpad, chkbox_s + addpad, chkbox_s + addpad);
        }
      }
      
      if (!drawed) {
        ag_roundgrad(c,
                     chkbox_x,
                     chkbox_y,
                     chkbox_s,
                     chkbox_s,
                     acfg()->controlbg_g,
                     acfg()->controlbg,
                     0);
        ag_roundgrad(c,
                     chkbox_x + halfdp,
                     chkbox_y + halfdp,
                     chkbox_s - halfdp2,
                     chkbox_s - halfdp2,
                     acfg()->textbg,
                     acfg()->textbg,
                     0);
                     
        if (p->checked) {
          ag_roundgrad(c,
                       chkbox_x + halfdp2,
                       chkbox_y + halfdp2,
                       chkbox_s - (halfdp2 * 2),
                       chkbox_s - (halfdp2 * 2),
                       acfg()->selectbg,
                       acfg()->selectbg_g,
                       0);
        }
      }
    }
  }
}
void afbox_redraw(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return;  //-- Not Valid Signature
  }
  
  if ((d->itemn > 0) && (d->draweditemn < d->itemn)) {
    ag_ccanvas(&d->client);
    ag_canvas(&d->client, d->clientWidth, d->nextY);
    ag_rect(&d->client, 0, 0, d->clientWidth, agdp() * 2, acfg()->textbg);
    //-- Set Values
    d->scrollY     = 0;
    d->maxScrollY  = d->nextY - (ctl->h - (agdp() * 2));
    
    if (d->maxScrollY < 0) {
      d->maxScrollY = 0;
    }
    
    //-- Draw Items
    int i;
    
    for (i = 0; i < d->itemn; i++) {
      ag_rect(&d->client, 0, d->items[i]->y, d->clientWidth, d->items[i]->h, acfg()->textbg);
      afbox_redrawitem(ctl, i);
    }
    
    d->draweditemn = d->itemn;
  }
}
void afbox_changeboxtype(ACONTROLP ctl, byte t) {
  ag_setbusy();
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return;  //-- Not Valid Signature
  }
  
  d->boxtype      = t;
  int i;
  
  for (i = 0; i < d->itemn; i++) {
    afbox_redrawitem(ctl, i);
  }
  
  ctl->ondraw(ctl);
  aw_redraw(ctl->win);
  ag_sync();
}
//-- Add Item Into Control
byte afbox_add(ACONTROLP ctl, char * title, char * desc, byte checked, PNGCANVAS * img,
               byte d_type, char * d_perm, dword d_data, byte selDef) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;  //-- Not Valid Signature
  }
  
  //-- Allocating Memory For Item Data
  AFBOXIP newip = (AFBOXIP) malloc(sizeof(AFBOXI));
  newip->d_type   = d_type;
  newip->d_data   = d_data;
  snprintf(newip->d_perm, 10, "%s", d_perm);
  snprintf(newip->title, 256, "%s", title);
  snprintf(newip->desc, 256, "%s", desc);
  newip->img      = img;
  int imgS        = agdp() * 24;
  newip->drawed   = 0;
  newip->th       = ag_txtheight(d->clientTextW, newip->title, 1);
  newip->dh       = ag_fontheight(0); // ag_txtheight(d->clientTextW,newip->desc,0);
  newip->ty       = agdp() * 4;
  newip->dy       = newip->ty + newip->th;
  newip->h        = (agdp() * 8) + newip->dh + newip->th;
  
  if (newip->h < (agdp() * 26)) {
    newip->h = (agdp() * 26);
  }
  
  newip->checked  = checked;
  newip->id       = d->itemn;
  newip->group    = d->groupCounts;
  newip->groupid  = ++d->groupCurrId;
  newip->isTitle  = 0;
  newip->y        = d->nextY;
  d->nextY       += newip->h;
  
  if (selDef) {
    d->focusedItem = d->itemn;
    d->selectedId  = d->itemn;
  }
  
  if (checked) {
    d->check_n++;
  }
  
  if (d->itemn > 0) {
    int i;
    AFBOXIP * tmpitms   = d->items;
    d->items              = malloc( sizeof(AFBOXIP) * (d->itemn + 1) );
    
    for (i = 0; i < d->itemn; i++) {
      d->items[i] = tmpitms[i];
    }
    
    d->items[d->itemn] = newip;
    free(tmpitms);
  }
  else {
    d->items    = malloc(sizeof(AFBOXIP));
    d->items[0] = newip;
  }
  
  d->itemn++;
  return 1;
}
//-- Add Item Into Control
byte afbox_addgroup(ACONTROLP ctl, char * title, char * desc) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return 0;  //-- Not Valid Signature
  }
  
  if (d->groupCounts + 1 >= ACCHK_MAX_GROUP) {
    return 0;
  }
  
  //-- Allocating Memory For Item Data
  AFBOXIP newip = (AFBOXIP) malloc(sizeof(AFBOXI));
  snprintf(newip->title, 64, "%s", title);
  snprintf(newip->desc, 128, "%s", desc);
  newip->th       = ag_txtheight(d->clientTextW + (agdp() * 14), newip->title, 0);
  newip->dh       = 0;
  newip->ty       = agdp() * 3;
  newip->dy       = (agdp() * 3) + newip->th;
  newip->h        = (agdp() * 6) + newip->dh + newip->th;
  newip->id       = d->itemn;
  newip->group    = ++d->groupCounts;
  d->groupCurrId  = -1;
  newip->groupid  = -1;
  newip->isTitle  = 1;
  newip->y        = d->nextY;
  d->nextY       += newip->h;
  
  if (d->itemn > 0) {
    int i;
    AFBOXIP * tmpitms   = d->items;
    d->items              = malloc( sizeof(AFBOXIP) * (d->itemn + 1) );
    
    for (i = 0; i < d->itemn; i++) {
      d->items[i] = tmpitms[i];
    }
    
    d->items[d->itemn] = newip;
    free(tmpitms);
  }
  else {
    d->items    = malloc(sizeof(AFBOXIP));
    d->items[0] = newip;
  }
  
  d->itemn++;
  return 1;
}

typedef struct  {
  ACONTROLP     ctl;
  long     *    tick;
} AFBOXWAITD, * AFBOXWAITDP;
static void * afbox_holdthread(void * cookie) {
  AFBOXWAITDP dt = (AFBOXWAITDP) cookie;
  
  if (dt->ctl->win->isActived) {
    dt->ctl->win->threadnum++;
  }
  else {
    free(dt);
    return NULL;
  }
  
  byte isvalid = 1;
  
  while ( alib_tick() < (dt->tick[0] + 50) ) {
    if (!dt->ctl->win->isActived) {
      isvalid = 0;
      break;
    }
    
    if (dt->tick[0] <= 0) {
      isvalid = 0;
      break;
    }
    
    usleep(10);
  }
  
  if ((isvalid) && (dt->tick[0] > 0)) {
    dt->tick[0] = -1;
    dt->ctl->oninput(dt->ctl, 444, NULL);
  }
  
  dt->ctl->win->threadnum--;
  free(dt);
  return NULL;
}
void afbox_reghold(
  ACONTROLP     ctl,
  long     *    tick
) {
  AFBOXWAITDP fdt = (AFBOXWAITDP) malloc(sizeof(AFBOXWAITD));
  fdt->ctl         = ctl;
  fdt->tick        = tick;
  pthread_t afboxholdt;
  pthread_create(&afboxholdt, NULL, afbox_holdthread, (void *) fdt);
  pthread_detach(afboxholdt);
}

void afbox_ondraw(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  AFBOXDP   d  = (AFBOXDP) ctl->d;
  CANVAS   *  pc = &ctl->win->c;
  afbox_redraw(ctl);
  
  if (d->invalidDrawItem != -1) {
    d->touchedItem = d->invalidDrawItem;
    afbox_redrawitem_ex(ctl, d->invalidDrawItem);
    d->invalidDrawItem = -1;
    d->lasttouch = alib_tick();
    
    if (((d->boxtype == 0) && (d->check_n == 0)) || (d->boxtype == 2)) {
      afbox_reghold(ctl, &d->lasttouch);
    }
  }
  
  //-- Init Device Pixel Size
  int minpadding = 2;
  int agdp3 = (agdp() * minpadding);
  int agdp6 = (agdp() * (minpadding * 2));
  int agdpX = agdp6;
  //-- Realtime redraw
  int dr_top = d->scrollY;
  int dr_bottom = dr_top + ctl->h;
  int i;
  
  for (i = 0; i < d->itemn; i++) {
    AFBOXIP p = d->items[i];
    
    if (p->y + p->h < dr_top) {
      continue;
    }
    else if (p->y > dr_bottom) {
      break;
    }
    else if (!p->drawed) {
      afbox_redrawitem_ex(ctl, i);
      p->drawed = 1;
    }
  }
  
  ag_draw(pc, &d->control, ctl->x, ctl->y);
  ag_draw_ex(pc, &d->client, ctl->x + agdp(), ctl->y + 1, 0, d->scrollY + 1, ctl->w - (agdp() * 2), ctl->h - 2);
  
  if (d->maxScrollY > 0) {
    //-- Glow
    // int i;
    byte isST = (d->scrollY > 0) ? 1 : 0;
    byte isSB = (d->scrollY < d->maxScrollY) ? 1 : 0;
    int add_t_y = 1;
    
    for (i = 0; i < agdpX; i++) {
      byte alph = 255 - round((((float) (i + 1)) / ((float) agdpX)) * 230);
      
      if (isST) {
        ag_rectopa(pc, ctl->x, ctl->y + i + add_t_y, ctl->w, 1, acfg()->textbg, alph);
      }
      
      if (isSB) {
        ag_rectopa(pc, ctl->x, ((ctl->y + ctl->h) - (add_t_y)) - (i + 1), ctl->w, 1, acfg()->textbg, alph);
      }
    }
    
    //-- Scrollbar
    int newh = ctl->h - agdp6;
    float scrdif    = ((float) newh) / ((float) d->client.h);
    int  scrollbarH = round(scrdif * newh);
    int  scrollbarY = round(scrdif * d->scrollY) + agdp3;
    
    if (d->scrollY < 0) {
      scrollbarY = agdp3;
      int alp = (1.0 - (((float) abs(d->scrollY)) / (((float) ctl->h) / 4))) * 255;
      
      if (alp < 0) {
        alp = 0;
      }
      
      ag_rectopa(pc, (ctl->w - agdp() - 2) + ctl->x, scrollbarY + ctl->y, agdp(), scrollbarH, acfg()->scrollbar, alp);
    }
    else if (d->scrollY > d->maxScrollY) {
      scrollbarY = round(scrdif * d->maxScrollY) + agdp3;
      int alp = (1.0 - (((float) abs(d->scrollY - d->maxScrollY)) / (((float) ctl->h) / 4))) * 255;
      
      if (alp < 0) {
        alp = 0;
      }
      
      ag_rectopa(pc, (ctl->w - agdp() - 2) + ctl->x, scrollbarY + ctl->y, agdp(), scrollbarH, acfg()->scrollbar, alp);
    }
    else {
      ag_rect(pc, (ctl->w - agdp() - 2) + ctl->x, scrollbarY + ctl->y, agdp(), scrollbarH, acfg()->scrollbar);
    }
  }
}
void afbox_scrolltoitem(ACONTROLP ctl) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->focusedItem > -1) {
    afbox_redraw(ctl);
    d->scrollY = d->items[d->focusedItem]->y - round((ctl->h / 2) - (d->items[d->focusedItem]->h / 2));
    
    if (d->scrollY < 0) {
      d->scrollY = 0;
    }
    
    if (d->scrollY > d->maxScrollY) {
      d->scrollY = d->maxScrollY;
    }
  }
  
  ctl->ondraw(ctl);
  aw_redraw(ctl->win);
}
void afbox_setcheckall(ACONTROLP ctl, byte checked) {
  AFBOXDP d = (AFBOXDP) ctl->d;
  
  if (d->acheck_signature != 177) {
    return;
  }
  
  int i;
  
  for (i = 0; i < d->itemn; i++) {
    AFBOXIP p = d->items[i];
    
    if (p->checked != checked) {
      p->checked = checked;
      afbox_redrawitem(ctl, i);
      d->check_n += (checked) ? 1 : -1;
    }
  }
  
  ctl->ondraw(ctl);
  aw_redraw(ctl->win);
  ag_sync();
  dword msg = aw_msg(d->changemsg, 1, 0, 0);
  atouch_send_message(msg);
}
dword afbox_oninput(void * x, int action, ATEV * atev) {
  ACONTROLP ctl = (ACONTROLP) x;
  AFBOXDP d  = (AFBOXDP) ctl->d;
  dword msg = 0;
  
  switch (action) {
    case 444: {
        // printf("HOLDED: %i - %s",d->touchedItem,d->items[d->touchedItem]->title);
        vibrate(30);
        int tmptouch = d->touchedItem;
        int tmpfocus = d->focusedItem;
        d->selectedId  = tmptouch;
        d->focusedItem = tmptouch;
        d->touchedItem = -1;
        
        if (tmpfocus != tmptouch) {
          afbox_redrawitem_ex(ctl, tmpfocus);
        }
        
        afbox_redrawitem_ex(ctl, tmptouch);
        ctl->ondraw(ctl);
        msg = aw_msg(d->holdmsg, 1, 0, 0);
        atouch_send_message(msg);
      }
      break;
      
    case ATEV_MOUSEDN: {
        d->prevTouchY  = atev->y;
        akinetic_downhandler(&d->akin, atev->y);
        int touchpos = atev->y - ctl->y + d->scrollY;
        int i;
        
        for (i = 0; i < d->itemn; i++) {
          if ((touchpos >= d->items[i]->y) && (touchpos < d->items[i]->y + d->items[i]->h)) {
            ac_regpushwait(
              ctl, &d->prevTouchY, &d->invalidDrawItem, i
            );
            break;
          }
        }
      }
      break;
      
    case ATEV_MOUSEUP: {
        byte is_holded = 0;
        
        if (d->lasttouch == -1) {
          is_holded = 1;
        }
        
        d->lasttouch = 0;
        byte ag_check_msg = 0;
        
        if ((d->prevTouchY != -50) && (abs(d->prevTouchY - atev->y) < agdp() * 5)) {
          d->prevTouchY = -50;
          int touchpos = atev->y - ctl->y + d->scrollY;
          byte oncheckpush = ((d->check_n > 0) || (atev->x > ((ctl->x + ctl->w) - (agdp() * 32)))) ? 1 : 0;
          
          if (d->boxtype != 0) {
            oncheckpush = 0;
          }
          
          int i;
          
          for (i = 0; i < d->itemn; i++) {
            if ((!d->items[i]->isTitle) && (touchpos >= d->items[i]->y) && (touchpos < d->items[i]->y + d->items[i]->h)) {
              if ((oncheckpush) && (!is_holded)) {
                d->items[i]->checked = (d->items[i]->checked) ? 0 : 1;
                
                if (d->items[i]->checked) {
                  d->check_n++;
                }
                else {
                  d->check_n--;
                }
                
                ag_check_msg = d->changemsg;
              }
              
              if ((d->touchedItem != -1) && (d->touchedItem != i)) {
                int tmptouch = d->touchedItem;
                d->touchedItem = -1;
                afbox_redrawitem_ex(ctl, tmptouch);
              }
              
              int prevfocus = d->focusedItem;
              d->focusedItem = i;
              d->touchedItem = i;
              
              if ((prevfocus != -1) && (prevfocus != i)) {
                afbox_redrawitem_ex(ctl, prevfocus);
              }
              
              afbox_redrawitem_ex(ctl, i);
              ctl->ondraw(ctl);
              aw_draw(ctl->win);
              
              if (!is_holded) {
                vibrate(30);
                
                if (!oncheckpush) {
                  ag_check_msg = d->touchmsg;
                  d->selectedId = i;
                  msg = aw_msg(ag_check_msg, 1, 0, 0);
                }
              }
              
              break;
            }
          }
          
          if ((d->scrollY < 0) || (d->scrollY > d->maxScrollY)) {
            ac_regbounce(ctl, &d->scrollY, d->maxScrollY);
          }
        }
        else {
          if (akinetic_uphandler(&d->akin, atev->y)) {
            ac_regfling(ctl, &d->akin, &d->scrollY, d->maxScrollY);
          }
          else if ((d->scrollY < 0) || (d->scrollY > d->maxScrollY)) {
            ac_regbounce(ctl, &d->scrollY, d->maxScrollY);
          }
        }
        
        if (d->touchedItem != -1) {
          usleep(30);
          int tmptouch = d->touchedItem;
          d->touchedItem = -1;
          afbox_redrawitem_ex(ctl, tmptouch);
          ctl->ondraw(ctl);
          msg = aw_msg(ag_check_msg, 1, 0, 0);
        }
      }
      break;
      
    case ATEV_MOUSEMV: {
        byte allowscroll = 1;
        
        if (atev->y != 0) {
          if (d->prevTouchY != -50) {
            if (abs(d->prevTouchY - atev->y) >= agdp() * 5) {
              d->prevTouchY = -50;
              
              if (d->touchedItem != -1) {
                int tmptouch = d->touchedItem;
                d->touchedItem = -1;
                afbox_redrawitem_ex(ctl, tmptouch);
                ctl->ondraw(ctl);
                aw_draw(ctl->win);
              }
            }
            else {
              allowscroll = 0;
            }
          }
          
          if (allowscroll) {
            d->lasttouch = 0;
            int mv = akinetic_movehandler(&d->akin, atev->y);
            
            if (mv != 0) {
              if ((d->scrollY < 0) && (mv < 0)) {
                float dumpsz = 0.6 - (0.6 * (((float) abs(d->scrollY)) / (ctl->h / 4)));
                d->scrollY += floor(mv * dumpsz);
              }
              else if ((d->scrollY > d->maxScrollY) && (mv > 0)) {
                float dumpsz = 0.6 - (0.6 * (((float) abs(d->scrollY - d->maxScrollY)) / (ctl->h / 4)));
                d->scrollY += floor(mv * dumpsz);
              }
              else {
                d->scrollY += mv;
              }
              
              if (d->scrollY < 0 - (ctl->h / 4)) {
                d->scrollY = 0 - (ctl->h / 4);
              }
              
              if (d->scrollY > d->maxScrollY + (ctl->h / 4)) {
                d->scrollY = d->maxScrollY + (ctl->h / 4);
              }
              
              msg = aw_msg(0, 1, 0, 0);
              ctl->ondraw(ctl);
            }
          }
        }
      }
      break;
      
    case ATEV_SEARCH: {
        d->lasttouch = 0;
        
        if (d->boxtype == 0) {
          if ((d->focusedItem > -1) && (d->draweditemn > 0)) {
            if (atev->d) {
              if ((d->touchedItem != -1) && (d->touchedItem != d->focusedItem)) {
                int tmptouch = d->touchedItem;
                d->touchedItem = -1;
                afbox_redrawitem_ex(ctl, tmptouch);
              }
              
              vibrate(30);
              d->touchedItem = d->focusedItem;
              afbox_redrawitem_ex(ctl, d->focusedItem);
              ctl->ondraw(ctl);
              msg = aw_msg(0, 1, 0, 0);
            }
            else {
              d->items[d->focusedItem]->checked = (d->items[d->focusedItem]->checked) ? 0 : 1;
              
              if (d->items[d->focusedItem]->checked) {
                d->check_n++;
              }
              else {
                d->check_n--;
              }
              
              d->touchedItem = -1;
              afbox_redrawitem_ex(ctl, d->focusedItem);
              ctl->ondraw(ctl);
              msg = aw_msg(d->changemsg, 1, 0, 0);
            }
          }
        }
      }
      break;
      
    case ATEV_SELECT: {
        d->lasttouch = 0;
        
        if ((d->focusedItem > -1) && (d->draweditemn > 0)) {
          if (atev->d) {
            if ((d->touchedItem != -1) && (d->touchedItem != d->focusedItem)) {
              int tmptouch = d->touchedItem;
              d->touchedItem = -1;
              afbox_redrawitem_ex(ctl, tmptouch);
            }
            
            vibrate(30);
            d->touchedItem = d->focusedItem;
            afbox_redrawitem_ex(ctl, d->focusedItem);
            ctl->ondraw(ctl);
            msg = aw_msg(0, 1, 0, 0);
          }
          else {
            if ((d->check_n > 0) && (d->boxtype == 0)) {
              d->items[d->focusedItem]->checked = (d->items[d->focusedItem]->checked) ? 0 : 1;
              
              if (d->items[d->focusedItem]->checked) {
                d->check_n++;
              }
              else {
                d->check_n--;
              }
              
              d->touchedItem = -1;
              afbox_redrawitem_ex(ctl, d->focusedItem);
              ctl->ondraw(ctl);
              msg = aw_msg(d->changemsg, 1, 0, 0);
            }
            else {
              if ((d->touchedItem != -1) && (d->touchedItem != d->focusedItem)) {
                int tmptouch = d->touchedItem;
                d->touchedItem = -1;
                afbox_redrawitem_ex(ctl, tmptouch);
              }
              
              d->selectedId = d->focusedItem;
              d->touchedItem = -1;
              afbox_redrawitem_ex(ctl, d->focusedItem);
              ctl->ondraw(ctl);
              msg = aw_msg(d->touchmsg, 1, 0, 0);
            }
          }
        }
      }
      break;
      
    case ATEV_DOWN: {
        d->lasttouch = 0;
        
        if ((d->focusedItem < d->itemn - 1) && (d->draweditemn > 0)) {
          int prevfocus = d->focusedItem;
          d->focusedItem++;
          
          while (d->items[d->focusedItem]->isTitle) {
            d->focusedItem++;
            
            if (d->focusedItem > d->itemn - 1) {
              d->focusedItem = prevfocus;
              d->selectedId  = d->focusedItem;
              return 0;
            }
          }
          
          d->selectedId  = d->focusedItem;
          afbox_redrawitem_ex(ctl, prevfocus);
          afbox_redrawitem_ex(ctl, d->focusedItem);
          ctl->ondraw(ctl);
          msg = aw_msg(0, 1, 1, 0);
          int reqY = d->items[d->focusedItem]->y - round((ctl->h / 2) - (d->items[d->focusedItem]->h / 2));
          ac_regscrollto(
            ctl,
            &d->scrollY,
            d->maxScrollY,
            reqY,
            &d->focusedItem,
            d->focusedItem
          );
        }
      }
      break;
      
    case ATEV_UP: {
        d->lasttouch = 0;
        
        if ((d->focusedItem > 0) && (d->draweditemn > 0)) {
          int prevfocus = d->focusedItem;
          d->focusedItem--;
          
          while (d->items[d->focusedItem]->isTitle) {
            d->focusedItem--;
            
            if (d->focusedItem < 0) {
              d->focusedItem = prevfocus;
              d->selectedId  = d->focusedItem;
              return 0;
            }
          }
          
          d->selectedId  = d->focusedItem;
          afbox_redrawitem_ex(ctl, prevfocus);
          afbox_redrawitem_ex(ctl, d->focusedItem);
          ctl->ondraw(ctl);
          msg = aw_msg(0, 1, 1, 0);
          int reqY = d->items[d->focusedItem]->y - round((ctl->h / 2) - (d->items[d->focusedItem]->h / 2));
          ac_regscrollto(
            ctl,
            &d->scrollY,
            d->maxScrollY,
            reqY,
            &d->focusedItem,
            d->focusedItem
          );
        }
      }
      break;
  }
  
  return msg;
}
byte afbox_onfocus(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  AFBOXDP   d  = (AFBOXDP) ctl->d;
  d->focused = 1;
  
  if ((d->focusedItem == -1) && (d->itemn > 0)) {
    d->focusedItem = 0;
  }
  
  if ((d->focusedItem != -1) && (d->draweditemn > 0)) {
    afbox_redrawitem_ex(ctl, d->focusedItem);
  }
  
  ctl->ondraw(ctl);
  return 1;
}
void afbox_onblur(void * x) {
  ACONTROLP   ctl = (ACONTROLP) x;
  AFBOXDP   d  = (AFBOXDP) ctl->d;
  d->focused = 0;
  
  if ((d->focusedItem != -1) && (d->draweditemn > 0)) {
    afbox_redrawitem_ex(ctl, d->focusedItem);
  }
  
  ctl->ondraw(ctl);
}
ACONTROLP afbox(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  byte touchmsg,
  byte holdmsg,
  byte boxtype,
  byte changemsg
) {
  //-- Validate Minimum Size
  if (h < agdp() * 16) {
    h = agdp() * 16;
  }
  
  if (w < agdp() * 20) {
    w = agdp() * 20;
  }
  
  //-- Initializing Text Data
  AFBOXDP d        = (AFBOXDP) malloc(sizeof(AFBOXD));
  memset(d, 0, sizeof(AFBOXD));
  //-- Set Signature
  d->acheck_signature = 177;
  d->touchmsg         = touchmsg;
  d->holdmsg          = holdmsg;
  d->changemsg        = changemsg;
  d->boxtype          = boxtype;
  d->lasttouch        = 0;
  d->check_n          = 0;
  //-- Initializing Canvas
  ag_canvas(&d->control, w, h);
  ag_canvas(&d->control_focused, w, h);
  int minpadding = 2;
  //-- Initializing Client Size
  d->clientWidth  = w - (agdp() * minpadding);
  d->clientTextW  = d->clientWidth - (agdp() * 44);
  d->clientTextX  = (agdp() * 26);
  d->client.data = NULL;
  //-- Draw Control
  ag_draw_ex(&d->control, &win->c, 0, 0, x, y, w, h);
  ag_roundgrad(&d->control, 0, 0, w, h, acfg()->textbg, acfg()->textbg, 0);
  //-- Draw Focused Control
  ag_draw_ex(&d->control_focused, &win->c, 0, 0, x, y, w, h);
  ag_roundgrad(&d->control_focused, 0, 0, w, h, acfg()->selectbg, acfg()->selectbg_g, 0);
  ag_roundgrad(&d->control_focused, 1, 1, w - 2, h - 2, acfg()->textbg, acfg()->textbg, 0);
  //-- Set Scroll Value
  d->selectedId  = -1;
  d->scrollY     = 0;
  d->maxScrollY  = 0;
  d->prevTouchY  = -50;
  d->invalidDrawItem = -1;
  //-- Set Data Values
  d->items       = NULL;
  d->itemn       = 0;
  d->touchedItem = -1;
  d->focusedItem = -1;
  d->nextY       = agdp();
  d->draweditemn = 0;
  d->groupCounts = 0;
  d->groupCurrId = -1;
  ACONTROLP ctl = malloc(sizeof(ACONTROL));
  ctl->ondestroy = &afbox_ondestroy;
  ctl->oninput  = &afbox_oninput;
  ctl->ondraw   = &afbox_ondraw;
  ctl->onblur   = &afbox_onblur;
  ctl->onfocus  = &afbox_onfocus;
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