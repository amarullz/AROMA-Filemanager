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
 * AROMA Filemanager Clipboard
 *
 */
static int auic_s = 0;		//-- Current add pos
static int auic_n = 0;		//-- Number of clipboard
static char ** auic_i = NULL;	//-- Clipboard data
static byte auic_t = 0;		//-- Clipboard type
static char * auic_p = NULL;	//-- Clipboard path

void auic_release() {
  int i;
  
  if (auic_i != NULL) {
    for (i = 0; i < auic_n; i++) {
      free(auic_i[i]);
    }
    
    free(auic_i);
    auic_i = NULL;
  }
  
  if (auic_p != NULL) {
    free(auic_p);
  }
}

void auic_init(int type, int count, char * path) {
  auic_release();
  auic_t = type;
  auic_n = count;
  auic_s = 0;
  auic_p = strdup(path);
  auic_i = malloc(sizeof(char *) * auic_n);
  int i;
  
  for (i = 0; i < auic_n; i++) {
    auic_i[i] = NULL;
  }
}

byte auic_add(char * file) {
  if (auic_s >= auic_n) {
    return 0;
  }
  
  auic_i[auic_s++] = strdup(file);
  return 1;
}

char * auic_getpath(int id) {
  if ((id < 0) || (id >= auic_n)) {
    return NULL;
  }
  
  char * file = auic_i[id];
  char * path = auic_p;
  int len = strlen(path) + strlen(file) + 1;
  char * newpath = malloc(len);
  snprintf(newpath, len, "%s%s", path, file);
  return newpath;
}

char * auic_destpath(int id, char * path) {
  if ((id < 0) || (id >= auic_n)) {
    return NULL;
  }
  
  char * file = auic_i[id];
  int len = strlen(path) + strlen(file) + 1;
  char * newpath = malloc(len);
  snprintf(newpath, len, "%s%s", path, file);
  return newpath;
}

char * auic_getfile(int id) {
  if ((id < 0) || (id >= auic_n)) {
    return NULL;
  }
  
  return auic_i[id];
}

byte auic_gettype() {
  return auic_t;
}
