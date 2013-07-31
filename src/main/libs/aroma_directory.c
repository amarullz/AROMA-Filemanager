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
 * AROMA File Manager Directory Listing
 *
 */
//*
//* Redraw Window Background
//*
/************************************[ START AND PARSE SCRIPT ]************************************/
char * aui_dir_active_path = NULL;
int aui_dtcheck(char * name, byte t, byte addsz) {
  if (t == 10) {
    char * buf = NULL;
    aui_setpath(&buf, aui_dir_active_path, name, 0);
    
    if (buf) {
      struct stat statbuf;
      
      if (!stat(buf, &statbuf)) {
        if (S_ISDIR(statbuf.st_mode)) {
          free(buf);
          return addsz + 4;
        }
        else {
          free(buf);
          return addsz + 8;
        }
      }
      
      free(buf);
    }
    
    return 10;
  }
  
  return t;
}

int aui_fsort(const struct dirent ** pa, const struct dirent ** pb) {
  struct dirent * a = (struct dirent *)*pa;
  struct dirent * b = (struct dirent *)*pb;
  byte adt = aui_dtcheck(a->d_name, a->d_type, 0);
  byte bdt = aui_dtcheck(b->d_name, b->d_type, 0);
  
  if (adt < bdt) {
    return 1;
  }
  else if (adt > bdt) {
    return -1;
  }
  
  return strcasecmp(b->d_name, a->d_name);
}

int aui_getdircount(char * path) {
  struct dirent ** files;
  int n = scandir(path, &files, 0, alphasort);
  int c = 0;
  
  if (n > 0) {
    while (n--) {
      char * dname = files[n]->d_name;
      
      if (!strcmp(dname, ".") || !strcmp(dname, "..")) {
        continue;
      }
      
      byte dtype = files[n]->d_type;
      
      if ((dtype == 4) || (dtype == 8) || (dtype == 10)) {
        c++;
      }
      
      free(files[n]);
    }
    
    free(files);
  }
  
  return c;
}

void aui_bytesize(char * buf, int val) {
  if (val > 524288)
    snprintf(buf, 64, "<#selectbg_g>%1.1f</#> MB",
             ((float)val) / 1048576);
  else if (val > 1024) {
    snprintf(buf, 64, "<#selectbg_g>%i</#> KB", round(val / 1024));
  }
  else {
    snprintf(buf, 64, "<#selectbg_g>%i</#> Bytes", val);
  }
}

void aui_filesize(char * buf, char * path) {
  struct stat fst;
  
  if (!stat(path, &fst)) {
    aui_bytesize(buf, fst.st_size);
  }
  else {
    snprintf(buf, 64, "<#selectbg_g>0</#> Bytes");
  }
}

void aui_fileperm(char * buf, char * path) {
  struct stat fst;
  
  if (!stat(path, &fst)) {
    buf[8] = (fst.st_mode & S_IXOTH) ? 'x' : '-';
    buf[7] = (fst.st_mode & S_IWOTH) ? 'w' : '-';
    buf[6] = (fst.st_mode & S_IROTH) ? 'r' : '-';
    buf[5] = (fst.st_mode & S_IXGRP) ? 'x' : '-';
    buf[4] = (fst.st_mode & S_IWGRP) ? 'w' : '-';
    buf[3] = (fst.st_mode & S_IRGRP) ? 'r' : '-';
    buf[2] = (fst.st_mode & S_IXUSR) ? 'x' : '-';
    buf[1] = (fst.st_mode & S_IWUSR) ? 'w' : '-';
    buf[0] = (fst.st_mode & S_IRUSR) ? 'r' : '-';
    //-- SUID, GUID, STICKY
    buf[2] = (fst.st_mode & S_ISUID) ? ((buf[2] == 'x') ? 's' : 'S') : buf[2];
    buf[5] = (fst.st_mode & S_ISGID) ? ((buf[2] == 'x') ? 's' : 'S') : buf[5];
    buf[8] = (fst.st_mode & S_ISVTX) ? ((buf[2] == 'x') ? 't' : 'T') : buf[8];
    buf[9] = 0;
  }
  else {
    snprintf(buf, 10, "---------");
  }
}

byte aui_ishidden_file(const char * fn) {
  if (strcmp(fn, "/dev") == 0) {
    return 1;
  }
  else if (strcmp(fn, "/proc") == 0) {
    return 1;
  }
  else if (strcmp(fn, "/sys") == 0) {
    return 1;
  }
  else if (strcmp(fn, "/acct") == 0) {
    return 1;
  }
  else if (strcmp(fn, "/vendor") == 0) {
    return 1;
  }
  else if (strcmp(fn, "/tmp/update-binary") == 0) {
    return 1;
  }
  else if (strcmp(fn, AROMA_TMP) == 0) {
    return 1;
  }
  
  return 0;
}

int aui_fetch(char * path, ACONTROLP FB, char * selfile) {
  struct dirent ** files;
  aui_dir_active_path = path;
  int n = scandir(path, &files, 0, *aui_fsort);
  
  if (n > 0) {
    while (n--) {
      char * dname = files[n]->d_name;
      
      if (!strcmp(dname, ".") || !strcmp(dname, "..")) {
        free(files[n]);
        continue;
      }
      
      if ((dname[0] == '.') && (auic()->showhidden == 0)) {
        free(files[n]);
        continue;
      }
      
      //-- GET TYPE
      byte dtype = aui_dtcheck(dname, files[n]->d_type, 20);
      char * full_path = NULL;
      aui_setpath(&full_path, aui_dir_active_path, dname, 0);
      
      if (full_path) {
        if (auic()->showhidden == 0) {
          if (aui_ishidden_file(full_path)) {
            free(full_path);
            free(files[n]);
            continue;
          }
        }
        
        byte selectedDefault =
          strcmp(selfile, dname) ? 0 : 1;
          
        //-- DIR
        if (dtype == 4) {
          int cnt = aui_getdircount(full_path);
          char desc[256];
          char perm[10];
          aui_fileperm(perm, full_path);
          char formats[256];
          char formats2[256];
          snprintf(formats, 256,
                   "<#selectbg_g>%i</#>", cnt);
          snprintf(formats2, 256,
                   alang_get((cnt >
                              1) ?
                             "dir.filecounts" :
                             "dir.filecount"),
                   formats);
          snprintf(desc, 256, "<@right>%s</@>",
                   formats2);
          afbox_add(FB, dname, desc, 0,
                    &UI_ICONS[22], dtype, perm, 0,
                    selectedDefault);
        }
        //-- FILE
        else if (dtype == 8) {
          char desc[256];
          char dsz[256];
          char perm[10];
          aui_fileperm(perm, full_path);
          aui_filesize(dsz, full_path);
          snprintf(desc, 256, "<@right>%s</@>",
                   dsz);
          afbox_add(FB, dname, desc, 0,
                    &UI_ICONS[21], dtype, perm, 0,
                    selectedDefault);
        }
        //-- LINK FILE / DIR
        else if ((dtype == 28) || (dtype == 24)) {
          char * desc = NULL;
          char buf[4096];
          int len =
            readlink(full_path, buf,
                     sizeof(buf));
                     
          if (len >= 0) {
            buf[len] = 0;
            desc =
              aui_strip(buf,
                        agw() -
                        (agdp() * 46), 0);
          }
          
          afbox_add(FB, dname,
                    ((desc ==
                      NULL) ?
                     alang_get("symlink.error") :
                     desc), 0,
                    &UI_ICONS[(dtype ==
                               24) ? 25 : 24],
                    dtype, "", 0,
                    selectedDefault);
                    
          if (desc != NULL) {
            free(desc);
          }
        }
        //-- ERROR LINK
        else if (dtype == 10) {
          afbox_add(FB, dname,
                    alang_get("symlink.error"), 0,
                    &UI_ICONS[23], dtype, "", 0,
                    selectedDefault);
        }
        
        free(full_path);
      }
      
      free(files[n]);
    }
    
    free(files);
  }
  
  return 1;
}
