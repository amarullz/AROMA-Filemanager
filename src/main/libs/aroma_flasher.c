/*
 * Descriptions:
 * -------------
 * KL Flasher Code
 *
 */
#include "../../../libs/minzip/Zip.h"

#define ASSUMED_UPDATE_BINARY_NAME  "META-INF/com/google/android/update-binary"
#define ASSUMED_UPDATE_SCRIPT_NAME  "META-INF/com/google/android/update-script"
static const int VERIFICATION_PROGRESS_TIME = 60;
static const float VERIFICATION_PROGRESS_FRACTION = 0.25;
static const float DEFAULT_FILES_PROGRESS_FRACTION = 0.4;
static const float DEFAULT_IMAGE_PROGRESS_FRACTION = 0.1;
enum { INSTALL_SUCCESS, INSTALL_ERROR, INSTALL_CORRUPT, INSTALL_UPDATE_SCRIPT_MISSING, INSTALL_UPDATE_BINARY_MISSING };


 ZipArchive* proczip(ZipArchive *za) {
	return za;
}
int EndsWith(const char *str, const char *suffix) {
	if (!str || !suffix)
			return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix >  lenstr)
			return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}
void mountit(char* mount) {
	if (!ismounted(mount)) {
		alib_exec("/sbin/mount", mount);
	}
}
int waitpidv2(int pid,AUI_VARS v) {
	int status;
	waitpid(pid, &status, 0);
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) {
			AWMENUITEM mi[2];
			aw_menuset(mi, 0, "ok", 33);
			char info[256];
			snprintf(info, 256, "Script Executed Successfully!");
			aw_multiconfirm_ex(v.hWin, "Success", info, aui_icons(37), mi, 1, 1);
			fprintf(apipe(), "ui_print\n");
			fprintf(apipe(), "ui_print Script Executed Successfully!\n");
			return 1;
		} else {
			AWMENUITEM mi[2];
			aw_menuset(mi, 0, "ok", 33);
			char info[256];
			snprintf(info, 256, "Process Failed To Execute Script,\nError: %s,\nPlease Try Again", strerror(WTERMSIG(status)));
			aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
			fprintf(apipe(), "ui_print\n");
			fprintf(apipe(), "ui_print Process Failed To Execute Script,\nError: %s,\nPlease Try Again", strerror(WTERMSIG(status)));
		}
	} else if (WIFSIGNALED(status)) {
		AWMENUITEM mi[2];
		aw_menuset(mi, 0, "ok", 33);
		char info[256];
		snprintf(info, 256, "Process terminated by signal %d\n", WTERMSIG(status));
		aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
		fprintf(apipe(), "ui_print\n");
		fprintf(apipe(), "ui_print Script terminated by signal %d\n", WTERMSIG(status));
	}
	return 0;
}

void test(int status,AUI_VARS v){
		AWMENUITEM mi[2];
		aw_menuset(mi, 0, "ok", 33);
		char info[256];
		snprintf(info, 256, "Process terminated by signal %d\n", (status));
		aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
}
void test2(char* status,AUI_VARS v){
		AWMENUITEM mi[2];
		aw_menuset(mi, 0, "ok", 33);
		char info[256];
		snprintf(info, 256, "Process terminated by signal %s\n", (status));
		aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
}

static int try_update_binary(const char *path, ZipArchive *zip,AUI_VARS v) {

    const ZipEntry* binary_entry =
            mzFindZipEntry(zip, ASSUMED_UPDATE_BINARY_NAME);
    if (binary_entry == NULL) {
        const ZipEntry* update_script_entry =
                mzFindZipEntry(zip, ASSUMED_UPDATE_SCRIPT_NAME);
        if (update_script_entry != NULL) {
            LOGE("Amend scripting (update-script) is no longer supported.\n");
            LOGE("Amend scripting was deprecated by Google in Android 1.5.\n");
            LOGE("It was necessary to remove it when upgrading to the ClockworkMod 3.0 Gingerbread based recovery.\n");
            LOGE("Please switch to Edify scripting (updater-script and update-binary) to create working update zip packages.\n");
            return INSTALL_UPDATE_BINARY_MISSING;
        }

        mzCloseZipArchive(zip);
        return INSTALL_UPDATE_BINARY_MISSING;
    }

    char* binary = "/tmp/update_binary_2";
    unlink(binary);
    int fd = creat(binary, 0755);
    if (fd < 0) {
        mzCloseZipArchive(zip);
        fprintf(apipe(),"Can't make %s\n", binary);
        return 0;
    }
    bool ok = mzExtractZipEntryToFile(zip, binary_entry, fd);
    close(fd);

    if (!ok) {
        fprintf(apipe(),"Can't copy %s\n", ASSUMED_UPDATE_BINARY_NAME);
        mzCloseZipArchive(zip);
        return 0;
    }

    int pipefd[2];
    pipe(pipefd);

    // When executing the update binary contained in the package, the
    // arguments passed are:
    //
    //   - the version number for this interface
    //
    //   - an fd to which the program can write in order to update the
    //     progress bar.  The program can write single-line commands:
    //
    //        progress <frac> <secs>
    //            fill up the next <frac> part of of the progress bar
    //            over <secs> seconds.  If <secs> is zero, use
    //            set_progress commands to manually control the
    //            progress of this segment of the bar
    //
    //        set_progress <frac>
    //            <frac> should be between 0.0 and 1.0; sets the
    //            progress bar within the segment defined by the most
    //            recent progress command.
    //
    //        firmware <"hboot"|"radio"> <filename>
    //            arrange to install the contents of <filename> in the
    //            given partition on reboot.
    //
    //            (API v2: <filename> may start with "PACKAGE:" to
    //            indicate taking a file from the OTA package.)
    //
    //            (API v3: this command no longer exists.)
    //
    //        ui_print <string>
    //            display <string> on the screen.
    //
    //   - the name of the package zip file.
    //

    char** args = malloc(sizeof(char*) * 5);
    args[0] = binary;
    args[1] = "3";   // defined in Android.mk
    args[2] = apipe();
    //sprintf(args[2], "%d", pipefd[1]);
    args[3] = path;
    args[4] = NULL;

    pid_t pid = fork();
    if (pid == 0) {
        setenv("UPDATE_PACKAGE", path, 1);
        close(pipefd[0]);
        execv(binary, args);
        fprintf(apipe(), "E:Can't run %s (%s)\n", binary, strerror(errno));
        //test2(strerror(errno),v);
        _exit(errno);
    }
    close(pipefd[1]);

    char buffer[1024];
    FILE* from_child = fdopen(pipefd[0], "r");
    while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
        char* command = strtok(buffer, " \n");
        if (command == NULL) {
            continue;
        } else if (strcmp(command, "ui_print") == 0) {
            char* str = strtok(NULL, "\n");
            if (str) {
                fprintf(apipe(),"%s", str);
            } else {
                fprintf(apipe(),"\n");
            }
        } else {
            fprintf(apipe(),"unknown command [%s]\n", command);
        }
    }
    fclose(from_child);

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(apipe(),"Error in %s\n(Status %d)\n\n", path, WEXITSTATUS(status));
        mzCloseZipArchive(zip);
        return 0;
    }


    mzCloseZipArchive(zip);
    return 1;
}

void choose_kernel(char * full_fl, int isZip) {
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

	ZipArchive za;
	ZipArchive* zip;
	if (isZip) {
		//-- IMG To Flash : 2
		acopt_addgroup(v.hFile, "Pick Kernel IMG", "");

		int err = mzOpenZipArchive(full_fl, &za);
		if (err != 0) {
			aw_destroy(v.hWin);
			return 0;
		}
		zip = proczip(&za);
		int i3;
		int i2 = 0;
		for (i3 = 0; i3 < zip->numEntries; i3++) {
			if (strstr(zip->pEntries[i3].fileName, ".img") != NULL) {
				int endpos = strstr(zip->pEntries[i3].fileName, ".img") - zip->pEntries[i3].fileName;
				char imgname[zip->pEntries[i3].fileNameLen];
				strncpy(imgname, zip->pEntries[i3].fileName, zip->pEntries[i3].fileNameLen);
				imgname[zip->pEntries[i3].fileNameLen]='\0';
				if (EndsWith(imgname,".img")) {
					acopt_add(v.hFile, imgname, "", 1);
					i2++;
				}
			}
		}
		if (i2 == 0) {
			AWMENUITEM mi[2];
			aw_menuset(mi, 0, "ok", 33);
			char info[256];
			snprintf(info, 256, "No Boot.img Found, Please Try Another Zip");
			aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
			mzCloseZipArchive(zip);
			//-- Window
			aw_destroy(v.hWin);
			return;
		}
			//-- Flash Module : 3
			acopt_addgroup(v.hFile, "Flash Zip Aswell", "");
			acopt_add(v.hFile, "Yes", "Would Flash Modules If Available",1);
			acopt_add(v.hFile, "No", "",0);
	}
	//-- TOOLS
	v.b1 = imgbtn(v.hWin, v.pad, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[33], aui_tbtitle(9), 1, 12);
	//-- Done
	//v.b2 = imgbtn(v.hWin, v.pad + v.btnW, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[14], aui_tbtitle(10), 1, 22); //-- Reset
	v.b2 = imgbtn(v.hWin, v.pad + v.btnW * 2, v.btnY - v.btnFH, v.btnW, v.btnH + v.btnFH, &UI_ICONS[0], aui_tbtitle(8), 1, 20);
	//-- Cancel
	//-- Show Window
	aw_show_ex(v.hWin, 2, 0, v.hFile);
	// aw_setfocus(v.hWin,v.hFile);
	aui_changetitle(&v, basename(full_fl));
	byte ondispatch = 1;
	byte apply = 1;
	do {
		dword msg = aw_dispatch(v.hWin);
		switch (aw_gm(msg)) {
			case 12: {
				if (apply) {
					char* bootimg = "/tmp/boot.img";
					char* klocation = acopt_getseltitle(v.hFile, 1);
					mountit(klocation);
					if (isZip) {
						char* imgpath = acopt_getseltitle(v.hFile, 2);
						unlink(bootimg);
						int fdk = creat(bootimg, 0755);
						if (fdk < 0) {
							LOGE("Can't make /tmp/%s\n", imgpath);
							AWMENUITEM mi[2];
							aw_menuset(mi, 0, "ok", 33);
							char info[256];
							snprintf(info, 256, "Process Failed To Extract Kernel, Please Try Again");
							aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
							break;
						}
						bool ok1 = mzExtractZipEntryToFile(zip, mzFindZipEntry(zip, imgpath), fdk);
						close(fdk);
						if (!ok1) {
							LOGE("Can't copy %s\n", imgpath);
							AWMENUITEM mi[2];
							aw_menuset(mi, 0, "ok", 33);
							char info[256];
							snprintf(info, 256, "Process Failed To Write Kernel, Please Try Again");
							aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
							break;
						}
					}
					pid_t pid;
					pid = fork();
					if(pid == 0) {
						char* noext;
  						noext=strndup(basename(full_fl), strlen(basename(full_fl))-4);
						char* args2[]= {
  								 
								"useless",(isZip) ? "zip" : "img", klocation, full_fl, noext,NULL
							}
							;
						/* left for illustration purposes
			  args2[1] = full_fl;//Zip Location
			  args2[2] = full_fl;//Zip Name
			  args2[3] = klocation;//Flash Location
			  args2[4] = imgpath;//Boot IMG Path
			  args2[5] = basename(imgpath);// Boot IMG Name
			  args2[6] = NULL;*/
						if (execv("/tmp/kl-script", args2)<0) {
							fprintf(apipe(), "Error Flashing Kernel %s\n", strerror(errno));
							_exit(errno);
						}
						_exit(1);
					}
					if (waitpidv2(pid,v) && isZip){
							if (strcmp(acopt_getseltitle(v.hFile, 3),"Yes") == 0){
									fprintf(apipe(), "ui_print\n");
									fprintf(apipe(), "ui_print -- Installing: %s\n",full_fl);
								if (try_update_binary(full_fl,zip,v)){
									AWMENUITEM mi[2];
									aw_menuset(mi, 0, "ok", 33);
									char info[256];
									snprintf(info, 256, "Zip Installed!!");
									aw_multiconfirm_ex(v.hWin, "Complete", info, aui_icons(37), mi, 1, 1);
									fprintf(apipe(), "ui_print Zip Installed Correctly.\n");
									fprintf(apipe(), "ui_print\n");

								} else {
									AWMENUITEM mi[2];
									aw_menuset(mi, 0, "ok", 33);
									char info[256];
									snprintf(info, 256, "Zip Installation Failed.");
									aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
									fprintf(apipe(), "ui_print Zip Installation Failed.\n");
									fprintf(apipe(), "ui_print\n");
								}
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
	if (isZip)
	mzCloseZipArchive(zip);
	//-- Window
	aw_destroy(v.hWin);
}
