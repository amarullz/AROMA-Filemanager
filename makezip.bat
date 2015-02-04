@echo off
echo MAKE ZIP AROMA FILEMANAGER
echo ==========================
echo * Copying binary into release
copy out\aromafm assets\META-INF\com\google\android\update-binary
echo.
echo * Building zip
cd assets
..\tools\7z a -mx9 -tzip ..\out/aromafm.zip .
cd ..
