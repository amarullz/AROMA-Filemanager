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
 * AROMA File Manager Language Initializing
 *
 */
void aui_releaselang() {
  alangd_release();
}

void aui_initlang() {
  alangd_init();
}

void aui_langreload() {
  alang_release();
  
  if (strcmp(auic()->language, "English") != 0) {
    char langpath[256];
    snprintf(langpath, 256, "%s/langs/%s/lang.prop", AROMA_DIR,
             auic()->language);
    alang_load(langpath);
  }
}
