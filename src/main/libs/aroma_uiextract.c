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
 * AROMA File Manager : Extract File
 *
 */
#include "../../../libs/minzip/Zip.h"
typedef struct{
  char * zip_path;
  char * bname;
  char * path;
  char desttarget[256];
  char currfile[256];
  int count;
  int curr;
  byte status;
  int maxw;
  
  byte update_flags;
  
  int curr_fd;
  int curr_target;
  int curr_val;
  int curr_tick;
} AUIEXTRACT, * AUIEXTRACTP;
static pthread_mutex_t _aui_extract_mutex = PTHREAD_MUTEX_INITIALIZER;

#define EXTRACT_DIRMODE 0755
#define EXTRACT_FILEMODE 0644

void aui_extract_find_name(char * out, int sz, char * path, char * bname){
  char fmname[512];
  snprintf(fmname,512,"%s%s",path,bname);
  fmname[strlen(fmname)-4]=0;
  snprintf(out,sz,"%s",fmname);
  int try_num = 2;
  while(file_exists(out)){
    snprintf(out,sz,"%s-%i",fmname,try_num++);
  }
}
static bool aui_extract_write_fn(
  const unsigned char *data,
  int dataLen, void * cookie)
{
  AUIEXTRACTP uix = (AUIEXTRACTP) cookie;
  ssize_t n = write(uix->curr_fd, data, dataLen);
  if (n != dataLen) {
      return false;
  }
  pthread_mutex_lock(&_aui_extract_mutex);
  uix->curr_val+=n;
  uix->update_flags|=1;
  pthread_mutex_unlock(&_aui_extract_mutex);
  return true;
}

static void * aui_extract_timer_thread(void * cookie){
  AUIEXTRACTP uix=(AUIEXTRACTP) cookie;
  while (uix->status>0){
    pthread_mutex_lock(&_aui_extract_mutex);
    if ((uix->update_flags!=0)&&(uix->status==1)){
      pthread_mutex_unlock(&_aui_extract_mutex);
      atouch_send_message(110);
    }
    else{
      pthread_mutex_unlock(&_aui_extract_mutex);
    }
    usleep(100000);
  }
  return NULL;
}

static void * aui_extract_thread(void * cookie){
  AUIEXTRACTP uix=(AUIEXTRACTP) cookie;
  
  ZipArchive fzip;
  if (mzOpenZipArchive(uix->zip_path, &fzip) != 0) {
    while (!atouch_send_message(103)) {
      usleep(100000);
    }
    return NULL;
  }
  else{
    aui_extract_find_name(uix->desttarget,256,uix->path,uix->bname);
    printf("Extract Destination : %s\n",uix->desttarget);
    mkdir(uix->desttarget,EXTRACT_DIRMODE);
    if (!file_exists(uix->desttarget)){
      /* cannot make directory */
      while (!atouch_send_message(104)) {
        usleep(100000);
      }
      mzCloseZipArchive(&fzip);
      return NULL;
    }
    
    uix->count=mzZipEntryCount(&fzip);
    int i;
    for (i=0;((i<uix->count)&&(uix->status>0));i++){
      const ZipEntry* ze = mzGetZipEntryAt(&fzip,i);
      char filename[1024];
      memcpy(filename,ze->fileName,ze->fileNameLen);
      filename[ze->fileNameLen]=0;
      char * dsz = aui_strip(filename, uix->maxw, 0);
      if (dsz != NULL) {
        snprintf(filename, 1024,"%s",dsz);
        free(dsz);
      }
      pthread_mutex_lock(&_aui_extract_mutex);
      snprintf(uix->currfile,256,"%s",filename);
      uix->update_flags|=2;
      pthread_mutex_unlock(&_aui_extract_mutex);
      
      /* extract */
      char abs_entry[1024];
      snprintf(abs_entry,1024,"%s/%s",uix->desttarget,filename);
      if (ze->fileName[ze->fileNameLen-1]=='/'){
        /* directory */
        mkpath(abs_entry, EXTRACT_DIRMODE);
      }
      else{
        /* file */ 
        char * dn = dirname(abs_entry);
        mkpath(dn, EXTRACT_DIRMODE); /* create parent path */
        free(dn);
        if (mzIsZipEntrySymlink(ze)){
          /* symlink */
          if (ze->uncompLen!=0){
            char *linkTarget = malloc(ze->uncompLen+1);
            if (linkTarget!=NULL) {
              if (mzReadZipEntry(&fzip,ze, linkTarget,ze->uncompLen)){
                linkTarget[ze->uncompLen] = '\0';
                symlink(linkTarget, abs_entry);
              }
              free(linkTarget);
            }
          }
        }
        else{ 
          /* regular file */
          uix->curr_fd=open(abs_entry,O_CREAT|O_RDWR);
          if (uix->curr_fd){
            pthread_mutex_lock(&_aui_extract_mutex);
            uix->curr_target=ze->uncompLen;
            uix->curr_val=0;
            uix->update_flags|=1;
            pthread_mutex_unlock(&_aui_extract_mutex);
            mzProcessZipEntryContents(
              &fzip,ze,aui_extract_write_fn,(void *) uix);
            close(uix->curr_fd);
            chmod(abs_entry,EXTRACT_FILEMODE);
          }
          uix->curr_fd=0;
        }
      }
      
      pthread_mutex_lock(&_aui_extract_mutex);
      uix->curr=i+1;
      uix->update_flags|=4;
      if (uix->status==2){
        pthread_mutex_unlock(&_aui_extract_mutex);
        while(uix->status==2){
          usleep(16000);
        }
      }
      else{
        pthread_mutex_unlock(&_aui_extract_mutex);
      }
    }
    mzCloseZipArchive(&fzip);
  }
  while (!atouch_send_message(101)) {
    usleep(16000);
  }
  return NULL;
}
void auido_show_extract(char * zip_path, char * bname, char * path) {
  //-- Init Dialog Window
  CANVAS * tmpc = aw_muteparent(NULL);
  aw_set_on_dialog(2);
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
    (pad * 2) + titH*2 + (txtH * 3) + hpad + prgH*2 + btnH + padB + padT;
  int winY = (agh() / 2) - (winH / 2);
  int titY = winY + padT;
  int curY = titY + titH + pad;
  int defW = cliW - pad;
  int txtW1 = (int)(defW * 0.8);
  int txtW2 = defW - txtW1;
  int txtX1 = cliX + hpad;
  int txtX2 = txtX1 + txtW1;
  
  AUIEXTRACT uix;
  uix.bname=bname;
  uix.zip_path=zip_path;
  uix.path=path;
  uix.count=0;
  uix.curr=0;
  uix.currfile[0]=0;
  uix.desttarget[0]=0;
  uix.status=1;
  uix.maxw=defW;
  uix.curr_tick=alib_tickms();
  uix.curr_target=0;
  uix.curr_val=0;
  uix.curr_fd=0;
  uix.update_flags=0;
  
  //-- Draw Canvas
  if (!atheme_draw("img.dialog", &bg, winX, winY, winW, winH)) {
    ag_roundgrad(&bg, winX - 1, winY - 1, winW + 2, winH + 2,
                 acfg()->border, acfg()->border_g,
                 (acfg()->roundsz * agdp()) + 1);
    ag_roundgrad(&bg, winX, winY, winW, winH, acfg()->dialogbg,
                 acfg()->dialogbg_g, acfg()->roundsz * agdp());
  }
  
  char extract_title[256];
  snprintf(extract_title,256,"%s",bname);
  char * dsz = aui_strip(bname, uix.maxw, 0);
  if (dsz != NULL) {
    snprintf(extract_title, 256, "%s", dsz);
    free(dsz);
  }
  
  char extract_num[256];
  snprintf(
    extract_num,
    256,
    alang_get("extract.extractof"),
    uix.curr,uix.count
  );
  
  char extract_percent[16]="0 %";

  AWINDOWP hWin = aw(&bg);
  aclabel(hWin, cliX, titY, cliW, titH, alang_get("extract.extractfile"),
    1,1, 2, acfg()->winfg);
    
  ACONTROLP mainInfo =
    aclabel(hWin, cliX, titY+titH, cliW, titH, extract_title, 1,1, 2, acfg()->winfg);
  
  curY += titH;
  
  ACONTROLP entryProg =
    acprog(hWin, cliX, curY + agdp(), cliW, prgH - (agdp() * 2), 0);
  curY += prgH;
  
  ACONTROLP ovrInfo = aclabel(hWin, txtX1, curY, defW, txtH, "", 0, 1, 1,
                              acfg()->textfg_gray);
  curY += txtH;
  
  ACONTROLP ovrProg =
    acprog(hWin, cliX, curY + agdp(), cliW, prgH - (agdp() * 2), 0);
  curY += prgH;
  ACONTROLP extraInfo =
    aclabel(hWin, cliX, curY, cliW, txtH, 
    alang_get("extract.preparing"), 0,1, 2,acfg()->winfg);
  curY += txtH + hpad;
  int halfW = defW / 2;
  ACONTROLP infPrc =
    aclabel(hWin, cliX, curY, cliW, txtH, extract_percent, 0, 1, 2,
            acfg()->textfg_gray);
  curY += txtH + pad;
  imgbtn(hWin, btnX, curY, btnW, btnH, aui_icons(0), alang_get("cancel"),
         3, 55);

  aw_show_ex2(hWin, 5, winX - 1, winY - 1, winW + 2, winH + 2, NULL);
  byte ondispatch = 1;
  acprog_sethidden(entryProg, 1, 0);
  acprog_setonwait(ovrProg, 1);
  
  pthread_t th;
  pthread_create(&th, NULL, aui_extract_thread, (void *)&uix);
  pthread_detach(th);
  pthread_t th2;
  pthread_create(&th2, NULL, aui_extract_timer_thread, (void *)&uix);
  pthread_detach(th2);
  
  byte lonwait=0;
  do {
    dword msg = aw_dispatch(hWin);
    switch (aw_gm(msg)) {
      case 55: {
          /* cancel */
          pthread_mutex_lock(&_aui_extract_mutex);
          uix.status=2;
          AWMENUITEM mi[2];
          aw_menuset(mi, 0, "yes", 33);
          aw_menuset(mi, 1, "no", 0);
          byte status =
            aw_multiconfirm(hWin,
              alang_get("extract.extractfile"), 
              alang_get("extract.cancelmsg"),
              aui_icons(39),
              mi,
              2);
          uix.status=(status==1)?1:0;
          pthread_mutex_unlock(&_aui_extract_mutex);
        }
        break;
      case 110:{
          pthread_mutex_lock(&_aui_extract_mutex);
          if (uix.update_flags&1){
            /* entry progress */
            float percent_int = ((float) uix.curr_val)/((float) uix.curr_target);
            acprog_setvalue(entryProg, percent_int, 1);
          }
          if (uix.update_flags&2){ /* 100 */
            /* update filename */
            if (!lonwait){
              acprog_sethidden(entryProg, 0, 0);
              acprog_setonwait(ovrProg, 0);
              lonwait=1;
              acprog_setvalue(ovrProg,0, 0);
            }
            aclabel_settext(ovrInfo, uix.currfile, 1);
          }
          if (uix.update_flags&4){
            /* update progress */
            snprintf(
              extract_num,
              256,
              alang_get("extract.extractof"),
              uix.curr,uix.count
            );
            float percent_int = (uix.curr*100.0) / uix.count;
            snprintf(extract_percent,16,"%0.1f%c",percent_int,'%');
            aclabel_settext(infPrc, extract_percent, 0);
            aclabel_settext(extraInfo, extract_num, 0);
            acprog_setvalue(ovrProg, percent_int/100, 1);
          }
          uix.update_flags=0;
          pthread_mutex_unlock(&_aui_extract_mutex);
        }
        break;
      case 101:{
          /* finished */
          ondispatch=0;
        }
        break;
      case 103:{
          /* error */
          aw_alert(NULL, alang_get("extract.error"),
               alang_get("extract.invalidfile"),
               aui_icons(37), alang_get("close"));
          ondispatch=0;
        }
        break;
      case 104:{
          /* error */
          aw_alert(NULL, alang_get("extract.error"),
               alang_get("permission.denied"),
               aui_icons(37), alang_get("close"));
          ondispatch=0;
        }
        break;
    }
  }
  while (ondispatch);
  
  //-- Release Resources
  aw_destroy(hWin);
  ag_ccanvas(&bg);
  aw_set_on_dialog(0);
  aw_unmaskparent(NULL, tmpc, maskc, winX - 1, winY - 1, winW + 2, winH + 2);
}

void auido_extract(char * zip_path, char * bname, char * path) {
  ag_setbusy();
  auido_show_extract(zip_path,bname,path);
  return;
}
