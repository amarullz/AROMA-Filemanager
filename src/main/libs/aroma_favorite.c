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
 * AROMA File Manager Favorite UI
 *
 */
byte auifav_isfav(const char * path_src) {
  const char * path = path_src;
  char * o = aarray_get(aui_cfg_array, "favorites");
  
  if (o == NULL) {
    return 0;
  }
  
  byte ret = 0;
  char * buf = strdup(o);
  char * tok = strtok(buf, ";");
  
  while (tok) {
    if (strcmp(tok, path) == 0) {
      ret = 1;
      break;
    }
    
    tok = strtok(NULL, ";");
  }
  
  free(buf);
  return ret;
}

byte auifav_add(const char * path_src) {
  const char * path = path_src;
  
  if (!path) {
    return 0;
  }
  
  if (auifav_isfav(path)) {
    return 0;
  }
  
  char * o = aarray_get(aui_cfg_array, "favorites");
  char * q = NULL;
  int n = strlen(path) + 1;
  
  if (o != NULL) {
    n += strlen(o) + 1;
  }
  
  q = malloc(n);
  
  if (o != NULL) {
    snprintf(q, n, "%s;%s", o, path);
  }
  else {
    snprintf(q, n, "%s", path);
  }
  
  aarray_set(aui_cfg_array, "favorites", q);
  free(q);
  aui_cfg_savechange();
  return 1;
}

byte auifav_del(const char * path_src) {
  const char * path = path_src;
  
  if (!path) {
    return 0;
  }
  
  if (!auifav_isfav(path)) {
    return 0;
  }
  
  char * o = aarray_get(aui_cfg_array, "favorites");
  
  if (o == NULL) {
    return 0;
  }
  
  int n = 0;
  int wl = strlen(o) + 1;
  char * w = malloc(wl);
  w[0] = 0;
  char * buf = strdup(o);
  char * tok = strtok(buf, ";");
  
  while (tok) {
    if (strcmp(path, tok) != 0) {
      if (n > 0) {
        char * tw = strdup(w);
        snprintf(w, wl, "%s;%s", tw, tok);
        free(tw);
      }
      else {
        snprintf(w, wl, "%s", tok);
      }
      
      n++;
    }
    
    tok = strtok(NULL, ";");
  }
  
  free(buf);
  aarray_set(aui_cfg_array, "favorites", w);
  free(w);
  aui_cfg_savechange();
  return 1;
}

byte auifav_fetch(ACONTROLP FB) {
  char * o = aarray_get(aui_cfg_array, "favorites");
  
  if (o == NULL) {
    return 0;
  }
  
  int n = 0;
  char * buf = strdup(o);
  char * tok = strtok(buf, ";");
  
  while (tok) {
    char * dname = NULL;
    char * desc = NULL;
    
    if (strcmp(tok, "/") == 0) {
      dname = strdup(alang_get("dir.root"));
      desc = strdup("/");
    }
    else {
      dname = basename(tok);
      desc = aui_strip(tok, agw() - (agdp() * 46), 0);
    }
    
    afbox_add(FB, dname, desc, 0, &UI_ICONS[36], 0, "", n++, 0);
    free(desc);
    tok = strtok(NULL, ";");
  }
  
  free(buf);
  return 1;
}

char * auifav_getpath(dword id) {
  char * o = aarray_get(aui_cfg_array, "favorites");
  
  if (o == NULL) {
    return NULL;
  }
  
  int n = 0;
  char * ret = NULL;
  char * buf = strdup(o);
  char * tok = strtok(buf, ";");
  
  while (tok) {
    if (n++ == id) {
      ret = strdup(tok);
      break;
    }
    
    tok = strtok(NULL, ";");
  }
  
  free(buf);
  return ret;
}

byte
auifav_win(CANVAS * bg, char * path, char ** out_char, int titY, int titH,
           int boxY, int boxH, byte first) {
  int btnS = agdp() * 20;
  int btnP = agdp() * 2;
  AWINDOWP hWin = aw(bg);
  ACONTROLP title =
    aclabel(hWin, btnS + btnP, titY, agw() - ((btnS + btnP) * 2), titH,
            alang_get("tools.favorite"), 1, 1, 2, acfg()->winfg);
  ACONTROLP addthis =
    imgbtn(hWin, btnP, titY + btnP, btnS, btnS, aui_icons(6), NULL, 1,
           55);
  ACONTROLP cancel =
    imgbtn(hWin, agw() - (btnS + btnP), titY + btnP, btnS, btnS,
           aui_icons(0), NULL, 1, 5);
  ACONTROLP hFile = afbox(hWin, 0, boxY, agw(), boxH, 7, 8, 2, 6);
  auifav_fetch(hFile);
  aw_show_ex(hWin, first ? 1 : 100, titY, hFile);
  byte ondispatch = 1;
  byte reshow = 0;
  
  do {
    dword msg = aw_dispatch_ex(hWin, titY);
    
    switch (aw_gm(msg)) {
      case 5: {
          //-- DONE
          ondispatch = 0;
        }
        break;
        
      case 55: {
          //-- REFRESH
          ondispatch = 0;
          reshow = 1;
        }
        break;
        
        //-- ITEM CLICK
      case 7: {
          dword fl = afbox_ddata(hFile);
          char * fpath = auifav_getpath(fl);
          
          if (fpath != NULL) {
            aui_setpath(out_char, fpath, "", 0);
            free(fpath);
            ondispatch = 0;
          }
        }
        break;
        
        //-- ITEM TAP & HOLD
      case 8: {
          dword fl = afbox_ddata(hFile);
          char * fpath = auifav_getpath(fl);
          
          if (fpath != NULL) {
            int cp = 0;
            AWMENUITEM mi[2];
            aw_menuset(mi, cp++, "select", 33);
            aw_menuset(mi, cp++,
                       "tools.favorite.del", 8);
            byte ret = aw_menu(hWin, NULL, mi, cp);
            
            if (ret == 2) {
              auifav_del(fpath);
              reshow = 1;
              free(fpath);
              ondispatch = 0;
            }
            else if (ret == 1) {
              aui_setpath(out_char, fpath, "",
                          0);
              ondispatch = 0;
            }
            
            free(fpath);
          }
        }
        break;
    }
  }
  while (ondispatch);
  
  aw_destroy(hWin);
  return reshow;
}

char * auifav(AWINDOWP parent, char * path) {
  //-- Mute Parent
  CANVAS * tmpc = aw_muteparent(parent);
  aw_set_on_dialog(2);
  ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 180);
  ag_sync();
  //-- Initializing Canvas
  CANVAS favbg;
  ag_canvas(&favbg, agw(), agh());
  ag_draw(&favbg, agc(), 0, 0);
  //-- Init Sizes
  int winY = agh() - round(agh() / 1.618);
  int winH = agh() - winY;
  int titH = agdp() * 24;	// (agdp()*6) + ag_fontheight(1);
  int boxY = winY + titH;
  int boxH = winH - titH;
  //-- Draw Title
  ag_roundgrad_ex(&favbg, 0, winY, agw(), titH, acfg()->navbg,
                  acfg()->navbg_g, 0, 0, 0, 0, 0);
  //-- Draw Main Window
  ag_roundgrad_ex(&favbg, 0, boxY, agw(), boxH, acfg()->navbg,
                  acfg()->navbg_g, 0, 0, 0, 0, 0);
  byte first = 1;
  char * out_char = NULL;
  
  while (auifav_win
         (&favbg, path, &out_char, winY, titH, boxY, boxH, first)) {
    first = 0;
  }
  
  //-- Cleanup Canvas
  ag_ccanvas(&favbg);
  //-- Unmute Parent
  aw_set_on_dialog(0);
  aw_unmuteparent(parent, tmpc);
  return out_char;
}
