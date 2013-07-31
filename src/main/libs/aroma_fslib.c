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
 * AROMA File Manager - Filesystem LIB
 *
 */
#include <pthread.h>

#define AFS_KILOBYTE 1024
typedef struct {
  byte status;		//-- Operation Status
  
  byte flag;		//-- Flag
  const char * path;	//-- Source Path
  const char * dest;	//-- Destination Path
  
  char curr[256];		//-- Current Proccessed File
  
  int k;			//-- Size in KB
  int b;			//-- Size in Bytes
  int n;			//-- Number of Files
  int cs;			//-- Current Size
  int cr;			//-- Current Readed
  
  dword msgFinish;	//-- Finish Message
  dword msgTick;		//-- Tick Message
  dword msgOvrwrt;	//-- Overwrite Message
  dword msgFull;		//-- Diskfull Message
  
  int intTick;		//-- Tick Interval
  long lstTick;		//-- Last Tick
  
  byte overwrite;		//-- Overwrite type, 0=No Overwrite, 1=Skip All, 2=Overwrite once,  3=Overwrite All
  pthread_mutex_t ovrMutex;
  pthread_cond_t ovrCond;
} AFSDT, *AFSDTP;

void
afs_setoftitle(char * buf, char * format, int v1, int v2, char * sv1, char * sv2) {
  char st1[64];
  char st2[64];
  
  if (sv1 != NULL) {
    snprintf(st1, 64, "%s", sv1);
  }
  else {
    snprintf(st1, 64, "%i", v1);
  }
  
  if (sv2 != NULL) {
    snprintf(st2, 64, "%s", sv2);
  }
  else {
    snprintf(st2, 64, "%i", v2);
  }
  
  snprintf(buf, 256, alang_get(format), st1, st2);
}

byte afs_tick(AFSDTP dt) {
  if ((dt->status != 1) || (!dt->msgTick)) {
    return 0;
  }
  
  long ctick = alib_tick();
  
  if (dt->lstTick < (ctick - dt->intTick)) {
    dt->lstTick = ctick;
    return atouch_send_message(dt->msgTick);
  }
  
  return 0;
}

byte afs_askoverwrite(AFSDTP dt) {
  if (dt->overwrite == 1) {
    return 0;
  }
  
  if (dt->overwrite == 3) {
    return 1;
  }
  
  pthread_mutex_lock(&dt->ovrMutex);
  
  while (!atouch_send_message(dt->msgOvrwrt)) {
    usleep(100000);
  }
  
  pthread_cond_wait(&dt->ovrCond, &dt->ovrMutex);
  pthread_mutex_unlock(&dt->ovrMutex);
  
  if (dt->overwrite < 2) {
    return 0;
  }
  
  return 1;
}

void afs_askdiskfull(AFSDTP dt) {
  pthread_mutex_lock(&dt->ovrMutex);
  
  while (!atouch_send_message(dt->msgFull)) {
    usleep(100000);
  }
  
  pthread_cond_wait(&dt->ovrCond, &dt->ovrMutex);
  pthread_mutex_unlock(&dt->ovrMutex);
}

byte afs_setOverwrite(AFSDTP dt, byte overwrite) {
  pthread_mutex_lock(&dt->ovrMutex);
  dt->overwrite = overwrite;
  pthread_cond_signal(&dt->ovrCond);
  pthread_mutex_unlock(&dt->ovrMutex);
  return 1;
}

byte afs_showOverwrite(AWINDOWP hWin, AFSDTP dt) {
  AWMENUITEM mi[4];
  aw_menuset(mi, 0, "overwrite.skip", 0);
  aw_menuset(mi, 1, "overwrite.skipall", 19);
  aw_menuset(mi, 2, "overwrite", 33);
  aw_menuset(mi, 3, "overwrite.all", 16);
  char info[300];
  snprintf(info, 300, "%s\n\n%s", alang_get("file.exists.info"),
           dt->curr);
  byte ovr =
    aw_multiconfirm(hWin, alang_get("file.exists"), info, aui_icons(39),
                    mi, 4);
  afs_setOverwrite(dt, ovr);
  return ovr;
}

void afs_showDiskfull(AWINDOWP hWin, AFSDTP dt) {
  AWMENUITEM mi[2];
  aw_menuset(mi, 0, "cancel", 0);
  aw_menuset(mi, 1, "continue", 33);
  char info[300];
  snprintf(info, 300, "%s\n\n%s", alang_get("disk.full.info"), dt->curr);
  byte status =
    aw_multiconfirm(hWin, alang_get("disk.full"), info, aui_icons(37),
                    mi,
                    2);
  pthread_mutex_lock(&dt->ovrMutex);
  dt->status = status;
  pthread_cond_signal(&dt->ovrCond);
  pthread_mutex_unlock(&dt->ovrMutex);
}

void afs_filesize_do(AFSDTP dt, const char * path, byte ischild) {
  //-- Define
  struct stat fstat;
  struct stat lfstat;
  byte islnk = 0;
  int sz = 0;
  
  //-- Get Stat
  if (stat(path, &fstat)) {
    return;
  }
  
  //-- Set Current Info
  if (ischild) {
    //-- Get Link Stat
    if ((lstat(path, &lfstat) == 0) && S_ISLNK(lfstat.st_mode)) {
      sz = lfstat.st_size;
      islnk = 1;
    }
    else {
      sz = fstat.st_size;
    }
    
    //-- Set Current Filename
    char * bsn = basename(path);
    
    if (bsn != NULL) {
      snprintf(dt->curr, 255, "%s", bsn);
      free(bsn);
    }
    
    //-- Set Current Status
    dt->n += 1;
    dt->cr = 0;
    dt->cs = sz;
    //-- Set Total Size
    int kbsz = sz / AFS_KILOBYTE;
    dt->b += sz % AFS_KILOBYTE;
    
    if (dt->b >= AFS_KILOBYTE) {
      dt->b -= AFS_KILOBYTE;
      dt->k += 1;
    }
    
    dt->k += kbsz;
    //-- Tick
    afs_tick(dt);
  }
  
  if (!islnk && S_ISDIR(fstat.st_mode)) {
    DIR * d = opendir(path);
    size_t path_len = strlen(path);
    
    if (d) {
      struct dirent * p;
      
      while ((dt->status == 1) && (p = readdir(d))) {
        //-- No "." & ".."
        if ((p->d_name[0] == '.')
            && ((p->d_name[1] == 0)
                || ((p->d_name[1] == '.')
                    && (p->d_name[2] == 0)))) {
          continue;
        }
        
        //-- Filename Buffer
        size_t len = path_len + strlen(p->d_name) + 2;
        char * buf = malloc(len);
        
        if (buf) {
          snprintf(buf, len, "%s/%s", path,
                   p->d_name);
          afs_filesize_do(dt, buf, 1);
          free(buf);
        }
      }
    }
    
    closedir(d);
  }
}

void afs_filedelete_do(AFSDTP dt, const char * path, byte ischild) {
  //-- Define
  struct stat fstat;
  struct stat lfstat;
  byte islnk = 0;
  int sz = 0;
  
  //-- Forbidden File
  if (isnodelete(path)) {
    return;
  }
  
  //-- Get Stat
  if (stat(path, &fstat)) {
    return;
  }
  
  //-- Set Current Info
  if (ischild) {
    //-- Get Link Stat
    if ((lstat(path, &lfstat) == 0) && S_ISLNK(lfstat.st_mode)) {
      sz = lfstat.st_size;
      islnk = 1;
    }
    else {
      sz = fstat.st_size;
    }
    
    //-- Set Current Filename
    char * bsn = basename(path);
    
    if (bsn != NULL) {
      snprintf(dt->curr, 255, "%s", bsn);
      free(bsn);
    }
    
    //-- Set Current Status
    dt->n += 1;
    dt->cr = 0;
    dt->cs = sz;
    //-- Set Total Size
    int kbsz = sz / AFS_KILOBYTE;
    dt->b += sz % AFS_KILOBYTE;
    
    if (dt->b >= AFS_KILOBYTE) {
      dt->b -= AFS_KILOBYTE;
      dt->k += 1;
    }
    
    dt->k += kbsz;
    //-- Tick
    afs_tick(dt);
  }
  
  if (!islnk && S_ISDIR(fstat.st_mode)) {
    DIR * d = opendir(path);
    size_t path_len = strlen(path);
    
    if (d) {
      struct dirent * p;
      
      while ((dt->status == 1) && (p = readdir(d))) {
        //-- No "." & ".."
        if ((p->d_name[0] == '.')
            && ((p->d_name[1] == 0)
                || ((p->d_name[1] == '.')
                    && (p->d_name[2] == 0)))) {
          continue;
        }
        
        //-- Filename Buffer
        size_t len = path_len + strlen(p->d_name) + 2;
        char * buf = malloc(len);
        
        if (buf) {
          snprintf(buf, len, "%s/%s", path,
                   p->d_name);
          afs_filedelete_do(dt, buf, 1);
          free(buf);
        }
      }
    }
    
    closedir(d);
    rmdir(path);
  }
  else {
    unlink(path);
  }
}

void afs_copy_do(AFSDTP dt, const char * path, const char * dest) {
  //-- Define
  struct stat fstat;
  struct stat lfstat;
  byte islnk = 0;
  int sz = 0;
  
  //-- Get Stat
  if (stat(path, &fstat)) {
    return;
  }
  
  //-- Get Link Stat
  if ((lstat(path, &lfstat) == 0) && S_ISLNK(lfstat.st_mode)) {
    sz = lfstat.st_size;
    islnk = 1;
  }
  else {
    sz = fstat.st_size;
  }
  
  //-- Set Current Info
  if (1) {
    char * bsn = basename(path);
    
    if (bsn != NULL) {
      snprintf(dt->curr, 255, "%s", bsn);
      free(bsn);
    }
    
    dt->n += 1;
    dt->cr = 0;
    dt->cs = sz;
  }
  
  byte copyit = 1;
  
  if (file_exists(dest)) {
    copyit = afs_askoverwrite(dt);
  }
  
  if (copyit) {
    //-- Process Directory
    if (!islnk && S_ISDIR(fstat.st_mode)) {
      //-- Make destination directory
      mkdir(dest, fstat.st_mode);
      
      //-- Check exist
      if (file_exists(dest)) {
        //-- Open dir to read
        DIR * d = opendir(path);
        size_t path_len = strlen(path);
        size_t dest_len = strlen(dest);
        
        if (d) {
          struct dirent * p;
          
          while ((dt->status == 1)
                 && (p = readdir(d))) {
            //-- No "." & ".."
            if ((p->d_name[0] == '.')
                && ((p->d_name[1] == 0)
                    ||
                    ((p->d_name[1] == '.')
                     && (p->d_name[2] ==
                         0)))) {
              continue;
            }
            
            //-- Filename Buffer
            size_t len =
              path_len +
              strlen(p->d_name) + 2;
            size_t led =
              dest_len +
              strlen(p->d_name) + 2;
            char * buf = malloc(len);
            char * bud = malloc(led);
            
            //-- Fetch
            if (buf) {
              if (bud) {
                snprintf(buf,
                         len,
                         "%s/%s",
                         path,
                         p->
                         d_name);
                snprintf(bud,
                         led,
                         "%s/%s",
                         dest,
                         p->
                         d_name);
                afs_copy_do(dt,
                            buf,
                            bud);
                free(bud);
              }
              
              free(buf);
            }
          }
        }
        
        //-- Clean up
        closedir(d);
        
        if (dt->flag == 1) {
          rmdir(path);
        }
      }
    }
    else if (islnk) {
      //-- Create symlink
      char buf[4096];
      int len = readlink(path, buf, sizeof(buf));
      
      if (len >= 0) {
        buf[len] = 0;
        unlink(dest);
        symlink(buf, dest);
        
        if (dt->flag == 1) {
          unlink(path);
        }
      }
    }
    else {
      byte do_full_copy = 1;
      
      if (dt->flag == 1) {
        if (file_exists(dest)) {
          unlink(dest);
        }
        
        if (rename(path, dest) != -1) {
          do_full_copy = 0;
        }
      }
      
      if (do_full_copy) {
        //-- Preparing File Copy
        int sfd = -1, dfd = -1, readsz = AFS_KILOBYTE * 32;	// * ((dt->intTick >> 3) << 6);
        byte success = 0;
        ssize_t readed;
        off_t offset = 0;
        
        //-- Open Source & Destination
        if ((sfd = open(path, O_RDONLY)) == -1) {
          goto done;
        }
        
        if ((dfd =
               open(dest, O_CREAT | O_WRONLY | O_TRUNC,
                    fstat.st_mode)) == -1) {
          goto done;
        }
        
        //-- Copy Contents
        while ((readed =
                  sendfile(dfd, sfd, &offset,
                           readsz)) > 0) {
          dt->cr += readed;
          int kbsz = readed / AFS_KILOBYTE;
          dt->b += readed % AFS_KILOBYTE;
          
          if (dt->b >= AFS_KILOBYTE) {
            dt->b -= AFS_KILOBYTE;
            dt->k += 1;
          }
          
          dt->k += kbsz;
          
          if (dt->status != 1) {
            readed = -1;
            break;
          }
          
          afs_tick(dt);
        }
        
        if (readed == -1) {
          if (errno == 28) {
            afs_askdiskfull(dt);
          }
          
          goto done;
        }
        
        success = 1;
        
        if (dt->cr <= sz) {
          sz -= dt->cr;
        }
        else {
          sz = 0;
        }
        
        //-- Cleanup
      done:
      
        if (sfd != -1) {
          close(sfd);
        }
        
        if (dfd != -1) {
          close(dfd);
        }
        
        if (success) {
          if (dt->flag == 1) {
            unlink(path);
          }
        }
        else {
          unlink(dest);
        }
      }
    }
  }
  else if (!islnk && S_ISDIR(fstat.st_mode)) {
    AFSDT dtsz;
    memset(&dtsz, 0, sizeof(AFSDT));
    dtsz.status = 1;
    dtsz.flag = 0;
    dtsz.path = path;
    dtsz.intTick = 100;
    afs_filesize_do(&dtsz, path, 0);
    sz += dtsz.b;
    dt->k += dtsz.k;
    dt->n += dtsz.n;
  }
  
  //-- Save Information
  dt->cr = dt->cs;
  int kbsz = sz / AFS_KILOBYTE;
  dt->b += sz % AFS_KILOBYTE;
  
  if (dt->b >= AFS_KILOBYTE) {
    dt->b -= AFS_KILOBYTE;
    dt->k += 1;
  }
  
  dt->k += kbsz;
  afs_tick(dt);
}

static void * afs_filesize_th(void * cookie) {
  AFSDTP dt = (AFSDTP) cookie;
  afs_filesize_do(dt, dt->path, dt->flag);
  dt->status = 2;
  
  if (dt->msgFinish) {
    while (!atouch_send_message(dt->msgFinish)) {
      usleep(dt->intTick * 1000);
    }
  }
  
  return NULL;
}

static void * afs_filedelete_th(void * cookie) {
  AFSDTP dt = (AFSDTP) cookie;
  afs_filedelete_do(dt, dt->path, dt->flag);
  dt->status = 2;
  
  if (dt->msgFinish) {
    while (!atouch_send_message(dt->msgFinish)) {
      usleep(dt->intTick * 1000);
    }
  }
  
  return NULL;
}

static void * afs_copy_th(void * cookie) {
  AFSDTP dt = (AFSDTP) cookie;
  afs_copy_do(dt, dt->path, dt->dest);
  dt->status = 2;
  
  if (dt->msgFinish) {
    while (!atouch_send_message(dt->msgFinish)) {
      usleep(dt->intTick * 1000);
    }
  }
  
  pthread_cond_destroy(&dt->ovrCond);
  pthread_mutex_destroy(&dt->ovrMutex);
  return NULL;
}

void afs_filesize(AFSDTP dt) {
  snprintf(dt->curr, 256, "");
  pthread_t th;
  pthread_create(&th, NULL, afs_filesize_th, (void *)dt);
  pthread_detach(th);
}

void afs_filedelete(AFSDTP dt) {
  snprintf(dt->curr, 256, "");
  pthread_t th;
  pthread_create(&th, NULL, afs_filedelete_th, (void *)dt);
  pthread_detach(th);
}

void afs_copy(AFSDTP dt) {
  snprintf(dt->curr, 256, "");
  pthread_mutex_init(&dt->ovrMutex, NULL);
  pthread_cond_init(&dt->ovrCond, NULL);
  pthread_t th;
  pthread_create(&th, NULL, afs_copy_th, (void *)dt);
  pthread_detach(th);
}
