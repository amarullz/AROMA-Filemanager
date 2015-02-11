/*
 * Descriptions:
 * -------------
 * KL Flasher Code
 *
 */
#include "../../../libs/minzip/Zip.h"

#define ASSUMED_UPDATE_BINARY_NAME  "META-INF/com/google/android/update-binary"
#define ASSUMED_UPDATE_SCRIPT_NAME  "META-INF/com/google/android/update-script"


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
bool waitpidv2(int pid,AUI_VARS v) {
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
			return true;
		} else {
			AWMENUITEM mi[2];
			aw_menuset(mi, 0, "ok", 33);
			char info[256];
			snprintf(info, 256, "Process Failed To Execute Script,\nError: %s,\nPlease Try Again.", strerror(WTERMSIG(status)));
			aw_multiconfirm_ex(v.hWin, "Error Has Occured", info, aui_icons(37), mi, 1, 1);
			fprintf(apipe(), "ui_print\n");
			fprintf(apipe(), "ui_print Process Failed To Execute Script,\nui_print Error: %s,\nui_print Please Try Again.\n", strerror(WTERMSIG(status)));
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
	return false;
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

void FormatCache(bool isZip, AUI_VARS v){

	if (strcmp(acopt_getseltitle(v.hFile, (isZip) ? 3 : 2),"Yes") == 0){


		char* args[]= {"/tmp/wipe-cache", NULL};
		pid_t pid = fork();
		if (pid == 0) {
			execvp("/tmp/wipe-cache", args);
			fprintf(apipe(), "E:Can't run /tmp/wipe-cache (%s)\n", strerror(errno));
			_exit(errno);
		}


		int status;
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			AWMENUITEM mi[2];
			aw_menuset(mi, 0, "ok", 33);
			char info[256];
			snprintf(info, 256, "Cache Format Failed.");
			aw_multiconfirm_ex(v.hWin, "Fail", info, aui_icons(37), mi, 1, 1);
			fprintf(apipe(), "ui_print Cache Format Failed.\n");
			fprintf(apipe(), "ui_print\n");
			return;
		} else {
			AWMENUITEM mi[2];
			aw_menuset(mi, 0, "ok", 33);
			char info[256];
			snprintf(info, 256, "Cache Formated.");
			aw_multiconfirm_ex(v.hWin, "Success", info, aui_icons(37), mi, 1, 1);
			fprintf(apipe(), "ui_print Cache Formated.\n");
			fprintf(apipe(), "ui_print\n");
		}
	}

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
            return 0;
        }

        mzCloseZipArchive(zip);
        return 0;
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

char* getrepack(AUI_VARS v, int isZip){
	char* repackv = acopt_getseltitle(v.hFile, (isZip) ? 4 : 3);
	if (strcmp(repackv,"zImage & Ramdisk") == 0){
		return "1";
	} else if (strcmp(repackv,"zImage") == 0){
		return "2";
	} else {
		return "3";
	}
}

char* readfile(char* fileName){
	    int size = 0;
	    FILE *file = fopen(fileName, "r");

	    if(!file) {
	        fputs("File error.\n", stderr);
	        return NULL;
	    }

	    fseek(file, 0, SEEK_END);
	    size = ftell(file);
	    rewind(file);

	    char *result = (char *) malloc(size);
	    if(!result) {
	        fputs("Memory error.\n", stderr);
	        return NULL;
	    }

	    if(fread(result, 1, size, file) != size) {
	        fputs("Read error.\n", stderr);
	        return NULL;
	    }

	    fclose(file);
	    return result;
	}

char* read_file_2(char* filename){

    unsigned long int size = 0;
    FILE *file = fopen(filename, "r");

    if(!file) {
        fputs("File error.\n", stderr);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    char *result = (char *) malloc(size);
    if(!result) {
        fputs("Memory error.\n", stderr);
        return NULL;
    }

    if(fread(result, 1, size, file) != size) {
        fputs("Read error.\n", stderr);
        return NULL;
    }

    fclose(file);
    result[size-1] = '\0';
    return result;
}

void DispKernels(AUI_VARS v){

	static const char *assets[][1] = 
	{
 		{ "/data/media/0/ZeusKL/" },
 		{ "data & sdcard" }
	};
	if (true)
		{
			int i = 0;
			char *kernel = NULL;
			char *ramdisk = NULL;
			if( access( assets[0][i], F_OK ) != -1 ) {
				
				char files1[30];char files2[30];
				strcpy(files1, assets[0][i]);strcat(files1,"kernel");
				strcpy(files2, assets[0][i]);strcat(files2,"ramdisk");
				if( access( files1, F_OK ) != -1 && access( "/data/media/0/ZeusKL/zImage", F_OK ) != -1) {//Kernel Check
					kernel = read_file_2(files1);
				}
				if( access( files2, F_OK ) != -1  && access( "/data/media/0/ZeusKL/initrd.img", F_OK ) != -1) {//Ramdisk Check
					ramdisk = read_file_2(files2);
				}
			}
			if (kernel == NULL || strcmp(kernel, "") == 0) {
					kernel = "NA";
			}
			if (ramdisk == NULL || strcmp(ramdisk, "") == 0) {
					ramdisk = "NA";
			}
			char tmpdata[800];
			acopt_addgroup(v.hFile, "Current Kernel & Ramdisk", "");
			strcpy(tmpdata, "Kernel: ");
			strcat(tmpdata,kernel);
			acopt_addgroup(v.hFile, tmpdata, "");
			char tmpdata2[800];
			strcpy(tmpdata2,"Ramdisk: ");
			strcat(tmpdata2,ramdisk);
			acopt_addgroup(v.hFile, tmpdata2, "");
		}
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
	ZipArchive za;
	ZipArchive* zip;
	if (isZip) {
		//-- IMG To Flash : 1
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
			//-- Flash Module : 2
			acopt_addgroup(v.hFile, "Flash Zip Aswell", "");
			acopt_add(v.hFile, "Yes", "Would Flash Modules If Available",1);
			acopt_add(v.hFile, "No", "",0);
	}

	//-- Format Cache: 2-3
	acopt_addgroup(v.hFile, "Format Cache", "");
	acopt_add(v.hFile, "Yes", "",1);
	acopt_add(v.hFile, "No", "",0);

	//-- Repack: 3-4
	acopt_addgroup(v.hFile, "Update:", "For Expert Use Only!!");
	acopt_add(v.hFile, "zImage & Ramdisk", "",1);
	acopt_add(v.hFile, "zImage", "",0);
	acopt_add(v.hFile, "Ramdisk", "Don't Flash Modules.",0);

	//-- Show Installed Kernels
	DispKernels(v);
	
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
					char* bootimg = "/tmp/boot";

					char* Repack = getrepack(v, isZip);
					unlink(bootimg);
					mountit("/data");
					if (isZip) {
						char* imgpath = acopt_getseltitle(v.hFile, 1);
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
								"/tmp/kl-update-script" ,(isZip) ? "zip" : "img", full_fl, noext, Repack, NULL
							}
							;
						/* left for illustration purposes
						args2[1] = full_fl;//Zip Location
						args2[2] = full_fl;//Zip Name
						args2[3] = klocation;//Flash Location
						args2[4] = imgpath;//Boot IMG Path
						args2[5] = basename(imgpath);// Boot IMG Name
						args2[6] = NULL;*/
						if (execv(args2[0], args2)<0) {
							fprintf(apipe(), "ui_print Error Flashing Kernel %s\n", strerror(errno));
							_exit(errno);
						}
						_exit(1);
					}
					if (waitpidv2(pid,v) && isZip){
							if (strcmp(acopt_getseltitle(v.hFile, 2),"Yes") == 0){
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
						FormatCache(isZip, v);
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
