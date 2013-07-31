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
 * AROMA File Manager UI Drawings
 *
 */
//*
//* Redraw Window Background
//*
void aui_redraw() {
  if (!aui_isbgredraw) {
    return;
  }
  
  ag_blank(&aui_bg);
  int elmP = agdp() * 2;
  int capH = ag_fontheight(1) + (elmP * 2);
  aui_minY = capH;
  ag_rect(&aui_bg, 0, 0, agw(), agh(), 0x0000);
  
  //-- Background
  if (!atheme_id_draw(0, &aui_bg, 0, 0, agw(), agh())) {
    ag_roundgrad(&aui_bg, 0, 0, agw(), agh(), acfg()->winbg,
                 acfg()->winbg_g, acfg()->winroundsz * agdp() + 2);
  }
  
  //-- Titlebar
  if (!atheme_id_draw(1, &aui_bg, 0, 0, agw(), capH)) {
    ag_roundgrad_ex(&aui_bg, 0, 0, agw(), capH, acfg()->titlebg,
                    acfg()->titlebg_g,
                    (acfg()->winroundsz * agdp()) - 2, 1, 1, 0, 0);
  }
  
  aui_isbgredraw = 0;
}

//*
//* Init Window Background With New Title
//*
void aui_setbg(char * titlev) {
  aui_redraw();
  ag_draw(&aui_win_bg, &aui_bg, 0, 0);
}

//*
//* Draw Title Text
//*
void aui_changetitle(AUI_VARSP v, char * titlev) {
  char title[128];
  snprintf(title, 128, "%s", titlev);
  int elmP = agdp() * 2;
  int titW = ag_txtwidth(title, 1);
  ag_draw(&v->hWin->c, v->hWin->bg, 0, 0);
  ag_draw_ex(&v->hWin->c, v->hWin->bg, 0, 0, 0, 0, agw(), aui_minY);
  ag_textf(&v->hWin->c, titW, ((agw() / 2) - (titW / 2)) + 1, elmP + 1,
           title, acfg()->titlebg_g, 1);
  ag_text(&v->hWin->c, titW, (agw() / 2) - (titW / 2), elmP, title,
          acfg()->titlefg, 1);
  //-- Redraw Window
  aw_redraw(v->b1->win);
  ag_sync();
}

//*
//* Draw Navigation Bar
//*
void aui_drawnav(CANVAS * bg, int x, int y, int w, int h) {
  if (!atheme_id_draw(2, bg, x, y, w, h)) {
    ag_rect(bg, x, y, w, 1,
            ag_calculatealpha(acfg()->border, acfg()->border_g,
                              50));
    ag_roundgrad_ex(bg, x, y + 1, w, h - 1, acfg()->navbg,
                    acfg()->navbg_g,
                    (acfg()->winroundsz * agdp()) - 2, 0, 0, 1, 1);
  }
}

//*
//* Draw Top Toolbar
//*
void aui_drawtopbar(CANVAS * bg, int x, int y, int w, int h) {
  ag_roundgrad(bg, x, y, w, h - 1,
               acfg()->navbg,
               ag_calculatealpha(acfg()->navbg, acfg()->navbg_g, 130), 0);
  ag_rect(bg, x, y + (h - 1), w, 1,
          ag_calculatealpha(acfg()->border, acfg()->border_g, 130));
}

//*
//* Draw Path
//*
void aui_pathui(CANVAS * bg, int x, int y, int w, int h, char * txt) {
  dword hl1 = ag_calchighlight(acfg()->controlbg, acfg()->controlbg_g);
  
  if (!atheme_draw("img.button", bg, x, y, w, h)) {
    ag_roundgrad(bg, x, y, w, h, acfg()->border, acfg()->border_g,
                 (agdp() * acfg()->btnroundsz));
    ag_roundgrad(bg, x + 1, y + 1, w - 2, h - 2,
                 ag_calculatealpha(acfg()->controlbg, acfg()->winbg,
                                   180),
                 ag_calculatealpha(acfg()->controlbg_g,
                                   acfg()->winbg, 160),
                 (agdp() * acfg()->btnroundsz) - 1);
    ag_roundgrad(bg, x + 2, y + 2, w - 4, h - 4, acfg()->controlbg,
                 acfg()->controlbg_g,
                 (agdp() * acfg()->btnroundsz) - 2);
    ag_roundgrad_ex(bg, x + 2, y + 2, w - 4, (h - 4) / 2,
                    LOWORD(hl1), HIWORD(hl1),
                    (agdp() * acfg()->btnroundsz) - 2, 1, 1, 0, 0);
  }
  
  int pad = agdp() * 2;
  int txtw = w - (pad * 2) - (agdp() * 20);
  int txty = ((h / 2) - (ag_fontheight(1) / 2)) + y;
  char * stripfile = aui_strip(txt, txtw, 1);
  
  if (strcmp(stripfile, "/") == 0) {
    free(stripfile);
    stripfile = strdup(alang_get("dir.root"));
  }
  else {
    stripfile[strlen(stripfile) - 1] = 0;
  }
  
  ag_text_exl(bg, txtw, x + (agdp() * 20) + pad + 1, txty + 1, stripfile,
              acfg()->controlbg, 1, 1, 0);
  ag_text_exl(bg, txtw, x + (agdp() * 20) + pad, txty, stripfile,
              acfg()->controlfg, 1, 0, 0);
              
  if (stripfile) {
    free(stripfile);
  }
}
