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
 * AROMA File Manager : Permission File
 *
 */
byte auido_setperm(AWINDOWP parent, char * path, char * fn) {
  struct stat fst;
  byte buf[12];
  memset(buf, 0, 12);
  
  if (!stat(path, &fst)) {
    buf[11] = (fst.st_mode & S_IXOTH) ? 1 : 0;
    buf[10] = (fst.st_mode & S_IWOTH) ? 1 : 0;
    buf[9]  = (fst.st_mode & S_IROTH) ? 1 : 0;
    buf[8] = (fst.st_mode & S_IXGRP) ? 1 : 0;
    buf[7] = (fst.st_mode & S_IWGRP) ? 1 : 0;
    buf[6] = (fst.st_mode & S_IRGRP) ? 1 : 0;
    buf[5] = (fst.st_mode & S_IXUSR) ? 1 : 0;
    buf[4] = (fst.st_mode & S_IWUSR) ? 1 : 0;
    buf[3] = (fst.st_mode & S_IRUSR) ? 1 : 0;
    buf[2] = (fst.st_mode & S_ISVTX) ? 1 : 0;
    buf[1] = (fst.st_mode & S_ISGID) ? 1 : 0;
    buf[0] = (fst.st_mode & S_ISUID) ? 1 : 0;
  }
  
  //-- Init Dialog Window
  CANVAS * tmpc = aw_muteparent(parent);
  aw_set_on_dialog(2);
  //ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 180);
  CANVAS * maskc = aw_maskparent();
  ag_sync();
  //-- Initializing Canvas
  CANVAS bg;
  ag_canvas(&bg, agw(), agh());
  ag_draw(&bg, agc(), 0, 0);
  //-- Size & Position
  /*
     MainTitle
     SubTitle
     [PAD]
     user  group others
     [x] r [x] w [x] x
     [x] r [x] w [x] x
     [x] r [x] w [x] x
     [x] s [x] g [x] t
     [PAD]
     [ BUTTONS ]
   */
  int pad  = agdp() * 4;
  int padB = pad;
  int padT = pad;
  int padL = pad;
  int padR = pad;
  PNGCANVASP winp = atheme("img.dialog");
  APNG9 winv;
  
  if (winp != NULL) {
    if (apng9_calc(winp, &winv, 1)) {
      padL = winv.l;
      padR = winv.r;
      padB = winv.b;
      padT = winv.t;
    }
  }
  
  int hpad = agdp() * 2;
  int winW = agw() - (pad * 2);	//-- Window
  int winX = pad;
  int cliW = winW - (padL + padR + (hpad * 2));	//-- Window Client
  int cliX = pad + padL + hpad;
  int titH = ag_fontheight(1) + (agdp() * 2);	//-- Title Height
  int chkH = agdp() * 20;
  int txtH = ag_fontheight(0) + agdp();	      //-- Text Interface Height
  int btnH = agdp() * 24;
  int cliH = titH + txtH + (chkH * 4) + btnH + (pad);
  int winH = cliH + padT + padB;
  int winY = (agh() / 2) - (winH / 2);
  int cliY = winY + padT;
  int _Y   = cliY;
  
  //-- Draw Canvas
  if (!atheme_draw("img.dialog", &bg, winX, winY, winW, winH)) {
    ag_roundgrad(&bg, winX - 1, winY - 1, winW + 2, winH + 2,
                 acfg()->border, acfg()->border_g,
                 (acfg()->roundsz * agdp()) + 1);
    ag_roundgrad(&bg, winX, winY, winW, winH, acfg()->dialogbg,
                 acfg()->dialogbg_g, acfg()->roundsz * agdp());
  }
  
  //-- Init Window & Controls
  AWINDOWP hWin = aw(&bg);
  ACONTROLP txtTitle =
    aclabel(hWin, cliX, _Y, cliW, titH,
            alang_get("tools.chmod"), 1, 1, 2, acfg()->winfg);
  _Y += titH;
  ACONTROLP txtFilename =
    aclabel(hWin, cliX, _Y, cliW, txtH,
            fn, 0, 1, 2, acfg()->winfg);
  _Y += txtH + pad;
  int colW = cliW / 4;
  int col1 = cliX;
  int col2 = col1 + colW;
  int col3 = col2 + colW;
  int col4 = col3 + colW;
  aclabel(hWin, col1, _Y, colW, chkH,
          alang_get("tools.chmod.user"), 0, 1, 1, acfg()->winfg);
  ACONTROLP cbR1 = accb(hWin, col2, _Y, colW, chkH, alang_get("tools.chmod.read"), buf[3]);
  ACONTROLP cbW1 = accb(hWin, col3, _Y, colW, chkH, alang_get("tools.chmod.write"), buf[4]);
  ACONTROLP cbX1 = accb(hWin, col4, _Y, colW, chkH, alang_get("tools.chmod.exec"), buf[5]);
  _Y += chkH;
  aclabel(hWin, col1, _Y, colW, chkH,
          alang_get("tools.chmod.group"), 0, 1, 1, acfg()->winfg);
  ACONTROLP cbR2 = accb(hWin, col2, _Y, colW, chkH, alang_get("tools.chmod.read"), buf[6]);
  ACONTROLP cbW2 = accb(hWin, col3, _Y, colW, chkH, alang_get("tools.chmod.write"), buf[7]);
  ACONTROLP cbX2 = accb(hWin, col4, _Y, colW, chkH, alang_get("tools.chmod.exec"), buf[8]);
  _Y += chkH;
  aclabel(hWin, col1, _Y, colW, chkH,
          alang_get("tools.chmod.others"), 0, 1, 1, acfg()->winfg);
  ACONTROLP cbR3 = accb(hWin, col2, _Y, colW, chkH, alang_get("tools.chmod.read"), buf[9]);
  ACONTROLP cbW3 = accb(hWin, col3, _Y, colW, chkH, alang_get("tools.chmod.write"), buf[10]);
  ACONTROLP cbX3 = accb(hWin, col4, _Y, colW, chkH, alang_get("tools.chmod.exec"), buf[11]);
  _Y += chkH;
  ACONTROLP cbZ1 = accb(hWin, col2, _Y, colW, chkH, alang_get("tools.chmod.setuid"), buf[0]);
  ACONTROLP cbZ2 = accb(hWin, col3, _Y, colW, chkH, alang_get("tools.chmod.setgid"), buf[1]);
  ACONTROLP cbZ3 = accb(hWin, col4, _Y, colW, chkH, alang_get("tools.chmod.sticky"), buf[2]);
  _Y += chkH;
  int btnW = cliW / 2;
  int btn1 = cliX;
  int btn2 = btn1 + btnW;
  ACONTROLP okBtn =
    imgbtn(hWin, btn1, _Y, btnW - (pad / 2), btnH, aui_icons(33), alang_get("ok"), 3, 6);
  ACONTROLP cancelBtn =
    imgbtn(hWin, btn2 + (pad / 2), _Y, btnW - (pad / 2), btnH, aui_icons(0), alang_get("cancel"), 3, 5);
  //-- Show Window
  //aw_show_ex(hWin, 0, 0, okBtn);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, okBtn);
  byte ondispatch = 1;
  byte retval = 1;
  
  //-- Dispatch
  do {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 5: {
          ondispatch = 0;
        }
        break;
        
      case 6: {
          // ondispatch = 0;
          buf[0] = accb_ischecked(cbZ1);
          buf[1] = accb_ischecked(cbZ2);
          buf[2] = accb_ischecked(cbZ3);
          buf[3] = accb_ischecked(cbR1);
          buf[4] = accb_ischecked(cbW1);
          buf[5] = accb_ischecked(cbX1);
          buf[6] = accb_ischecked(cbR2);
          buf[7] = accb_ischecked(cbW2);
          buf[8] = accb_ischecked(cbX2);
          buf[9] = accb_ischecked(cbR3);
          buf[10] = accb_ischecked(cbW3);
          buf[11] = accb_ischecked(cbX3);
          mode_t newmod = 0;
          
          if (buf[0]) {
            newmod |= S_ISUID;
          }
          
          if (buf[1]) {
            newmod |= S_ISGID;
          }
          
          if (buf[2]) {
            newmod |= S_ISVTX;
          }
          
          if (buf[3]) {
            newmod |= S_IRUSR;
          }
          
          if (buf[4]) {
            newmod |= S_IWUSR;
          }
          
          if (buf[5]) {
            newmod |= S_IXUSR;
          }
          
          if (buf[6]) {
            newmod |= S_IRGRP;
          }
          
          if (buf[7]) {
            newmod |= S_IWGRP;
          }
          
          if (buf[8]) {
            newmod |= S_IXGRP;
          }
          
          if (buf[9]) {
            newmod |= S_IROTH;
          }
          
          if (buf[10]) {
            newmod |= S_IWOTH;
          }
          
          if (buf[11]) {
            newmod |= S_IXOTH;
          }
          
          printf("CHMOD %o - %s\n", newmod, path);
          chmod(path, newmod);
          ondispatch = 0;
          retval = 0;
        }
        break;
    }
  }
  while (ondispatch);
  
  //-- Release Resources
  aw_destroy(hWin);
  ag_ccanvas(&bg);
  aw_set_on_dialog(0);
  //aw_unmuteparent(parent, tmpc);
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
  return retval;
}
