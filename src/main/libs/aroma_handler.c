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
 * AROMA File Manager WINDOW MESSAGE HANDLER
 *
 */

//*
//* CHECKED FILE CHANGE
//*
void auia_chkcount(AUI_VARSP v) {
  if (*v->state != 3) {
    int ckn = afbox_checkcount(v->hFile);
    int itn = afbox_itemcount(v->hFile);
    
    if (ckn == 0) {
      aui_changetitle(v, AROMA_NAME);
    }
    else {
      char titlex[128];
      
      if (ckn == 1)
        snprintf(titlex, 128,
                 alang_get("selected.file"), ckn);
      else
        snprintf(titlex, 128,
                 alang_get("selected.files"), ckn);
                 
      aui_changetitle(v, titlex);
    }
    
    if (ckn == 0) {
      *v->state = aui_tbstate(v, 0, *v->state);
    }
    else if (ckn == itn) {
      *v->state = aui_tbstate(v, 2, *v->state);
    }
    else {
      *v->state = aui_tbstate(v, 1, *v->state);
    }
  }
}

//*
//* PARENT DIR
//*
byte auia_parent(AUI_VARSP v) {
  if (strlen(v->path) > 2) {
    char * newdir = dirname(v->path);
    char * basen = basename(v->path);
    
    if (basen != NULL) {
      snprintf(v->selfile, 256, "%s", basen);
      free(basen);
    }
    
    aui_setpath(&v->path, newdir, "", 0);
    free(newdir);
    v->reshow = 1;
    return 0;
  }
  
  return 1;
}

//*
//* ONCLICK
//*
byte auia_click(AUI_VARSP v) {
  char * fl = afbox_getselectedfile(v->hFile);
  byte dtype = afbox_dtype(v->hFile);
  
  if (((dtype == 4) || (dtype == 24)) && (fl != NULL)) {
    aui_setpath(&v->path, v->path, fl, 1);
    v->reshow = 1;
    return 0;
  }
  else if (fl != NULL) {
    printf("File  [%i]: %s\n", dtype, fl);
  }
  
  return 1;
}

//*
//* CHANGE PERM
//*
byte auia_hold_perm(AUI_VARSP v, char * fname) {
  byte ret = 1;
  char * path_filename = NULL;
  aui_setpath(&path_filename, v->path, fname, 0);
  ret = auido_setperm(v->hWin, path_filename, fname);
  free(path_filename);
  
  if (!ret) {
    v->reshow = 1;
    snprintf(v->selfile, 256, "%s", fname);
  }
  
  return ret;
}

//*
//* MAKE NEW FOLDER
//*
byte auia_hold_rename(AUI_VARSP v, char * oldname) {
  byte redoing = 0;
  char new_folder_name[256];
  snprintf(new_folder_name, 256, "%s", oldname);
  byte dispatch_ret = 1;
  
  do {
    redoing = 0;
    dispatch_ret = 1;
    char * ret =
      aw_ime(v->hWin, new_folder_name, alang_get("rename"));
      
    if (ret != NULL) {
      if (strlen(ret) == 0) {
        redoing = aw_confirm(v->hWin,
                             alang_get("rename"),
                             alang_get
                             ("rename.insertname"),
                             aui_icons(37),
                             alang_get("tryagain"),
                             alang_get("cancel"));
      }
      else if (strcmp(ret, oldname) == 0) {
        redoing = 0;
      }
      else {
        char * path_newfolder = NULL;
        aui_setpath(&path_newfolder, v->path, ret, 0);
        
        if (path_newfolder != NULL) {
          if (file_exists(path_newfolder)) {
            redoing = aw_confirm(v->hWin,
                                 alang_get
                                 ("rename"),
                                 alang_get
                                 ("rename.exist"),
                                 aui_icons
                                 (39),
                                 alang_get
                                 ("tryagain"),
                                 alang_get
                                 ("cancel"));
          }
          else {
            char * path_oldfolder = NULL;
            aui_setpath(&path_oldfolder,
                        v->path, oldname,
                        0);
                        
            if (path_oldfolder) {
              int retval =
                rename
                (path_oldfolder,
                 path_newfolder);
                 
              if (retval != -1) {
                dispatch_ret =
                  0;
                v->reshow = 1;
                snprintf(v->
                         selfile,
                         256,
                         "%s",
                         ret);
              }
              else {
                redoing =
                  aw_confirm
                  (v->hWin,
                   alang_get
                   ("rename"),
                   alang_get
                   ("rename.error"),
                   aui_icons
                   (37),
                   alang_get
                   ("tryagain"),
                   alang_get
                   ("cancel"));
              }
              
              free(path_oldfolder);
            }
          }
          
          free(path_newfolder);
        }
      }
      
      snprintf(new_folder_name, 256, "%s", ret);
      free(ret);
    }
  }
  while (redoing);
  
  return dispatch_ret;
}

byte auia_do_delete(AUI_VARSP v) {
  AWMENUITEM mi[2];
  aw_menuset(mi, 0, "yes", 33);
  aw_menuset(mi, 1, "no", 0);
  char info[256];
  char numfl[64];
  snprintf(numfl, 64, "%i", auic_s);
  snprintf(info, 256, alang_get("delete.message"), numfl);
  byte ovr = aw_multiconfirm_ex(v->hWin, alang_get("delete.title"), info,
                                aui_icons(37),
                                mi, 2, 1);
  ag_setbusy();
  
  if (ovr == 0) {
    aui_changetitle(v, AROMA_NAME);
    v->reshow = 5;
    return 0;
  }
  
  auic_release();
  return 1;
}

byte auia_hold(AUI_VARSP v) {
  char * fl = afbox_getselectedfile(v->hFile);
  byte dtype = afbox_dtype(v->hFile);
  
  if (fl != NULL) {
    AWMENUITEM mi[6];
    int cp = 0;
    byte onfav = 0;
    byte isonfav = 0;
    
    if ((dtype == 4) || (dtype == 24)) {
      isonfav = auifav_isfav(fl);
      
      if (isonfav) {
        aw_menuset(mi, cp++, "tools.favorite.del", 8);
      }
      else {
        aw_menuset(mi, cp++, "tools.favorite.add", 7);
      }
      
      onfav = 1;
    }
    
    aw_menuset(mi, cp++, "tools.rename", 15);
    aw_menuset(mi, cp++, "tools.copy", 2);
    aw_menuset(mi, cp++, "tools.cut", 3);
    aw_menuset(mi, cp++, "tools.delete", 4);
    aw_menuset(mi, cp++, "tools.chmod", 1);
    byte ret = aw_menu(v->hWin, fl, mi, cp);
    
    if ((!onfav) && (ret > 0)) {
      ret++;
    }
    
    if (ret == 2) {
      //-- Rename
      return auia_hold_rename(v, fl);
    }
    else if (ret == 6) {
      //-- Rename
      return auia_hold_perm(v, fl);
    }
    else if ((ret == 3) || (ret == 4)) {
      //-- Copy / cut
      ag_setbusy();
      auic_init(ret - 2, 1, v->path);
      auic_add(fl);
      *v->state = aui_tbstate(v, 3, *v->state);
      afbox_changeboxtype(v->hFile, 1);
      aui_changetitle(v, alang_get("paste.location"));
      afbox_setcheckall(v->hFile, 0);
    }
    else if (ret == 5) {
      //-- Copy / cut
      ag_setbusy();
      auic_init(3, 1, v->path);
      auic_add(fl);
      return auia_do_delete(v);
    }
    else if (ret == 1) {
      //-- Add Remove Favorite
      char * full_fl = NULL;
      aui_setpath(&full_fl, v->path, fl, 1);
      
      if (full_fl != NULL) {
        if (isonfav) {
          auifav_del(full_fl);
        }
        else {
          auifav_add(full_fl);
        }
        
        LOGS("Add To Favorite: %s\n", full_fl);
        free(full_fl);
      }
    }
  }
  
  return 1;
}

//*
//* COPY CUT
//*
void auia_setclipboard(AUI_VARSP v, byte type) {
  ag_setbusy();
  int ckn = afbox_checkcount(v->hFile);
  int itn = afbox_itemcount(v->hFile);
  auic_init(type, ckn, v->path);
  int i;
  
  for (i = 0; i < itn; i++) {
    char * fn = afbox_getcfile(v->hFile, i);
    
    if (fn != NULL) {
      auic_add(fn);
    }
  }
  
  if (type != 3) {
    *v->state = aui_tbstate(v, 3, *v->state);
    afbox_changeboxtype(v->hFile, 1);
    aui_changetitle(v, alang_get("paste.location"));
    afbox_setcheckall(v->hFile, 0);
  }
}

//*
//* CANCEL CLIPBOARD
//*
void auia_unsetclipboard(AUI_VARSP v) {
  ag_setbusy();
  auic_release();
  *v->state = aui_tbstate(v, 0, *v->state);
  afbox_changeboxtype(v->hFile, 0);
  aui_changetitle(v, AROMA_NAME);
}

//*
//* MAKE NEW FOLDER
//*
byte auia_newfolder(AUI_VARSP v) {
  byte redoing = 0;
  char new_folder_name[256];
  snprintf(new_folder_name, 256, "New Folder");
  byte dispatch_ret = 1;
  
  do {
    redoing = 0;
    dispatch_ret = 1;
    char * ret =
      aw_ime(v->hWin, new_folder_name, alang_get("newfolder"));
      
    if (ret != NULL) {
      if (strlen(ret) == 0) {
        redoing = aw_confirm(v->hWin,
                             alang_get("newfolder"),
                             alang_get
                             ("newfolder.insertname"),
                             aui_icons(37),
                             alang_get("tryagain"),
                             alang_get("cancel"));
      }
      else {
        char * path_newfolder = NULL;
        aui_setpath(&path_newfolder, v->path, ret, 0);
        
        if (path_newfolder != NULL) {
          if (file_exists(path_newfolder)) {
            redoing = aw_confirm(v->hWin,
                                 alang_get
                                 ("newfolder"),
                                 alang_get
                                 ("newfolder.exist"),
                                 aui_icons
                                 (39),
                                 alang_get
                                 ("tryagain"),
                                 alang_get
                                 ("cancel"));
          }
          else {
            int retval =
              mkdir(path_newfolder, 0777);
              
            if (retval == 0) {
              dispatch_ret = 0;
              v->reshow = 1;
              snprintf(v->selfile,
                       256, "%s",
                       ret);
            }
            else {
              redoing =
                aw_confirm(v->hWin,
                           alang_get
                           ("newfolder"),
                           alang_get
                           ("newfolder.error"),
                           aui_icons
                           (37),
                           alang_get
                           ("tryagain"),
                           alang_get
                           ("cancel"));
            }
          }
          
          free(path_newfolder);
        }
      }
      
      snprintf(new_folder_name, 256, "%s", ret);
      free(ret);
    }
  }
  while (redoing);
  
  return dispatch_ret;
}

//*
//* SHOW MENU
//*
byte auia_menu(AUI_VARSP v) {
  byte common_cmd = 0;
  byte cstate = *v->state;
  
  if ((cstate == 1) || (cstate == 2)) {
    AWMENUITEM mi[5];
    int cp = 0;
    
    if (cstate == 1) {
      aw_menuset(mi, cp++, "tools.unselect", 19);
    }
    
    aw_menuset(mi, cp++, "tools.delete", 4);
    aw_menuset(mi, cp++, "settings", 17);
    aw_menuset(mi, cp++, "about", 9);
    aw_menuset(mi, cp++, "exit", 0);
    byte ret = aw_menu(v->hWin, NULL, mi, cp);
    
    if ((cstate == 2) && (ret > 0)) {
      ret++;
    }
    
    if (ret == 5) {
      common_cmd = 1;
    }
    else if (ret == 3) {
      common_cmd = 2;
    }
    else if (ret == 4) {
      common_cmd = 3;
    }
    else if (ret == 2) {
      //-- Delete
      auia_setclipboard(v, 3);
      return auia_do_delete(v);
    }
    else if (ret == 1) {
      afbox_setcheckall(v->hFile, 0);
    }
  }
  else {
    int cp = 0;
    AWMENUITEM mi[7];
    
    if (cstate == 3) {
      aw_menuset(mi, cp++, "tools.newfolder", 11);
    }
    
    byte isonfav = auifav_isfav(v->path);
    
    if (isonfav) {
      aw_menuset(mi, cp++, "tools.favorite.del", 8);
    }
    else {
      aw_menuset(mi, cp++, "tools.favorite.add", 7);
    }
    
    aw_menuset(mi, cp++,
               (auic()->showhidden ? "tools.hiddenfile" :
                "tools.hiddenfile.show"), 18);
    aw_menuset(mi, cp++, "tools.terminal", 40);
    aw_menuset(mi, cp++, "settings", 17);
    aw_menuset(mi, cp++, "about", 9);
    aw_menuset(mi, cp++, "exit", 0);
    byte ret = aw_menu(v->hWin, NULL, mi, cp);
    
    if ((cstate != 3) && (ret > 0)) {
      ret++;
    }
    
    if (ret == 7) { /* EXIT */
      common_cmd = 1;
    }
    else if (ret == 6) { /* ABOUT */
      common_cmd = 3;
    }
    else if (ret == 4) { /* TERMINAL */
      common_cmd = 4;
    }
    else if (ret == 5) { /* SETTING */
      common_cmd = 2;
    }
    else if (ret == 3) {
      auic()->showhidden = (auic()->showhidden ? 0 : 1);
      aui_cfg_save();
      v->reshow = 1;
      return 0;
    }
    else if (ret == 2) {
      //-- Add Remove Favorite
      if (isonfav) {
        auifav_del(v->path);
      }
      else {
        auifav_add(v->path);
      }
    }
    else if (ret == 1) {
      //-- Show New Folder
      return auia_newfolder(v);
    }
  }
  
  //-- Common command
  if (common_cmd == 1) {	//-- Exit
    AWMENUITEM mi[2];
    aw_menuset(mi, 0, "yes", 33);
    aw_menuset(mi, 1, "no", 0);
    byte ovr = aw_multiconfirm_ex(v->hWin, alang_get("exit"),
                                  alang_get("exit.message"),
                                  aui_icons(39), mi, 2,
                                  1);
                                  
    if (ovr == 0) {
      return 0;
    }
  }
  else if (common_cmd == 2) {	//-- Settings
    v->reshow = 2;
    return 0;
  }
  else if (common_cmd == 3) {	//-- About
    auido_about_dialog(v->hWin);
  }
  else if (common_cmd == 4) {	//-- Terminal
    v->reshow = 6;
    return 0;
  }
  
  return 1;
}

//*
//* WINDOW MESSAGE HANDLER
//*
byte aui_dispatch(AUI_VARSP v) {
  byte ondispatch = 1;
  dword msg = aw_dispatch(v->hWin);
  
  switch (aw_gm(msg)) {
    case 20: {
        ondispatch = auia_newfolder(v);
      }
      break;
      
    case 21: {
        char * out = auifav(v->hWin, v->path);
        
        if (out != NULL) {
          aui_setpath(&v->path, out, "", 0);
          ondispatch = 0;
          v->reshow = 1;
          free(out);
        }
      }
      break;
      
      //-- CHECK COUNT
    case 6: {
        auia_chkcount(v);
      }
      break;
      
      //-- Check All
    case 22: {
        afbox_setcheckall(v->hFile, 1);
      }
      break;
      
      //-- Un Check All
    case 23: {
        afbox_setcheckall(v->hFile, 0);
      }
      break;
      
      //-- Copy
    case 30: {
        auia_setclipboard(v, 1);
      }
      break;
      
      //-- Cut
    case 31: {
        auia_setclipboard(v, 2);
      }
      break;
      
      //-- Cancel Clipboard
    case 41: {
        auia_unsetclipboard(v);
      }
      break;
      
      //-- Paste Clipboard
    case 40: {
        if (auic_t == 1) {
          //-- COPY
          aui_changetitle(v, AROMA_NAME);
          v->reshow = 3;
          ondispatch = 0;
        }
        else if (auic_t == 2) {
          //-- CUT
          aui_changetitle(v, AROMA_NAME);
          v->reshow = 4;
          ondispatch = 0;
        }
      }
      break;
      
      //-- MAIN MENU
    case 3: {
        ondispatch = auia_menu(v);
      }
      break;
      
      //-- EXIT
    case 4: {
        ondispatch = 0;
      }
      break;
      
      //-- PARENT
    case 11: {
        ondispatch = auia_parent(v);
      }
      break;
      
      //-- BACK BUTTON
    case 12: {
        if ((*v->state == 0) || (*v->state == 3)) {
          //-- Non Check
          ondispatch = auia_parent(v);
        }
        else {
          afbox_setcheckall(v->hFile, 0);
        }
      }
      break;
      
      //-- REFRESH
    case 10: {
        ondispatch = 0;
        v->reshow = 1;
      }
      break;
      
      //-- ITEM CLICK
    case 7: {
        ondispatch = auia_click(v);
      }
      break;
      
      //-- ITEM TAP & HOLD
    case 8: {
        ondispatch = auia_hold(v);
      }
      break;
  }
  
  //-- Still On Dispatch?
  return ondispatch;
}
