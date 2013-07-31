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
 * AROMA File Manager Icons
 *
 */
#define UI_ICONSN 45

//* ICONS
static char UI_ICONS_NAME[UI_ICONSN][26] = {
  "tools.cancel",		// 0
  "tools.chmod",		// 1
  "tools.copy",		// 2
  "tools.cut",		// 3
  "tools.delete",		// 4
  "tools.details",	// 5
  "tools.fav",		// 6
  "tools.favadd",		// 7
  "tools.favdel",		// 8
  "tools.info",		// 9
  "tools.menu",		// 10
  "tools.newfolder",	// 11
  "tools.parent",		// 12
  "tools.paste",		// 13
  "tools.refresh",	// 14
  "tools.rename",		// 15
  "tools.selectall",	// 16
  "tools.settings",	// 17
  "tools.showhide",	// 18
  "tools.unselectall",	// 19
  "tools.usb",		// 20
  
  "type.default",		// 21
  "type.folder",		// 22
  "type.linkerror",	// 23
  "type.linkfile",	// 24
  "type.linkfolder",	// 25
  "type.zip",		// 26
  
  "ime.shiftlow",		// 27
  "ime.shiftup",		// 28
  "ime.shiftcaps",	// 29
  "ime.123",		// 30
  "ime.abc",		// 31
  "ime.backspace",	// 32
  "tools.ok",		// 33
  "ime.left",		// 34
  "ime.right",		// 35
  "type.folder.fav",	// 36
  
  "dialog.alert",		// 37
  "dialog.info",		// 38
  "dialog.question",	// 39
  
  "tools.terminal",	// 40
  "ime.enter",      // 41
  "ime.tab",         // 42
  "ime.up",         // 43
  "ime.down"         // 44
};

PNGCANVAS UI_ICONS[UI_ICONSN];
PNGCANVAS * aui_icons(int id) {
  return &UI_ICONS[id];
}

void aui_load_icons() {
  int i;
  char iconsetdir[256];
  snprintf(iconsetdir, 256, "icons/%s", auic()->iconset);
  
  for (i = 0; i < UI_ICONSN; i++) {
    char path[256];
    snprintf(path, 256, "%s/%s", iconsetdir, UI_ICONS_NAME[i]);
    apng_load(&UI_ICONS[i], path);
  }
}

void aui_unload_icons() {
  int i;
  
  for (i = 0; i < UI_ICONSN; i++) {
    apng_close(&UI_ICONS[i]);
  }
}
