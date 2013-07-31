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
 * AROMA File Manager : Cut File
 *
 */
byte
auido_next_del_size(AFSDTP dt, int * curr_id, byte * copy_status,
                    char ** source_path, int number_files) {
  int i;
  
  for (i = *curr_id; i < number_files; i++) {
    if (copy_status[i] == 0) {
      *curr_id = i;
      dt->status = 1;
      dt->flag = 1;
      dt->path = source_path[i];
      dt->msgFinish = aw_msg(11, 0, 0, 0);
      dt->msgTick = aw_msg(10, 0, 0, 0);
      dt->intTick = 25;
      dt->lstTick = 0;
      afs_filedelete(dt);
      return 1;
    }
  }
  
  return 0;
}

void auido_show_del(byte * copy_status, char ** source_path, int number_files) {
  //-- Init Dialog Window
  CANVAS * tmpc = aw_muteparent(NULL);
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
     [PAD]
     mainInfo
     extraInfo
     [*************perProg*************]
     perInfo
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
  int prgH = agdp() * 12;	//-- Progress Height
  int btnH = agdp() * 24;
  int btnW = winW / 2;
  int btnX = agw() / 2 - btnW / 2;
  int winH = (pad * 2) + titH + (txtH * 2) + (prgH) + btnH + padB + padT;
  int winY = (agh() / 2) - (winH / 2);
  int titY = winY + padT;
  int curY = titY + titH + pad;
  int defW = cliW - pad;
  int txtW1 = (int)(defW * 0.8);
  int txtW2 = defW - txtW1;
  int txtX1 = cliX + hpad;
  int txtX2 = txtX1 + txtW1;
  
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
  ACONTROLP mainInfo =
    aclabel(hWin, cliX, titY, cliW, titH, alang_get("delete.prepare"),
            1, 1,
            2, acfg()->winfg);
  ACONTROLP extraInfo =
    aclabel(hWin, cliX, curY, cliW, txtH, alang_get("calculating"), 0,
            1, 2,
            acfg()->winfg);
  curY += txtH;
  ACONTROLP perProg =
    acprog(hWin, cliX, curY + agdp(), cliW, prgH - (agdp() * 2), 0);
  curY += prgH;
  ACONTROLP perInfo =
    aclabel(hWin, cliX, curY, cliW, txtH, "", 0, 1, 2,
            acfg()->textfg_gray);
  curY += txtH + pad;
  imgbtn(hWin, btnX, curY, btnW, btnH, aui_icons(0), alang_get("cancel"),
         3, 55);
  //-- Show Window
  //aw_show(hWin);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  byte ondispatch = 1;
  acprog_setonwait(perProg, 1);
  //-- Start Delete Proc
  AFSDT dt;
  memset(&dt, 0, sizeof(AFSDT));
  int curr_id = 0;
  byte proc_state = 0;
  long kbps_tick = 0;
  
  if (auido_next_del_size
      (&dt, &curr_id, copy_status, source_path, number_files)) {
    //-- Dispatch
    do {
      dword msg = aw_dispatch(hWin);
      
      switch (aw_gm(msg)) {
          //-- CALCULATING
        case 10: {
            //-- Discovery Tick
            if (proc_state == 0) {
              char info[256];
              char strfl[64];
              char format[256];
              snprintf(strfl, 64, "%i", dt.n);
              snprintf(format, 256,
                       "%s ( %s )",
                       alang_get("deleting"),
                       "\%0.1f");
              snprintf(info, 256, format,
                       strfl,
                       ((float)dt.k) / 1024);
              aclabel_settext(perInfo,
                              dt.curr, 0);
              aclabel_settext(extraInfo, info,
                              1);
            }
          }
          break;
          
        case 11: {
            //-- Discovery Finish
            if (proc_state == 0) {
              curr_id++;
              
              if (!auido_next_del_size
                  (&dt, &curr_id, copy_status,
                   source_path,
                   number_files)) {
                ondispatch = 0;
                aclabel_settext
                (mainInfo,
                 alang_get
                 ("finishing"), 1);
              }
            }
            else if (proc_state == 2) {
              ondispatch = 0;
              aclabel_settext(mainInfo,
                              alang_get
                              ("finishing"),
                              1);
            }
          }
          break;
          
        case 55: {
            if (proc_state == 0) {
              aclabel_settext(mainInfo,
                              alang_get
                              ("canceling"),
                              1);
              proc_state = 2;
              dt.status = 0;
            }
          }
          break;
      }
    }
    while (ondispatch);
  }
  
  //-- Release Resources
  aw_destroy(hWin);
  ag_ccanvas(&bg);
  aw_set_on_dialog(0);
  //aw_unmuteparent(NULL, tmpc);
  aw_unmaskparent(NULL, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
}

void auido_del(char * dest, byte * tool_state) {
  ag_setbusy();
  
  if (auic_s > 0) {
    int valid_files = 0;
    int number_files = auic_s;
    byte * copy_status = malloc(number_files * sizeof(byte));
    char ** source_path = malloc(number_files * sizeof(char *));
    memset(copy_status, 0, number_files * sizeof(byte));
    memset(source_path, 0, number_files * sizeof(char *));
    int i = 0;
    
    for (i = 0; i < number_files; i++) {
      source_path[i] = auic_getpath(i);
      
      if (source_path[i] != NULL) {
        //-- Check paste validity
        copy_status[i] = 0;
        char * real_source =
          realpath(source_path[i], NULL);
          
        if (isnodelete(real_source)) {
          copy_status[i] = 1;
        }
        
        if (copy_status[i] != 1) {
          valid_files++;
        }
        
        free(real_source);
      }
      else {
        copy_status[i] = 3;
      }
    }
    
    if (valid_files > 0) {
      auido_show_del(copy_status, source_path, number_files);
    }
    
    //-- Releases
    for (i = 0; i < number_files; i++) {
      if (source_path[i] != NULL) {
        free(source_path[i]);
      }
    }
    
    free(source_path);
    free(copy_status);
  }
  
  auic_release();
  *tool_state = 0;
  return;
}
