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
 * AROMA FILE MANAGER UI
 *
 */

//*
//* MAIN HEADER
//*
#include <sys/times.h>
// #include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <errno.h>
#include "../aroma.h"

//*
//* GLONAL VARIABLES
//*
static  byte    aui_isbgredraw    = 0;  //-- Is Background Need Redrawed
static  int     aui_minY          = 0;  //-- Most Top Allowable UI Draw Position (Y)
static  CANVAS  aui_bg;                 //-- Saved CANVAS for background
static  CANVAS  aui_win_bg;             //-- Current drawed CANVAS for windows background
static  byte    aui_is_show_tips  = 1;
//*
//* UI MAIN STRUCT
//*
typedef struct {
  AWINDOWP  hWin;
  ACONTROLP hFile;
  
  ACONTROLP b1;
  ACONTROLP b2;
  ACONTROLP b3;
  ACONTROLP b4;
  
  char   *  path;
  char   *  selfile;
  byte      reshow;
  byte   *  state;
  byte      isRoot;
  byte      hFileType;
  
  int       pad;
  int       btnY;
  int       btnW;
  int       btnH;
  
  int       boxH;
  int       boxY;
  
  int       navY;
  int       navW;
  
  int       pthW;
  int       btnFH;
  
} AUI_VARS, *AUI_VARSP;

//*
//* UI LIBRARIES
//*
#include "libs/aroma_fmlib.c"
#include "libs/aroma_fslib.c"
#include "libs/aroma_config.c"
#include "libs/aroma_langs.c"
#include "libs/aroma_clipboard.c"
#include "libs/aroma_icons.c"
#include "libs/aroma_themes.c"
#include "libs/aroma_draws.c"
#include "libs/aroma_directory.c"

#include "libs/aroma_favorite.c"
#include "libs/aroma_uisettings.c"
#include "libs/aroma_uicopy.c"
#include "libs/aroma_uicut.c"
#include "libs/aroma_uidel.c"
#include "libs/aroma_uiabout.c"
#include "libs/aroma_uiperm.c"
#include "libs/aroma_uiterminal.c"

#include "libs/aroma_uimain.c"
#include "libs/aroma_handler.c"

//*
//* AROMA START UI
//*
byte aui_start() {
  //-- LOAD CONFIG
  aui_cfg_init();
  aui_cfg_loadcfg();
  //-- CLEANUP THEME:
  int i = 0;
  
  for (i = 0; i < AROMA_THEME_CNT; i++) {
    acfg()->theme[i] = NULL;
    acfg()->theme_9p[i] = 0;
  }
  
  ag_canvas(&aui_bg, agw(), agh());     //-- Initializing background canvas
  ag_canvas(&aui_win_bg, agw(), agh()); //-- Initializing window background canvas
  acfg_init();                          //-- Initializing configuration
  aui_load_icons();                     //-- Initializing Icons
  aui_initlang();                       //-- Initializing default language
  aui_cfg_reloadfonts();                //-- Load Fonts
  aui_langreload();                     //-- Load Custom Language
  aui_themereload();                    //-- Load Themes
  aui_isbgredraw = 1;                   //-- Set draw status
  //-- Show calibration tools
  /*
  if (!aui_cfg_loadcalib()){
   if (aw_calibtools(NULL))
     aui_cfg_save();
  }
  */
  //-- INIT OPENED PATH
  char * path       = NULL;
  byte tool_state   = 0;
  aui_setpath(&path, "", "", 1);
  //-- SELECTED FILE
  char selfile[256];
  snprintf(selfile, 256, "");
  
  if (path) {
    if (auic()->automount) {
      ag_setbusy_withtext(alang_get("mounting"));
      LOGS("Automount All Partitions\n");
      alib_exec("/sbin/mount", "-a");
    }
    
    //-- MAIN LOOP
    byte back_ani = 0;
    
    do {
      LOGS("Change Folder %s\n", path);
      byte ret_show = aui_show(&path, &tool_state, selfile, back_ani);
      back_ani = 0;
      
      if (!ret_show) {
        break;
      }
      else if (ret_show == 2) {
        aui_show_setting();
        back_ani = 1;
      }
      else if (ret_show == 3) {
        auido_copy(path, &tool_state);
      }
      else if (ret_show == 4) {
        auido_cut(path, &tool_state);
      }
      else if (ret_show == 5) {
        auido_del(path, &tool_state);
      }
      else if (ret_show == 6) {
        /* Terminal */
        printf("Opening Terminal At : %s\n", path);
        aui_show_terminal(path);
        back_ani = 1;
      }
    }
    while (1);
    
    //-- Free path memory
    free(path);
  }
  
  auic_release();           //-- Cleanup Clipboard
  aui_unload_icons();       //-- Release icons
  ag_ccanvas(&aui_win_bg);  //-- Release window background canvas
  ag_ccanvas(&aui_bg);      //-- Release background canvas
  alang_release();          //-- Release Custom language
  aui_releaselang();        //-- Release Default language
  atheme_releaseall();      //-- Release themes
  aui_cfg_release();        //-- Release Config Array
  return 1;
}