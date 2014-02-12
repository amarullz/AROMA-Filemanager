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
 * AROMA File Manager Configuration
 *
 */
static AARRAYP aui_cfg_array = NULL;
typedef struct {
  byte tooltext;
  byte fontsize;
  byte automount;
  byte showhidden;
  byte colorspace;
  char fontfamily[256];
  char iconset[256];
  char language[256];
  char theme[256];
} AUICV, *AUICVP;
AUICV auicv;
AUICVP auic() {
  return &auicv;
}

void aui_cfg_init() {
  auicv.tooltext = 1;
  auicv.fontsize = 2;
  auicv.automount = 0;
  auicv.colorspace = 1;
  auicv.showhidden = 0;
  acfg()->fadeframes = 4;
  snprintf(auicv.fontfamily, 256, "Droid Sans");
  snprintf(auicv.iconset, 256, "Generic");
  snprintf(auicv.language, 256, "English");
  snprintf(auicv.theme, 256, "");
}

int aui_cfg_btnFH() {
  if (!auic()->tooltext) {
    return 0;
  }
  
  return ag_fontheight(0);
}

void aui_cfg_setcolorspace() {
  if ((auicv.colorspace < 1) || (auicv.colorspace > 4)) {
    auicv.colorspace = 1;
  }
  if (auicv.colorspace == 1) {  //rgba
      ag_changecolorspace(0, 8, 16, 24);
    }
    else if (auicv.colorspace == 2) {   //abgr
      ag_changecolorspace(24, 16, 8, 0);
    }
    else if (auicv.colorspace == 3) {   //argb
      ag_changecolorspace(8, 16, 24, 0);
    }
    else if (auicv.colorspace == 4) {  //bgra
      ag_changecolorspace(16, 8, 0, 24);
    }
  }

void aui_cfg_reloadfonts() {
  byte fszs[3] = { 9, 11, 12 };
  byte fszb[3] = { 12, 14, 17 };
  
  if ((auicv.fontsize < 1) || (auicv.fontsize > 3)) {
    auicv.fontsize = 2;
  }
  
  char fontdir[256];
  snprintf(fontdir, 256, "%s/fonts/%s/", AROMA_DIR, auicv.fontfamily);
  char fonts[256];
  snprintf(fonts, 256, "");
  AZREADDIRP r = az_readdir(fontdir);
  
  if (r != NULL) {
    int i = 0;
    
    for (i = 0; i < r->n; i++) {
      if (r->t[i] == 0) {
        char * vfont = strdup(fonts);
        snprintf(fonts, 256, "%s%s;", vfont, r->f[i]);
        free(vfont);
      }
      
      fonts[strlen(fonts) - 1] = 0;
    }
    
    az_readdir_free(r);
    ag_loadsmallfont(fonts, fszs[auicv.fontsize - 1], fontdir);
    ag_loadbigfont(fonts, fszb[auicv.fontsize - 1], fontdir);
    ag_loadfixedfont("fixed.ttf;", fszs[auicv.fontsize - 1], AROMA_DIR "/common/");
    printf("Load Font: %s - dir: %s\n", fonts, fontdir);
  }
}

//*
//* GET TOOL TITLE
//*
char * aui_tbtitle(int id) {
  if (!auic()->tooltext) {
    return NULL;
  }
  
  char * titlekey[11] = {
    "tools.newfolder",	// 0
    "tools.favorite",	// 1
    "tools.selectall",	// 2
    "tools.unselect",	// 3
    "tools.copy",	// 4
    "tools.cut",	// 5
    "tools.paste",	// 6
    "tools.menu",	// 7
    "cancel",	// 8
    "done",		// 9
    "settings.reset"	// 10
  };
  return alang_get(titlekey[id]);
}

char * aui_cfg_build(AARRAYP a) {
  if (!a) {
    return NULL;
  }
  
  int i;
  int len = 1;
  
  for (i = 0; i < a->length; i++) {
    if (a->items[i].key != NULL) {
      len += strlen(a->items[i].key);
      int j = 0;
      int k = strlen(a->items[i].val);
      
      for (j = 0; j < k; j++) {
        if (a->items[i].val[i] == '\n') {
          len++;
        }
        
        len++;
      }
      
      len += 2;
    }
  }
  
  char * buf = malloc(len);
  memset(buf, 0, len);
  int pos = 0;
  
  for (i = 0; i < a->length; i++) {
    if (a->items[i].key != NULL) {
      int j = 0;
      int k = strlen(a->items[i].key);
      
      for (j = 0; j < k; j++) {
        buf[pos++] = a->items[i].key[j];
      }
      
      buf[pos++] = '=';
      k = strlen(a->items[i].val);
      
      for (j = 0; j < k; j++) {
        if (a->items[i].val[j] == '\n') {
          buf[pos++] = '\\';
        }
        
        buf[pos++] = a->items[i].val[j];
      }
      
      buf[pos++] = '\n';
    }
  }
  
  return buf;
}

//*
//* Parse AMS
//*
byte aui_cfg_parse(AARRAYP * arr, char * bufs) {
  if (bufs == NULL) {
    return 0;
  }
  
  if (*arr == NULL) {
    *arr = aarray_create();
  }
  
  char * buf = strdup(bufs);
  char * vuf = buf;
  
  if (strlen(vuf) > 3) {
    //-- Check UTF-8 File Header
    if ((vuf[0] == 0xEF) && (vuf[1] == 0xBB) && (vuf[2] == 0xBF)) {
      vuf += 3;
    }
  }
  
  byte state = 0;
  byte slash = 0;
  char c = 0;
  char pc = 0;
  char * key = NULL;
  char * val = NULL;
  
  while ((c = *vuf)) {
    if (state == 0) {
      //-- First State
      if (!isspace(c)) {
        key = vuf;
        state = 2;
      }
      else if (c == '#') {
        state = 1;
      }
    }
    else if (state == 1) {
      //-- Comment
      if (c == '\n') {
        state = 0;
      }
    }
    else if (state == 2) {
      if (isspace(c) || (c == '=') || (c == '\n')) {
        *vuf = 0;
        
        if (c == '=') {
          state = 3;
        }
        else if (c == '\n') {
          state = 0;
        }
      }
    }
    else if (state == 3) {
      if (!isspace(c)) {
        val = vuf;
        state = 4;
        pc = c;
      }
      else if (c == '\n') {
        state = 0;
      }
    }
    else if (state == 4) {
      if (((c == '\n') && (pc != '\\')) || (*(vuf + 1) == 0)) {
        if ((c == '\n') && (pc != '\\')) {
          *vuf = 0;
        }
        
        //-- Cleanup backslashes
        int i;
        int j = 0;
        int l = strlen(val);
        
        for (i = 0; i < l; i++) {
          if ((val[i] == '\\')
              && (val[i + 1] == '\n')) {
            continue;
          }
          
          val[j++] = val[i];
        }
        
        val[j] = 0;
        //-- Save Lang Value
        aarray_set(*arr, key, val);
        //-- End Of String
        state = 0;
      }
      
      pc = c;
    }
    
    vuf++;
  }
  
  free(buf);
  return 1;
}

void aui_cfg_savecalib() {
  /*
  AW_CALIBMATRIXP matrix = atouch_getcalibmatrix();
  char data_calib[256];
  if (atouch_usematrix()) {
  	snprintf(data_calib, 256, "%f;%f;%f;%f;%f;%f;%f;%i",
  		 matrix->An, matrix->Bn, matrix->Cn,
  		 matrix->Dn, matrix->En, matrix->Fn,
  		 matrix->Divider, atouch_gethack());
  } else {
  	snprintf(data_calib, 256, "");
  }
  aarray_set(aui_cfg_array, "calibration", data_calib);
  */
}

byte aui_cfg_loadcalib() {
  return 1;
  /*
  byte res = 0;
  char *calibdata = aarray_get(aui_cfg_array, "calibration");
  if ((calibdata) && (strcmp(calibdata, "") != 0)) {
  	AW_CALIBMATRIX matrix;
  	char *buf = strdup(calibdata);
  	char *tok;
  
  	if (!(tok = strtok(buf, ";")))
  		goto done;
  	matrix.An = (float)strtof(tok, NULL);
  
  	if (!(tok = strtok(NULL, ";")))
  		goto done;
  	matrix.Bn = (float)strtof(tok, NULL);
  
  	if (!(tok = strtok(NULL, ";")))
  		goto done;
  	matrix.Cn = (float)strtof(tok, NULL);
  
  	if (!(tok = strtok(NULL, ";")))
  		goto done;
  	matrix.Dn = (float)strtof(tok, NULL);
  
  	if (!(tok = strtok(NULL, ";")))
  		goto done;
  	matrix.En = (float)strtof(tok, NULL);
  
  	if (!(tok = strtok(NULL, ";")))
  		goto done;
  	matrix.Fn = (float)strtof(tok, NULL);
  
  	if (!(tok = strtok(NULL, ";")))
  		goto done;
  	matrix.Divider = (float)strtof(tok, NULL);
  
  	if (!(tok = strtok(NULL, ";")))
  		goto done;
  	byte usealt = (byte) atoi(tok);
  	atouch_matrix_calibrate(&matrix);
  	atouch_sethack(usealt);
  
  	res = 1;
  done:
  	free(buf);
  }
  
  return res;
  */
}

void aui_cfg_fromarray() {
  char * o;
  o = aarray_get(aui_cfg_array, "automount");
  
  if (o != NULL) {
    auic()->automount = atoi(o);
  }
  
  o = aarray_get(aui_cfg_array, "colorspace");
  
  if (o != NULL) {
    auic()->colorspace = atoi(o);
  }
  
  o = aarray_get(aui_cfg_array, "tooltext");
  
  if (o != NULL) {
    auic()->tooltext = atoi(o);
  }
  
  o = aarray_get(aui_cfg_array, "fontsize");
  
  if (o != NULL) {
    auic()->fontsize = atoi(o);
  }
  
  o = aarray_get(aui_cfg_array, "fadeframes");
  
  if (o != NULL) {
    acfg()->fadeframes = atoi(o);
  }
  
  o = aarray_get(aui_cfg_array, "showhidden");
  
  if (o != NULL) {
    auic()->showhidden = atoi(o);
  }
  
  o = aarray_get(aui_cfg_array, "fontfamily");
  
  if (o != NULL) {
    snprintf(auicv.fontfamily, 128, "%s", o);
  }
  
  o = aarray_get(aui_cfg_array, "iconset");
  
  if (o != NULL) {
    snprintf(auicv.iconset, 128, "%s", o);
  }
  
  o = aarray_get(aui_cfg_array, "language");
  
  if (o != NULL) {
    snprintf(auicv.language, 128, "%s", o);
  }
  
  o = aarray_get(aui_cfg_array, "theme");
  
  if (o != NULL) {
    snprintf(auicv.theme, 128, "%s", o);
  }
}

void aui_cfg_savechange() {
  char cfgname[256];
  snprintf(cfgname, 256, "%s.cfg", getArgv(1));
  char * buf = aui_cfg_build(aui_cfg_array);
  
  if (buf) {
    aui_writetofs(cfgname, buf);
    free(buf);
  }
}

void aui_cfg_save() {
  char o[256];
  snprintf(o, 256, "%i", auic()->tooltext);
  aarray_set(aui_cfg_array, "tooltext", o);
  snprintf(o, 256, "%i", auic()->showhidden);
  aarray_set(aui_cfg_array, "showhidden", o);
  snprintf(o, 256, "%i", auic()->fontsize);
  aarray_set(aui_cfg_array, "fontsize", o);
  snprintf(o, 256, "%i", auic()->automount);
  aarray_set(aui_cfg_array, "automount", o);
  snprintf(o, 256, "%i", auic()->colorspace);
  aarray_set(aui_cfg_array, "colorspace", o);
  snprintf(o, 256, "%i", acfg()->fadeframes);
  aarray_set(aui_cfg_array, "fadeframes", o);
  aarray_set(aui_cfg_array, "fontfamily", auicv.fontfamily);
  aarray_set(aui_cfg_array, "iconset", auicv.iconset);
  aarray_set(aui_cfg_array, "language", auicv.language);
  aarray_set(aui_cfg_array, "theme", auicv.theme);
  //aui_cfg_savecalib();
  aui_cfg_savechange();
}

void aui_cfg_loadcfg() {
  aui_cfg_array = aarray_create();
  char cfgname[256];
  snprintf(cfgname, 256, "%s.cfg", getArgv(1));
  char * buf = aui_readfromfs(cfgname);
  
  if (buf == NULL) {
    aui_cfg_save();
    return;
  }
  
  aui_cfg_parse(&aui_cfg_array, buf);
  free(buf);
  aui_cfg_fromarray();
  aui_is_show_tips = 0;
}

void aui_cfg_release() {
  if (aui_cfg_array != NULL) {
    aarray_free(aui_cfg_array);
  }
}
