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
 * AROMA File Manager Themes
 *
 */
void aui_themereload() {
  printf("acfg_init_ex() - start\n");
  acfg_init_ex(1);
  printf("acfg_init_ex() - end\n");
  
  if (strcmp(auic()->theme, "") != 0) {
    //-- Init Theme Prop Path
    char themename[256];
    snprintf(themename, 256, "%s/themes/%s/theme.prop", AROMA_DIR,
             auic()->theme);
    snprintf(acfg()->themename, 64, "%s", auic()->theme);
    //-- Read and Parse
    char * propstr = aui_readfromzip(themename);
    
    if (propstr) {
      int i = 0;
      
      for (i = 0; i < AROMA_THEME_CNT; i++) {
        char * key = atheme_key(i);
        char * val = aui_parsepropstring(propstr, key);
        
        if (val != NULL) {
          if (strcmp(val, "") != 0) {
            snprintf(themename, 256,
                     "themes/%s/%s",
                     auic()->theme, val);
            atheme_create(key, themename);
          }
          
          free(val);
        }
      }
      
      aui_setthemecolor(propstr, "color.winbg",
                        &acfg()->winbg);
      aui_setthemecolor(propstr, "color.winbg_g",
                        &acfg()->winbg_g);
      aui_setthemecolor(propstr, "color.winfg",
                        &acfg()->winfg);
      aui_setthemecolor(propstr, "color.winfg_gray",
                        &acfg()->winfg_gray);
      aui_setthemecolor(propstr, "color.dialogbg",
                        &acfg()->dialogbg);
      aui_setthemecolor(propstr, "color.dialogbg_g",
                        &acfg()->dialogbg_g);
      aui_setthemecolor(propstr, "color.dialogfg",
                        &acfg()->dialogfg);
      aui_setthemecolor(propstr, "color.textbg",
                        &acfg()->textbg);
      aui_setthemecolor(propstr, "color.textfg",
                        &acfg()->textfg);
      aui_setthemecolor(propstr, "color.textfg_gray",
                        &acfg()->textfg_gray);
      aui_setthemecolor(propstr, "color.controlbg",
                        &acfg()->controlbg);
      aui_setthemecolor(propstr, "color.controlbg_g",
                        &acfg()->controlbg_g);
      aui_setthemecolor(propstr, "color.controlfg",
                        &acfg()->controlfg);
      aui_setthemecolor(propstr, "color.selectbg",
                        &acfg()->selectbg);
      aui_setthemecolor(propstr, "color.selectbg_g",
                        &acfg()->selectbg_g);
      aui_setthemecolor(propstr, "color.selectfg",
                        &acfg()->selectfg);
      aui_setthemecolor(propstr, "color.titlebg",
                        &acfg()->titlebg);
      aui_setthemecolor(propstr, "color.titlebg_g",
                        &acfg()->titlebg_g);
      aui_setthemecolor(propstr, "color.titlefg",
                        &acfg()->titlefg);
      aui_setthemecolor(propstr, "color.dlgtitlebg",
                        &acfg()->dlgtitlebg);
      aui_setthemecolor(propstr, "color.dlgtitlebg_g",
                        &acfg()->dlgtitlebg_g);
      aui_setthemecolor(propstr, "color.dlgtitlefg",
                        &acfg()->dlgtitlefg);
      aui_setthemecolor(propstr, "color.scrollbar",
                        &acfg()->scrollbar);
      aui_setthemecolor(propstr, "color.navbg",
                        &acfg()->navbg);
      aui_setthemecolor(propstr, "color.navbg_g",
                        &acfg()->navbg_g);
      aui_setthemecolor(propstr, "color.border",
                        &acfg()->border);
      aui_setthemecolor(propstr, "color.border_g",
                        &acfg()->border_g);
      aui_setthemecolor(propstr, "color.progressglow",
                        &acfg()->progressglow);
      aui_setthemeconfig(propstr, "config.roundsize",
                         &acfg()->roundsz);
      aui_setthemeconfig(propstr, "config.button_roundsize",
                         &acfg()->btnroundsz);
      aui_setthemeconfig(propstr, "config.window_roundsize",
                         &acfg()->winroundsz);
      // aui_setthemeconfig(propstr, "config.transition_frame",  &acfg()->fadeframes);
      free(propstr);
    }
    else {
      snprintf(acfg()->themename, 64, "");
    }
  }
  
  aui_isbgredraw = 1;
}
