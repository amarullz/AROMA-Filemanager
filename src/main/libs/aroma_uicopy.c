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
 * AROMA File Manager : Copy File
 *
 */
byte
auido_next_copy_size(AFSDTP dt, int * curr_id, byte * copy_status,
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
      afs_filesize(dt);
      return 1;
    }
  }
  
  return 0;
}

byte
auido_next_copy_action(AFSDTP dt, int * curr_id, byte * copy_status,
                       char ** source_path, char ** dest_path,
                       int number_files, byte flag) {
  int i;
  
  for (i = *curr_id; i < number_files; i++) {
    if (copy_status[i] == 0) {
      *curr_id = i;
      dt->status = 1;
      dt->flag = flag;
      dt->path = source_path[i];
      dt->dest = dest_path[i];
      dt->msgTick = aw_msg(20, 0, 0, 0);
      dt->msgOvrwrt = aw_msg(21, 0, 0, 0);
      dt->msgFull = aw_msg(22, 0, 0, 0);
      dt->msgFinish = aw_msg(23, 0, 0, 0);
      dt->intTick = 50;
      dt->lstTick = 0;
      afs_copy(dt);
      return 1;
    }
  }
  
  return 0;
}

byte auido_copy_different_name(char ** dest) {
  int i;
  int eps = 0;
  int dsl = strlen(*dest);
  char * dst = *dest;
  char * ext = malloc(dsl + 1);
  char * ful = malloc(dsl + 1);
  memset(ext, 0, dsl + 1);
  memset(ful, 0, dsl + 1);
  
  for (i = 0; i < dsl; i++) {
    if (eps == 0) {
      if (dst[i] == '.') {
        ext[eps++] = '.';
      }
      else {
        ful[i] = dst[i];
      }
    }
    else {
      ext[eps++] = dst[i];
    }
  }
  
  i = 1;
  byte ret = 0;
  
  do {
    char * buf = malloc(dsl + 32);
    memset(buf, 0, dsl + 32);
    
    if (i == 1) {
      snprintf(buf, dsl + 32, "%s-copy%s", ful, ext);
    }
    else {
      snprintf(buf, dsl + 32, "%s-copy-%i%s", ful, i, ext);
    }
    
    if (!file_exists(buf)) {
      free(*dest);
      *dest = strdup(buf);
      free(buf);
      ret = 1;
      break;
    }
    
    free(buf);
    i++;
  }
  while (i < 10);
  
  free(ext);
  free(ful);
  return ret;
}

void
auido_show_copy(byte * copy_status, char ** source_path, char ** dest_path,
                int number_files) {
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
     perInfo                    perPrc
     [HALF-PAD]
     [*************ovrProg*************]
     ovrInfo                    ovrPrc
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
  int winH =
    (pad * 2) + titH + (txtH * 4) + (hpad * 2) + (prgH * 2) + btnH +
    padB + padT;
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
    aclabel(hWin, cliX, titY, cliW, titH, alang_get("copy.prepare"), 1,
            1,
            2, acfg()->winfg);
  ACONTROLP extraInfo =
    aclabel(hWin, cliX, curY, cliW, txtH, alang_get("calculating"), 0,
            1, 2,
            acfg()->winfg);
  curY += txtH;
  ACONTROLP perProg =
    acprog(hWin, cliX, curY + agdp(), cliW, prgH - (agdp() * 2), 0);
  curY += prgH;
  ACONTROLP perInfo = aclabel(hWin, txtX1, curY, txtW1, txtH, "", 0, 1, 1,
                              acfg()->textfg_gray);
  ACONTROLP perPrc =
    aclabel(hWin, txtX2, curY, txtW2, txtH, "", 0, 1, 3, acfg()->winfg);
  curY += txtH + hpad;
  ACONTROLP ovrProg =
    acprog(hWin, cliX, curY + agdp(), cliW, prgH - (agdp() * 2), 0);
  curY += prgH;
  ACONTROLP ovrInfo = aclabel(hWin, txtX1, curY, txtW1, txtH, "", 0, 1, 1,
                              acfg()->textfg_gray);
  ACONTROLP ovrPrc =
    aclabel(hWin, txtX2, curY, txtW2, txtH, "", 0, 1, 3, acfg()->winfg);
  curY += txtH + hpad;
  int halfW = defW / 2;
  ACONTROLP infPrc =
    aclabel(hWin, cliX, curY, cliW, txtH, "", 0, 1, 2,
            acfg()->textfg_gray);
  curY += txtH + pad;
  imgbtn(hWin, btnX, curY, btnW, btnH, aui_icons(0), alang_get("cancel"),
         3, 55);
  acprog_sethidden(perProg, 1, 0);
  //-- Show Window
  // aw_show(hWin);
  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  byte ondispatch = 1;
  acprog_setonwait(ovrProg, 1);
  //-- Start Copy Proc
  AFSDT dt;
  AFSDT ct;
  memset(&dt, 0, sizeof(AFSDT));
  memset(&ct, 0, sizeof(AFSDT));
  int curr_id = 0;
  int copy_id = 0;
  byte proc_state = 0;
  long kbps_tick = 0;
  
  if (auido_next_copy_size
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
              char nicebuf[64];
              snprintf(strfl, 64, "%i", dt.n);
              snprintf(format, 256,
                       "%s ( %s )",
                       alang_get
                       ("discovered"), "\%s");
              snprintf(info, 256, format,
                       strfl,
                       aui_kilosize(nicebuf,
                                    dt.k));
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
              
              if (!auido_next_copy_size
                  (&dt, &curr_id, copy_status,
                   source_path,
                   number_files)) {
                char info[256];
                char nicebuf[64];
                afs_setoftitle(info,
                               "copying",
                               0, dt.n,
                               NULL,
                               NULL);
                aclabel_settext
                (mainInfo, info, 0);
                aclabel_settext(perInfo,
                                "", 0);
                afs_setoftitle(info,
                               "progress.of",
                               0, 0,
                               NULL,
                               aui_kilosize
                               (nicebuf,
                                dt.k));
                aclabel_settext
                (extraInfo, info,
                 0);
                acprog_setonwait
                (ovrProg, 0);
                acprog_sethidden
                (ovrProg, 0, 0);
                acprog_sethidden
                (perProg, 0, 0);
                snprintf(info, 256,
                         "0%c", '%');
                aclabel_settext(ovrPrc,
                                info,
                                0);
                aclabel_settext(perPrc,
                                info,
                                0);
                snprintf(info, 256,
                         "-- Bytes/%s",
                         alang_get
                         ("seconds"));
                aclabel_settext(ovrInfo,
                                info,
                                0);
                                
                if (!auido_next_copy_action(&ct, &copy_id, copy_status, source_path, dest_path, number_files, 0)) {
                  ct.flag = 0;
                  ondispatch = 0;
                }
                else {
                  kbps_tick =
                    alib_tick();
                  proc_state = 1;
                }
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
          
          //-- COPYING
        case 21: {
            if (proc_state == 1) {
              long wait_tick = alib_tick();
              afs_showOverwrite(hWin, &ct);
              kbps_tick +=
                alib_tick() - wait_tick;
            }
          }
          break;
          
        case 22: {
            if (proc_state == 1) {
              long wait_tick = alib_tick();
              afs_showDiskfull(hWin, &ct);
              kbps_tick +=
                alib_tick() - wait_tick;
            }
          }
          break;
          
        case 20: {
            if (proc_state == 1) {
              char nicebuf[64];
              char nicebuf2[64];
              char info[256];
              afs_setoftitle(info, "copying",
                             ct.n, dt.n, NULL,
                             NULL);
              aclabel_settext(perInfo,
                              ct.curr, 0);
              aclabel_settext(mainInfo, info,
                              0);
              afs_setoftitle(info,
                             "progress.of", 0,
                             0,
                             aui_kilosize
                             (nicebuf, ct.k),
                             aui_kilosize
                             (nicebuf2,
                              dt.k));
              aclabel_settext(extraInfo, info,
                              0);
              float prc = 0;
              
              if (ct.cs > 0)
                prc =
                  ((float)ct.cr) /
                  ((float)ct.cs);
                  
              acprog_setvalue(perProg, prc,
                              0);
              snprintf(info, 256, "%0.1f%c",
                       prc * 100, '%');
              aclabel_settext(perPrc, info,
                              0);
              float cpspeed =
                ((float)
                 (alib_tick() -
                  kbps_tick)) / 100.0;
                  
              if (cpspeed <= 0) {
                cpspeed = 0.001;
              }
              
              int kbps_avr =
                round(ct.k / cpspeed);
              snprintf(info, 256, "%s/%s",
                       aui_kilosize(nicebuf,
                                    kbps_avr),
                       alang_get("seconds"));
              aclabel_settext(ovrInfo, info,
                              0);
              int kbleft = dt.k - ct.k;
              int remain = 0;
              
              if (kbps_avr > 0)
                remain =
                  round(kbleft /
                        kbps_avr);
                        
              int rem_hou = remain / 3600;
              int rem_min =
                (remain % 3600) / 60;
              int rem_sec =
                (remain % 3600) % 60;
                
              if (rem_hou > 0)
                snprintf(info, 256,
                         "%i %s %i %s %s",
                         rem_hou,
                         alang_get
                         ("hours"),
                         rem_min,
                         alang_get
                         ("minutes"),
                         alang_get
                         ("remaining"));
              else if (rem_min > 0)
                snprintf(info, 256,
                         "%i %s %i %s %s",
                         rem_min,
                         alang_get
                         ("minutes"),
                         rem_sec,
                         alang_get
                         ("seconds"),
                         alang_get
                         ("remaining"));
              else
                snprintf(info, 256,
                         "%i %s %s",
                         rem_sec,
                         alang_get
                         ("seconds"),
                         alang_get
                         ("remaining"));
                         
              aclabel_settext(infPrc, info,
                              0);
              prc = 0;
              
              if (dt.k > 0)
                prc =
                  ((float)ct.k) /
                  ((float)dt.k);
                  
              acprog_setvalue(ovrProg, prc,
                              0);
              snprintf(info, 256, "%0.1f%c",
                       prc * 100, '%');
              aclabel_settext(ovrPrc, info,
                              1);
            }
          }
          break;
          
        case 23: {
            if (proc_state == 1) {
              copy_id++;
              
              if (!auido_next_copy_action
                  (&ct, &copy_id, copy_status,
                   source_path, dest_path,
                   number_files, 0)) {
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
            else if (proc_state == 1) {
              aclabel_settext(mainInfo,
                              alang_get
                              ("canceling"),
                              1);
              proc_state = 2;
              ct.status = 0;
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

void auido_copy(char * dest, byte * tool_state) {
  ag_setbusy();
  byte release_clipboard = 0;
  
  if (auic_s > 0) {
    int valid_files = 0;
    int number_files = auic_s;
    byte * copy_status = malloc(number_files * sizeof(byte));
    char ** source_path = malloc(number_files * sizeof(char *));
    char ** dest_path = malloc(number_files * sizeof(char *));
    memset(copy_status, 0, number_files * sizeof(byte));
    memset(source_path, 0, number_files * sizeof(char *));
    memset(dest_path, 0, number_files * sizeof(char *));
    byte not_valid = 0;
    int i = 0;
    
    for (i = 0; i < number_files; i++) {
      source_path[i] = auic_getpath(i);
      dest_path[i] = auic_destpath(i, dest);
      
      if ((source_path[i] != NULL) && (dest_path[i] != NULL)) {
        //-- Check paste validity
        copy_status[i] = 0;
        char * real_source =
          realpath(source_path[i], NULL);
        char * dest_dn = dirname(dest_path[i]);
        char * dest_bn = basename(dest_path[i]);
        char * real_dn = realpath(dest_dn, NULL);
        int real_dest_n =
          strlen(real_dn) + strlen(dest_bn) + 2;
        char * real_dest = malloc(real_dest_n);
        snprintf(real_dest, real_dest_n, "%s/%s",
                 real_dn, dest_bn);
        free(real_dn);
        free(dest_bn);
        free(dest_dn);
        int lspath = strlen(real_source);
        
        if (lspath < strlen(real_dest)) {
          char * ckd = strdup(real_dest);
          
          if ((ckd[lspath] == 0)
              || (ckd[lspath] == '/')) {
            ckd[lspath] = 0;
            
            if (strcmp(ckd, real_source) ==
                0) {
              copy_status[i] = 1;
              not_valid = 1;
            }
          }
          
          free(ckd);
        }
        
        if (strcmp(real_dest, real_source) == 0) {
          if (!auido_copy_different_name
              (&dest_path[i])) {
            copy_status[i] = 1;
          }
        }
        
        if (copy_status[i] != 1) {
          valid_files++;
        }
        
        free(real_source);
        free(real_dest);
      }
      else {
        copy_status[i] = 3;
      }
    }
    
    if (not_valid) {
      aw_alert(NULL, alang_get("paste.error"),
               alang_get("paste.subfolder_paste"),
               aui_icons(37), alang_get("cancel"));
    }
    else if (valid_files > 0) {
      auido_show_copy(copy_status, source_path, dest_path,
                      number_files);
      release_clipboard = 1;
    }
    else {
      aw_alert(NULL, alang_get("paste.error"),
               alang_get("paste.nofile"), aui_icons(37),
               alang_get("cancel"));
    }
    
    //-- Releases
    for (i = 0; i < number_files; i++) {
      if (source_path[i] != NULL) {
        free(source_path[i]);
      }
      
      if (dest_path[i] != NULL) {
        free(dest_path[i]);
      }
    }
    
    free(source_path);
    free(dest_path);
    free(copy_status);
  }
  
  if (release_clipboard) {
    auic_release();
    *tool_state = 0;
  }
  
  return;
}
