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
 * AROMA File Manager Settings
 *
 */

void auis_readinfo(char * out, char * path, char * dir) {
  char z[256];
  snprintf(z, 256, "%s%s/info.txt", path, dir);
  char * buf = aui_readfromzip(z);
  
  if (buf != NULL) {
    snprintf(out, 256, "%s", buf);
    free(buf);
  }
  else {
    snprintf(out, 256, "");
  }
}

//*
//* SHOW MAIN DIRECTORY LIST UI
//*
void aui_show_setting() {
  printf("SHOW SETTINGS\n");
  //-- REDRAW BG
  aui_setbg(NULL);
  //-- Main Variable
  AUI_VARS v;
  //-- Return Value
  v.reshow = 0;
  //-- Init Drawing Data
  v.selfile = "";
  v.path = "";
  v.state = 0;
  v.pad = agdp() * 2;
  v.btnH = agdp() * 20;
  v.boxH = agh() - (aui_minY + (v.btnH) + (v.pad * 2));
  v.boxY = aui_minY;
  v.btnY = v.boxY + v.boxH + v.pad;
  v.navW = agw() - (v.pad * 2);
  v.pthW = v.navW;
  v.navY = aui_minY + v.pad;
  v.btnW = floor(v.navW / 3);
  int btnHlv = v.navW - (v.pad / 2);
  v.hFileType = 0;
  v.btnFH = aui_cfg_btnFH();
  //-- Draw Navigation Bar
  // aui_drawtopbar(&aui_win_bg, 0, aui_minY, agw(), v.btnH + (v.pad * 2));
  aui_drawnav(&aui_win_bg, 0, v.btnY - (v.pad + v.btnFH), agw(),
              v.btnH + (v.pad * 2) + v.btnFH);
  //-- Create Main Window
  v.hWin = aw(&aui_win_bg);
  //****************************[ INIT WINDOW CONTROLS ]****************************/
  // imgbtn(v.hWin, v.pad, v.navY, btnHlv, v.btnH, &UI_ICONS[20], alang_get("settings.mount"), 3, 22);
  /*imgbtn(v.hWin, v.pad, v.navY, btnHlv, v.btnH, &UI_ICONS[17],
         alang_get("settings.calib"), 3, 21);
  */
  v.hFile = acopt(v.hWin, 0, v.boxY, agw(), (v.boxH - v.btnFH));
  //-- Toolbar style : 1
  acopt_addgroup(v.hFile, alang_get("settings.toolbar"), "");
  acopt_add(v.hFile, alang_get("settings.toolbar.icontext"), "",
            auic()->tooltext ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.toolbar.icononly"), "",
            auic()->tooltext ? 0 : 1);
  //-- Font size : 2
  acopt_addgroup(v.hFile, alang_get("settings.fontsize"), "");
  acopt_add(v.hFile, alang_get("settings.fontsize.small"), "",
            (auic()->fontsize == 1) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.fontsize.medium"), "",
            (auic()->fontsize == 2) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.fontsize.big"), "",
            (auic()->fontsize == 3) ? 1 : 0);
  //-- Font Family : 3
  acopt_addgroup(v.hFile, alang_get("settings.fontfamily"), "");
  
  if (1) {
    AZREADDIRP rz_fonts = az_readdir(AROMA_DIR "/fonts/");
    
    if (rz_fonts != NULL) {
      int i = 0;
      
      for (i = 0; i < rz_fonts->n; i++) {
        if (rz_fonts->t[i] == 1) {
          char desc[256];
          auis_readinfo(desc, AROMA_DIR "/fonts/",
                        rz_fonts->f[i]);
          acopt_add(v.hFile, rz_fonts->f[i], desc,
                    (strcmp
                     (auic()->fontfamily,
                      rz_fonts->f[i]) ==
                     0) ? 1 : 0);
        }
      }
      
      az_readdir_free(rz_fonts);
    }
  }
  
  //-- Icon set : 4
  acopt_addgroup(v.hFile, alang_get("settings.iconset"), "");
  
  if (1) {
    AZREADDIRP rz_icons = az_readdir(AROMA_DIR "/icons/");
    
    if (rz_icons != NULL) {
      int i = 0;
      
      for (i = 0; i < rz_icons->n; i++) {
        if (rz_icons->t[i] == 1) {
          char desc[256];
          auis_readinfo(desc, AROMA_DIR "/icons/",
                        rz_icons->f[i]);
          acopt_add(v.hFile, rz_icons->f[i], desc,
                    (strcmp
                     (auic()->iconset,
                      rz_icons->f[i]) ==
                     0) ? 1 : 0);
        }
      }
      
      az_readdir_free(rz_icons);
    }
  }
  
  //-- Theme : 5
  acopt_addgroup(v.hFile, alang_get("settings.themes"), "");
  acopt_add(v.hFile, "Generic", "Unskinned AROMA File Manager",
            (strcmp(auic()->theme, "") == 0) ? 1 : 0);
            
  if (1) {
    AZREADDIRP rz_theme = az_readdir(AROMA_DIR "/themes/");
    
    if (rz_theme != NULL) {
      int i = 0;
      
      for (i = 0; i < rz_theme->n; i++) {
        if (rz_theme->t[i] == 1) {
          char desc[256];
          auis_readinfo(desc,
                        AROMA_DIR "/themes/",
                        rz_theme->f[i]);
          acopt_add(v.hFile, rz_theme->f[i], desc,
                    (strcmp
                     (auic()->theme,
                      rz_theme->f[i]) ==
                     0) ? 1 : 0);
        }
      }
      
      az_readdir_free(rz_theme);
    }
  }
  
  //-- Languages : 6
  acopt_addgroup(v.hFile, alang_get("settings.lang"), "");
  
  if (1) {
    AZREADDIRP rz_langs = az_readdir(AROMA_DIR "/langs/");
    
    if (rz_langs != NULL) {
      int i = 0;
      
      for (i = 0; i < rz_langs->n; i++) {
        if (rz_langs->t[i] == 1) {
          char desc[256];
          auis_readinfo(desc, AROMA_DIR "/langs/",
                        rz_langs->f[i]);
          acopt_add(v.hFile, rz_langs->f[i], desc,
                    (strcmp
                     (auic()->language,
                      rz_langs->f[i]) ==
                     0) ? 1 : 0);
        }
      }
      
      az_readdir_free(rz_langs);
    }
  }
  
  //-- Font size : 7
  acopt_addgroup(v.hFile, alang_get("settings.trans"), "");
  acopt_add(v.hFile, alang_get("settings.trans.0"), "",
            (acfg()->fadeframes == 0) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.trans.1"), "",
            (acfg()->fadeframes == 2) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.trans.2"), "",
            (acfg()->fadeframes == 4) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.trans.3"), "",
            (acfg()->fadeframes == 6) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.trans.4"), "",
            (acfg()->fadeframes == 8) ? 1 : 0);
  //-- Automount : 8
  acopt_addgroup(v.hFile, alang_get("settings.mount"), "");
  acopt_add(v.hFile, alang_get("settings.mount.all"), "",
            auic()->automount ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.mount.none"), "",
            auic()->automount ? 0 : 1);
            
   
  //-- colorspace : 9
  acopt_addgroup(v.hFile, alang_get("settings.colorspace"), "");
  acopt_add(v.hFile, alang_get("settings.colorspace.rgba"), "",
            (auic()->colorspace == 1) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.colorspace.abgr"), "",
            (auic()->colorspace == 2) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.colorspace.argb"), "",
            (auic()->colorspace == 3) ? 1 : 0);
  acopt_add(v.hFile, alang_get("settings.colorspace.bgra"), "",
            (auic()->colorspace == 4) ? 1 : 0);           
            
            
  //-- Font size : 8
  /*
     acopt_addgroup(v.hFile,"Automount Partitions","");
     acopt_add(v.hFile,"All partitions","",0);
     acopt_add(v.hFile,"None","",1);
   */
  //-- TOOLS
  v.b1 = imgbtn(v.hWin, v.pad, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[33], aui_tbtitle(9), 1, 12);	//-- Done
  v.b2 = imgbtn(v.hWin, v.pad + v.btnW, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[14], aui_tbtitle(10), 1, 22);	//-- Reset
  v.b3 = imgbtn(v.hWin, v.pad + v.btnW * 2, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[0], aui_tbtitle(8), 1, 20);	//-- Cancel
  //-- Show Window
  aw_show_ex(v.hWin, 2, 0, v.hFile);
  // aw_setfocus(v.hWin,v.hFile);
  aui_changetitle(&v, alang_get("settings"));
  byte ondispatch = 1;
  byte saveconfig = 1;
  byte save_to_file = 0;
  
  do {
    dword msg = aw_dispatch(v.hWin);
    
    switch (aw_gm(msg)) {
      case 12: {
          //-- DONE
          ondispatch = 0;
        }
        break;
        
      case 20: {
          //-- CANCEL
          ondispatch = 0;
          saveconfig = 0;
        }
        break;
        
      case 22: {
          //-- RESET
          if (aw_confirm(v.hWin,
                         alang_get("settings.reset"),
                         alang_get("settings.reset.msg"),
                         aui_icons(39), NULL, NULL)) {
            ondispatch = 0;
            saveconfig = 0;
            aui_cfg_init();
            aui_load_icons();
            aui_cfg_reloadfonts();
            aui_langreload();
            aui_themereload();
            save_to_file = 1;
          }
        }
        break;
        
      case 21: {
          //-- CALIB
          if (aw_calibtools(NULL)) {
            aui_cfg_save();
          }
        }
        break;
    }
  }
  while (ondispatch);
  
  if (saveconfig) {
    byte font_reloaded = 0;
    byte colorspace_refreshed = 0;
    //-- Text on toolbar
    auic()->tooltext = (acopt_getvalue(v.hFile, 1) == 1) ? 1 : 0;
    auic()->automount = (acopt_getvalue(v.hFile, 8) == 1) ? 1 : 0;
    //-- Font Size
    byte newfontsz = (byte) acopt_getvalue(v.hFile, 2);
    
    if (newfontsz != auic()->fontsize) {
      auic()->fontsize = newfontsz;
      font_reloaded = 1;
    }
    
    byte newcolorspace = (byte) acopt_getvalue(v.hFile, 9);
    
    if (newcolorspace != auic()->colorspace) {
      auic()->colorspace = newcolorspace;
      colorspace_refreshed = 1;
    }
    
    //-- Font Family
    char * fontsel = acopt_getseltitle(v.hFile, 3);
    
    if ((fontsel != NULL)
        && (strcmp(fontsel, auic()->fontfamily) != 0)) {
      snprintf(auic()->fontfamily, 256, "%s", fontsel);
      font_reloaded = 1;
    }
    
    //-- Iconset
    fontsel = acopt_getseltitle(v.hFile, 4);
    
    if ((fontsel != NULL)
        && (strcmp(fontsel, auic()->iconset) != 0)) {
      snprintf(auic()->iconset, 256, "%s", fontsel);
      aui_unload_icons();
      aui_load_icons();
    }
    
    //-- Theme
    fontsel = acopt_getseltitle(v.hFile, 5);
    
    if ((fontsel != NULL) && (strcmp(fontsel, auic()->theme) != 0)) {
      int newtheme = acopt_getvalue(v.hFile, 5);
      
      if (newtheme == 1) {
        if (strcmp(auic()->theme, "") != 0) {
          snprintf(auic()->theme, 256, "");
          aui_themereload();
        }
      }
      else {
        snprintf(auic()->theme, 256, "%s", fontsel);
        aui_themereload();
      }
    }
    
    //-- Language
    fontsel = acopt_getseltitle(v.hFile, 6);
    
    if ((fontsel != NULL)
        && (strcmp(fontsel, auic()->language) != 0)) {
      snprintf(auic()->language, 256, "%s", fontsel);
      aui_langreload();
    }
    
    //-- Transition
    int transition = acopt_getvalue(v.hFile, 7);
    acfg()->fadeframes = (transition - 1) * 2;
    
    //-- Reload Font
    if (font_reloaded) {
      aui_cfg_reloadfonts();
    }
    
    // set new color space
    if (colorspace_refreshed) {
      aui_cfg_setcolorspace();
    }
    
    save_to_file = 1;
  }
  
  //-- Window
  aw_destroy(v.hWin);
  
  //-- Save
  if (save_to_file) {
    aui_cfg_save();
  }
}
