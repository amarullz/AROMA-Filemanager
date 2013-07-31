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
 * AROMA File Manager MAIN UI
 *
 */

//*
//* DEFINE FUNCTIONS
//*
byte aui_dispatch(AUI_VARSP v);

//*
//* SHOW TIPS
//*
void aui_showtips(AUI_VARSP v) {
  if (aui_is_show_tips) {
    aui_is_show_tips = 0;
    CANVAS * tmpc = aw_muteparent(v->hWin);
    aw_set_on_dialog(2);
    ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 180);
    ag_sync();
    CANVAS alertbg;
    ag_canvas(&alertbg, agw(), agh());
    ag_draw(&alertbg, agc(), 0, 0);
    //-- Load Tips Font
    ag_loadbigfont("fonts/HTC Hand/HTCHand.otf", 18, AROMA_DIR "/");
    PNGCANVAS ap;
    int imS = agdp() * 12;
    int bHH = v->btnH / 2;
    int w75 = round(agw() * 0.6);
    
    if (apng_load(&ap, "common/tips1")) {
      apng_stretch(&alertbg, &ap, v->pad + v->btnH,
                   v->boxY - bHH, imS, imS, 0, 0, ap.w, ap.h);
      apng_close(&ap);
    }
    
    ag_text(&alertbg, w75, v->pad + v->btnH + imS, v->boxY - bHH,
            "Here is the path of the current working directory",
            0xffff, 1);
            
    if (apng_load(&ap, "common/tips2")) {
      apng_stretch(&alertbg, &ap, agw() - (v->btnH + imS),
                   v->boxY + (v->btnH * 4.5), imS, imS, 0, 0,
                   ap.w, ap.h);
      apng_close(&ap);
    }
    
    ag_text(&alertbg, w75, agw() - (v->btnH + imS + w75),
            v->boxY + (v->btnH * 4.5),
            "<@right>Tap the checkbox to select multiple files</@>",
            0xffff, 1);
            
    if (apng_load(&ap, "common/tips3")) {
      apng_stretch(&alertbg, &ap, agw() - (v->btnH + imS * 3),
                   v->btnY - (v->pad + v->btnFH + imS), imS,
                   imS, 0, 0, ap.w, ap.h);
      apng_close(&ap);
    }
    
    ag_text(&alertbg, w75, agw() - (v->btnH + imS * 3 + w75),
            v->btnY - (v->pad + v->btnFH + (imS * 2)),
            "<@right>All functions you need is on this toolbar</@>",
            0xffff, 1);
    //-- Revert Font
    aui_cfg_reloadfonts();
    //-- Create Window
    AWINDOWP hWin = aw(&alertbg);
    aw_show(hWin);
    byte ondispatch = 1;
    
    while (ondispatch) {
      dword msg = aw_dispatch_ex(hWin, agh());
      byte msgm = aw_gm(msg);
      
      if (msgm == 5) {
        ondispatch = 0;
      }
    }
    
    aw_destroy(hWin);
    ag_ccanvas(&alertbg);
    aw_set_on_dialog(0);
    aw_unmuteparent(v->hWin, tmpc);
  }
}

//*
//* CHANGE TOOLBAR STATE
//*
byte aui_tbstate(AUI_VARSP v, byte state, byte cstate) {
  if (cstate == state) {
    return state;
  }
  
  int ics[4] = { 11, 6, 16, 10 };
  int tid[4] = { 0, 1, 2, 7 };
  byte msg[4] = { 20, 21, 22, 3 };
  
  //-- On Check not all
  if (state == 1) {
    ics[0] = 2;
    msg[0] = 30;
    tid[0] = 4;	//-- Copy
    ics[1] = 3;
    msg[1] = 31;
    tid[1] = 5;	//-- Cut
    ics[2] = 16;
    msg[2] = 22;
    tid[2] = 2;	//-- Select All
    ics[3] = 10;
    msg[3] = 3;
    tid[3] = 7;	//-- MENU
  }
  //-- On Check all
  else if (state == 2) {
    ics[0] = 2;
    msg[0] = 30;
    tid[0] = 4;	//-- Copy
    ics[1] = 3;
    msg[1] = 31;
    tid[1] = 5;	//-- Cut
    ics[2] = 19;
    msg[2] = 23;
    tid[2] = 3;	//-- unselectall
    ics[3] = 10;
    msg[3] = 3;
    tid[3] = 7;	//-- MENU
  }
  //-- On Copy / Cut
  else if (state == 3) {
    ics[0] = 13;
    msg[0] = 40;
    tid[0] = 6;	//-- Paste
    ics[1] = 0;
    msg[1] = 41;
    tid[1] = 8;	//-- Cancel
    //ics[2]=11;  msg[2]  = 20;   tid[2]  = 0;      //-- New Folder
    ics[2] = 6;
    msg[2] = 21;
    tid[2] = 1;	//-- Favorite
    ics[3] = 10;
    msg[3] = 3;
    tid[3] = 7;	//-- MENU
  }
  
  //-- Change Buttons State
  imgbtn_reinit(NULL, v->b1, 0, 0, 0, 0, &UI_ICONS[ics[0]],
                aui_tbtitle(tid[0]), 1, msg[0]);
  imgbtn_reinit(NULL, v->b2, 0, 0, 0, 0, &UI_ICONS[ics[1]],
                aui_tbtitle(tid[1]), 1, msg[1]);
  imgbtn_reinit(NULL, v->b3, 0, 0, 0, 0, &UI_ICONS[ics[2]],
                aui_tbtitle(tid[2]), 1, msg[2]);
  imgbtn_reinit(NULL, v->b4, 0, 0, 0, 0, &UI_ICONS[ics[3]],
                aui_tbtitle(tid[3]), 1, msg[3]);
  //-- Redraw Window
  aw_redraw(v->b1->win);
  ag_sync();
  //-- Reture new state
  return state;
}

//*
//* SHOW MAIN DIRECTORY LIST UI
//*
byte aui_show(char ** path, byte * state, char * selfile, byte back_ani) {
  //-- REDRAW BG
  aui_setbg(NULL);
  //-- Main Variable
  AUI_VARS v;
  //-- Return Value
  v.reshow = 0;
  //-- Init Drawing Data
  v.selfile = selfile;
  v.path = *path;
  v.state = state;
  v.pad = agdp() * 2;
  v.btnH = agdp() * 20;
  v.boxH = agh() - (aui_minY + (v.btnH * 2) + (v.pad * 4));
  v.boxY = aui_minY + v.btnH + (v.pad * 2);
  v.btnY = v.boxY + v.boxH + v.pad;
  v.navW = agw() - (v.pad * 2);
  v.pthW = v.navW;
  v.navY = aui_minY + v.pad;
  v.btnW = floor(v.navW / 4);
  v.hFileType = (*v.state == 3) ? 1 : 0;
  v.btnFH = aui_cfg_btnFH();
  //-- Draw Navigation Bar
  aui_drawtopbar(&aui_win_bg, 0, aui_minY, agw(), v.btnH + (v.pad * 2));
  aui_drawnav(&aui_win_bg, 0, v.btnY - (v.pad + v.btnFH), agw(),
              v.btnH + (v.pad * 2) + v.btnFH);
  //-- Check is ROOT Filesystem
  v.isRoot = (strcmp(v.path, "/") != 0) ? 0 : 1;
  
  //-- Justify Path UI Width
  if (!v.isRoot) {
    v.pthW -= v.btnH + v.pad;
  }
  
  //-- Draw Path UI
  aui_pathui(&aui_win_bg, v.pad, v.navY, v.pthW, v.btnH, v.path);
  //-- Create Main Window
  v.hWin = aw(&aui_win_bg);
  //****************************[ INIT WINDOW CONTROLS ]****************************/
  //-- REFRESH BUTTON
  imgbtn(v.hWin, v.pad, v.navY, v.btnH, v.btnH, &UI_ICONS[14], NULL, 2,
         10);
         
  //-- PARENT BUTTON
  if (!v.isRoot)
    imgbtn(v.hWin, agw() - (v.btnH + v.pad), v.navY, v.btnH, v.btnH,
           &UI_ICONS[12], NULL, 0, 11);
           
  //-- FILE LIST BOX
  v.hFile =
    afbox(v.hWin, 0, v.boxY, agw(), v.boxH - v.btnFH, 7, 8, v.hFileType,
          6);
  //-- TOOLS
  v.b1 = imgbtn(v.hWin, v.pad, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[11], aui_tbtitle(0), 1, 20);	//-- NEW FOLDER
  v.b2 = imgbtn(v.hWin, v.pad + v.btnW, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[6], aui_tbtitle(1), 1, 21);	//-- FAVORITE
  v.b3 = imgbtn(v.hWin, v.pad + v.btnW * 2, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[16], aui_tbtitle(2), 1, 22);	//-- SELECT ALL
  v.b4 = imgbtn(v.hWin, v.pad + v.btnW * 3, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[10], aui_tbtitle(7), 1, 3);	//-- MENU
  
  //-- SET TOOLBAR STATE
  if (*v.state != 3) {
    *v.state = 0;
  }
  
  *v.state = aui_tbstate(&v, *v.state, 0);
  //-- FETCH ITEMS FROM DIRECTORY
  aui_fetch(v.path, v.hFile, v.selfile);
  afbox_scrolltoitem(v.hFile);
  snprintf(v.selfile, 256, "");
  //-- Show Window
  aw_show_ex(v.hWin, back_ani ? 3 : 4, 0, v.hFile);
  // aw_setfocus(v.hWin,v.hFile);
  
  //-- Set Filebox Type
  if (*v.state == 3) {
    afbox_changeboxtype(v.hFile, 1);
    aui_changetitle(&v, alang_get("paste.location"));
  }
  else {
    aui_changetitle(&v, AROMA_NAME);
  }
  
  aui_showtips(&v);
  
  do {
  }
  while (aui_dispatch(&v));
  
  //-- Window
  aw_destroy(v.hWin);
  //-- Set New Path
  printf("RESHOW PATH [%s]\n", v.path);
  *path = v.path;
  return v.reshow;
}
