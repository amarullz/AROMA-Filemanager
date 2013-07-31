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
 * AROMA File Manager Main LIB
 *
 */
void aw_menuset(AWMENUITEMP mi, int id, char * title_id, int ico_id) {
  snprintf(mi[id].title, 128, "%s", alang_get(title_id));
  mi[id].icon = aui_icons(ico_id);
}

char * aui_kilosize(char * buf, int val) {
  if (val > 524288) {
    snprintf(buf, 64, "%1.2f GB", ((float)val) / 1048576);
  }
  else if (val > 1024) {
    snprintf(buf, 64, "%1.1f MB", ((float)val) / 1024);
  }
  else {
    snprintf(buf, 64, "%i KB", val);
  }
  
  return buf;
}


//*
//* Expand Path Size
//*
void aui_setpath(char ** buf, char * pre, char * post, byte addslash) {
  int prel = strlen(((pre == NULL) ? "" : pre));
  int postl = strlen(((post == NULL) ? "" : post));
  
  if ((prel + postl == 0) && (!addslash)) {
    return;
  }
  
  int len = prel + postl + 1 + addslash;
  char * newdir = malloc(len);
  snprintf(newdir, len, ((addslash) ? "%s%s/" : "%s%s"),
           ((pre == NULL) ? "" : pre), ((post == NULL) ? "" : post));
           
  if (*buf != NULL) {
    free(*buf);
  }
  
  *buf = newdir;
}


//*
//* Read Strings From filesystem
//*
char * aui_readfromfs(char * name) {
  char * buffer = NULL;
  struct stat st;
  
  if (stat(name, &st) < 0) {
    return NULL;
  }
  
  if (st.st_size > MAX_FILE_GETPROP_SIZE) {
    return NULL;
  }
  
  buffer = malloc(st.st_size + 1);
  
  if (buffer == NULL) {
    goto done;
  }
  
  FILE * f = fopen(name, "rb");
  
  if (f == NULL) {
    goto done;
  }
  
  if (fread(buffer, 1, st.st_size, f) != st.st_size) {
    fclose(f);
    goto done;
  }
  
  buffer[st.st_size] = '\0';
  fclose(f);
  return buffer;
done:
  free(buffer);
  return NULL;
}


//*
//* Write Strings into file
//*
void aui_writetofs(char * name, char * value) {
  FILE * fp = fopen(name, "wb");
  
  if (fp != NULL) {
    fwrite(value, 1, strlen(value), fp);
    fclose(fp);
  }
}


//*
//* Read Strings From Temporary File
//*
char * aui_readfromtmp(char * name) {
  char path[256];
  snprintf(path, 256, "%s/%s", AROMA_TMP, name);
  return aui_readfromfs(path);
}

//*
//* Write Strings From Temporary File
//*
void aui_writetotmp(char * name, char * value) {
  char path[256];
  snprintf(path, 256, "%s/%s", AROMA_TMP, name);
  aui_writetofs(path, value);
}


//*
//* Read Strings From ZIP
//*
char * aui_readfromzip(char * name) {
  AZMEM filedata;
  
  if (!az_readmem(&filedata, name, 0)) {
    return NULL;
  }
  
  return filedata.data;
}


//*
//* Parse PROP String
//*
char * aui_parsepropstring(char * bf, char * key) {
  char * result = NULL;
  
  if (bf == NULL) {
    return result;
  }
  
  char * buffer = strdup(bf);
  char * line = strtok(buffer, "\n");
  
  do {
    while (*line && isspace(*line)) {
      ++line;
    }
    
    if (*line == '\0' || *line == '#') {
      continue;
    }
    
    char * equal = strchr(line, '=');
    
    if (equal == NULL) {
      goto done;
    }
    
    char * key_end = equal - 1;
    
    while ((key_end > line) && isspace(*key_end)) {
      key_end--;
    }
    
    key_end[1] = '\0';
    
    if (strcmp(key, line) != 0) {
      continue;
    }
    
    char * val_start = equal + 1;
    
    while (*val_start && isspace(*val_start)) {
      ++val_start;
    }
    
    char * val_end = val_start + strlen(val_start) - 1;
    
    while (val_end > val_start && isspace(*val_end)) {
      --val_end;
    }
    
    val_end[1] = '\0';
    result = strdup(val_start);
    break;
  }
  while ((line = strtok(NULL, "\n")));
  
  free(buffer);
done:
  return result;
}


//*
//* Parse PROP Files
//*
char * aui_parseprop(char * filename, char * key) {
  char * buffer = aui_readfromfs(filename);
  char * result = aui_parsepropstring(buffer, key);
  free(buffer);
  return result;
}


//*
//* Parse PROP from ZIP
//*
char * aui_parsepropzip(char * filename, char * key) {
  char * buffer = aui_readfromzip(filename);
  char * result = aui_parsepropstring(buffer, key);
  free(buffer);
  return result;
}


//*
//* Read Variable
//*
char * aui_getvar(char * name) {
  char path[256];
  snprintf(path, 256, "%s/.__%s.var", AROMA_TMP, name);
  return aui_readfromfs(path);
}


//*
//* Set Variable
//*
void aui_setvar(char * name, char * value) {
  char path[256];
  snprintf(path, 256, "%s/.__%s.var", AROMA_TMP, name);
  aui_writetofs(path, value);
}


//*
//* Append Variable
//*
void aui_appendvar(char * name, char * value) {
  char path[256];
  snprintf(path, 256, "%s/.__%s.var", AROMA_TMP, name);
  FILE * fp = fopen(path, "ab");
  
  if (fp != NULL) {
    fwrite(value, 1, strlen(value), fp);
    fclose(fp);
  }
}


//*
//* Delete Variable
//*
void aui_delvar(char * name) {
  char path[256];
  snprintf(path, 256, "%s/.__%s.var", AROMA_TMP, name);
  unlink(path);
}


//*
//* Prepend Variable
//*
void aui_prependvar(char * name, char * value) {
  char path[256];
  snprintf(path, 256, "%s/.__%s.var", AROMA_TMP, name);
  char * buf = aui_getvar(name);
  FILE * fp = fopen(path, "wb");
  
  if (fp != NULL) {
    fwrite(value, 1, strlen(value), fp);
    
    if (buf != NULL) {
      fwrite(buf, 1, strlen(buf), fp);
    }
    
    fclose(fp);
  }
  
  if (buf != NULL) {
    free(buf);
  }
}


//*
//* Set Colorset From Prop String
//*
void aui_setthemecolor(char * prop, char * key, color * cl) {
  char * val = aui_parsepropstring(prop, key);
  
  if (val != NULL) {
    cl[0] = strtocolor(val);
    free(val);
  }
}


//*
//* Set Drawing Config From Prop String
//*
void aui_setthemeconfig(char * prop, char * key, byte * b) {
  char * val = aui_parsepropstring(prop, key);
  
  if (val != NULL) {
    b[0] = (byte) min(atoi(val), 255);
    free(val);
  }
}


char * aui_strip(char * str, int maxw, byte isbig) {
  int clen = ag_txtwidth(str, isbig);
  
  if (clen < maxw) {
    return strdup(str);
  }
  
  int basepos = 0;
  int i = 0;
  char basestr[64];
  char allstr[128];
  memset(basestr, 0, 64);
  
  for (i = strlen(str) - 2; i >= 0; i--) {
    if (str[i] == '/') {
      basepos = i - 2;
      snprintf(basestr, 63, "%s", &(str[i]));
      
      if (i > 0)
        snprintf(allstr, 127, "/%c%c..%s", str[1],
                 str[2], basestr);
      else {
        snprintf(allstr, 127, "%s", basestr);
      }
      
      break;
    }
  }
  
  if (basepos > 50) {
    basepos = 50;
  }
  
  do {
    if (basepos <= 0) {
      break;
    }
    
    char dirstr[64];
    memset(dirstr, 0, 64);
    memcpy(dirstr, str, basepos);
    snprintf(allstr, 127, "%s..%s", dirstr, basestr);
    clen = ag_txtwidth(allstr, isbig);
    basepos--;
  }
  while (clen >= maxw);
  
  return strdup(allstr);
}


