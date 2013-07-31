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
 * AROMA File Manager : About Dialog
 *
 */
void auido_about_dialog(AWINDOWP parent) {
  //-- Init Dialog Window
  CANVAS * tmpc = aw_muteparent(parent);
  aw_set_on_dialog(2);
  CANVAS * maskc = aw_maskparent();
  //ag_rectopa(agc(), 0, 0, agw(), agh(), 0x0000, 180);
  ag_sync();
  //-- Initializing Canvas
  CANVAS bg;
  ag_canvas(&bg, agw(), agh());
  ag_draw(&bg, agc(), 0, 0);
  //-- Size & Position
  /*
     [PAD]
     [LOGO]  AROMA File Manager
     Date and Codename
     Copy Info
     [PAD]
     [*********************************]
     [                                 ]
     [             LICENSE             ]
     [                                 ]
     [*********************************]
     [PAD]
     [ CANCEL BUTTON ]
     [PAD]
   */
  int pad = agdp() * 4;
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
  int cliW = winW - (padL + padR);	//-- Window Client
  int cliX = pad + padL;
  int titH = ag_fontheight(1) + (agdp() * 2);	//-- Title Height
  int txtH = ag_fontheight(0) + agdp();	//-- Text Interface Height
  int btnH = agdp() * 24;
  int padH = (pad * 2) + titH + (txtH * 2) + btnH + padB + padT;
  int winH = round(agh() / 1.618);
  int boxH = winH - padH;
  int winY = (agh() / 2) - (winH / 2);
  int titY = winY + padT;
  int curY = titY + titH;
  int defW = cliW - pad;
  int txtX = cliX + hpad;
  
  //-- Draw Canvas
  if (!atheme_draw("img.dialog", &bg, winX, winY, winW, winH)) {
    ag_roundgrad(&bg, winX - 1, winY - 1, winW + 2, winH + 2,
                 acfg()->border, acfg()->border_g,
                 (acfg()->roundsz * agdp()) + 1);
    ag_roundgrad(&bg, winX, winY, winW, winH, acfg()->dialogbg,
                 acfg()->dialogbg_g, acfg()->roundsz * agdp());
  }
  
  //-- Draw Logo
  PNGCANVAS imlogo;
  apng_load(&imlogo, "common/logo");
  int imgS = agdp() * 24;
  int imgP = imgS + pad;
  apng_stretch(&bg,
               &imlogo, cliX, titY, imgS, imgS, 0, 0, imlogo.w, imlogo.h);
  apng_close(&imlogo);
  //-- Init Window & Controls
  AWINDOWP hWin = aw(&bg);
  ACONTROLP txtTitle = aclabel(hWin, cliX + imgP, titY, cliW - imgP, titH,
                               AROMA_NAME " " AROMA_VERSION, 1, 1, 1,
                               acfg()->winfg);
  ACONTROLP txtCopy =
    aclabel(hWin, cliX + imgP, curY, cliW - imgP, txtH, AROMA_COPY, 0,
            1, 1,
            acfg()->winfg);
  curY += txtH;
  ACONTROLP txtVersion =
    aclabel(hWin, cliX + imgP, curY, cliW - imgP, txtH,
            AROMA_BUILD_CN " (" AROMA_BUILD ")", 0, 1, 1,
            acfg()->textfg_gray);
  curY += txtH + pad;
  char * licensetxt = aui_readfromzip(AROMA_DIR "/common/license.txt");
  ACONTROLP txtLicense = actext(hWin, cliX, curY, cliW, boxH,
                                licensetxt ? licensetxt :
                                "<@center>Cannot read license text...</@>",
                                0);
                                
  if (licensetxt) {
    free(licensetxt);
  }
  
  curY += boxH + pad;
  int webW = floor(cliW / 1.618);
  int btnW = (cliW - webW);
  int btnX = cliX + webW;
  ACONTROLP txtWeb =
    aclabel(hWin, cliX, curY, webW, btnH,
            AROMA_BUILD_URL "\n" AROMA_BUILD_A,
            0, 1, 0, acfg()->textfg_gray);
  ACONTROLP okBtn =
    imgbtn(hWin, btnX, curY, btnW, btnH, aui_icons(33), alang_get("ok"),
           3,
           5);
  //-- Show Window
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, okBtn);
  // aw_show_ex(hWin, 0, 0, okBtn);
  byte ondispatch = 1;
  
  //-- Dispatch
  do {
    dword msg = aw_dispatch(hWin);
    
    switch (aw_gm(msg)) {
      case 5: {
          ondispatch = 0;
        }
        break;
    }
  }
  while (ondispatch);
  
  //-- Release Resources
  aw_destroy(hWin);
  ag_ccanvas(&bg);
  aw_set_on_dialog(0);
  // aw_unmuteparent(parent, tmpc);
  aw_unmaskparent(parent, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
}
