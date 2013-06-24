@echo off
echo Deploying...
REM
REM "D:\DevelTools\sdk\" change to android sdk path
REM

echo Initializing Device
D:\DevelTools\sdk\platform-tools\adb shell mount -a
D:\DevelTools\sdk\platform-tools\adb shell mkdir -p /sdcard/0/
echo Copiying zip file
D:\DevelTools\sdk\platform-tools\adb push out/aromafm.zip /sdcard/0/aromafm.zip
echo Copiying Binary File
D:\DevelTools\sdk\platform-tools\adb push out/aroma_filemanager /tmp/update-binary
echo CHMOD
D:\DevelTools\sdk\platform-tools\adb shell chmod 777 /tmp/update-binary

echo STARTING
echo --------------------------------------------------------
echo.
D:\DevelTools\sdk\platform-tools\adb shell /tmp/update-binary 1 0 /sdcard/0/aromafm.zip
echo.
echo --------------------------------------------------------
echo CTRL-C = Save, Enter = Clean
pause
D:\DevelTools\sdk\platform-tools\adb shell rm /tmp/update-binary