/*
 * Descriptions:
 * -------------
 * KL Flasher Code
 *
 */
#include "../../../libs/minzip/Zip.h"

 ZipArchive* proczip(ZipArchive *za){
    return za;
}

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void choose_kernel(char * fl,char * full_fl) {
  printf("Choose_kernel_setting\n");
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
  
  //-- Kernel Flash Location : 1
  acopt_addgroup(v.hFile, "Kernel Flash Location", "");
  acopt_add(v.hFile, "/system", "",0);
  acopt_add(v.hFile, "/data", "",1);
  acopt_add(v.hFile, "/sdcard", "",0);

  //-- IMG To Flash : 2
  acopt_addgroup(v.hFile, "Pick Kernel IMG", fl);

        ZipArchive za;
        ZipArchive* zip;
        int err = mzOpenZipArchive(full_fl, &za);
        if (err != 0) {
                    aw_destroy(v.hWin);
                    return 0;
                }

                zip = proczip(&za);
                int i3;int i2 = 0;
                for (i3 = 0; i3 < zip->numEntries; i3++) {
                     if (strstr(zip->pEntries[i3].fileName, ".img") != NULL) {

	                      int endpos = strstr(zip->pEntries[i3].fileName, ".img") - zip->pEntries[i3].fileName;
	                      char imgname[zip->pEntries[i3].fileNameLen];
	                      strncpy(imgname, zip->pEntries[i3].fileName, zip->pEntries[i3].fileNameLen);
	                      imgname[zip->pEntries[i3].fileNameLen]='\0';
	                      if (EndsWith(imgname,".img")){
	                        acopt_add(v.hFile, imgname, "", 0);
	                        i2++;
	                      }
                     }
                  }

                      if (i2 == 0){
					  AWMENUITEM mi[2];
					  aw_menuset(mi, 0, "ok", 33);
					  char info[256];
					  snprintf(info, 256, "No Boot.img Found, Please Try Another Zip");
					  aw_multiconfirm_ex(v.hWin, "Error Has Occured", info,
					                                aui_icons(37),
					                                mi, 1, 1);
					  mzCloseZipArchive(zip);
					  //-- Window
					  aw_destroy(v.hWin); return;
                        }
                




  //-- TOOLS
  v.b1 = imgbtn(v.hWin, v.pad, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[33], aui_tbtitle(9), 1, 12); //-- Done
  //v.b2 = imgbtn(v.hWin, v.pad + v.btnW, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[14], aui_tbtitle(10), 1, 22); //-- Reset
  v.b2 = imgbtn(v.hWin, v.pad + v.btnW * 2, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[0], aui_tbtitle(8), 1, 20); //-- Cancel
  //-- Show Window
  aw_show_ex(v.hWin, 2, 0, v.hFile);
  // aw_setfocus(v.hWin,v.hFile);
  aui_changetitle(&v, fl);
  byte ondispatch = 1;
  byte apply = 1;
  
  do {
    dword msg = aw_dispatch(v.hWin);
    
    switch (aw_gm(msg)) {
      case 12: {
      	  if (apply){
		    char* bootimg = "/tmp/boot.img";
		    char* zippath = acopt_getseltitle(v.hFile, 2);
		    char* klocation = acopt_getseltitle(v.hFile, 1);

		    unlink(bootimg);
		    int fdk = creat(bootimg, 0755);
		    if (fdk < 0) {
		        LOGE("Can't make /tmp/%s\n", zippath);
		        AWMENUITEM mi[2];
				  aw_menuset(mi, 0, "ok", 33);
				  char info[256];
				  snprintf(info, 256, "Process Failed To Extract Kernel, Please Try Again");
				  aw_multiconfirm_ex(v.hWin, "Error Has Occured", info,
				                                aui_icons(37),
				                                mi, 1, 1);
		        break;
		    }
		    bool ok1 = mzExtractZipEntryToFile(zip, mzFindZipEntry(zip, zippath), fdk);
		    close(fdk);
		    if (!ok1) {
		        LOGE("Can't copy %s\n", zippath);
		        AWMENUITEM mi[2];
				  aw_menuset(mi, 0, "ok", 33);
				  char info[256];
				  snprintf(info, 256, "Process Failed To Write Kernel, Please Try Again");
				  aw_multiconfirm_ex(v.hWin, "Error Has Occured", info,
				                                aui_icons(37),
				                                mi, 1, 1);
		        break;
		    } 

		    
		    pid_t pid;
		    pid = fork();
		    if(pid == 0){
		      char* args2[]={"useless", full_fl, basename(full_fl), klocation, zippath, basename(zippath),NULL};
		      /* left for illustration purposes
		      args2[1] = full_fl;//Zip Location
		      args2[2] = full_fl;//Zip Name
		      args2[3] = klocation;//Flash Location
		      args2[4] = zippath;//Boot IMG Path
		      args2[5] = basename(zippath);// Boot IMG Name
		      args2[6] = NULL;*/
		      if (execv("/tmp/kl-script", args2)<0){
		        LOGE("Error %s\n", strerror(errno));
		        AWMENUITEM mi[2];
				  aw_menuset(mi, 0, "ok", 33);
				  char info[256];
				  snprintf(info, 256, "Process Failed To Execute Script,\nError: %s,\nPlease Try Again", strerror(errno));
				  aw_multiconfirm_ex(v.hWin, "Error Has Occured", info,
				                                aui_icons(37),
				                                mi, 1, 1);
		        break;
		      }
		     
		    }
		  }
          ondispatch = 0;
        }
        break;
        
      case 20: {
          //-- CANCEL
          ondispatch = 0;
          apply = 0;
        }
        break;
    }
  }
  while (ondispatch);
  



  mzCloseZipArchive(zip);
  //-- Window
  aw_destroy(v.hWin);
  
 }

