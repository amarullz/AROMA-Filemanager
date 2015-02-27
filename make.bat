@echo off
echo BUILDING AROMA FILEMANAGER
echo ==========================
mkdir obj
mkdir out
cd obj
echo * Compiling... This may take a moments...
echo.

D:\DevelTools\linaro\bin\arm-linux-gnueabihf-gcc.exe ^
  -O3 ^
  -ftree-vectorize -mvectorize-with-neon-quad ^
  -static -fdata-sections -ffunction-sections -Wl,--gc-sections ^
  -fPIC -DPIC  -Wl,-s -Werror ^
  -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_DEBUG ^
  -DFT2_BUILD_LIBRARY=1 -DDARWIN_NO_CARBON ^
  ^
    -D_AROMA_NODEBUG ^
  ^
    -DAROMA_NAME="\"AROMA Filemanager\"" ^
    -DAROMA_VERSION="\"2.00\"" ^
    -DAROMA_BUILD="\"15022810\"" ^
    -DAROMA_BUILD_CN="\"Kacapi\"" ^
  ^
  -mfloat-abi=hard -mfpu=neon ^
  -D__ARM_HAVE_NEON ^
    ^
      *.o ^
    ^
  	  ../src/aroma_openpty.c ^
      ../src/controls/aroma_controls.c ^
      ../src/controls/aroma_control_button.c ^
      ../src/controls/aroma_control_check.c ^
      ../src/controls/aroma_control_checkbox.c ^
      ../src/controls/aroma_control_console.c ^
      ../src/controls/aroma_control_edit.c ^
      ../src/controls/aroma_control_filebox.c ^
      ../src/controls/aroma_control_ime.c ^
      ../src/controls/aroma_control_ime2.c ^
      ../src/controls/aroma_control_imgbutton.c ^
      ../src/controls/aroma_control_label.c ^
      ../src/controls/aroma_control_menubox.c ^
      ../src/controls/aroma_control_optbox.c ^
      ../src/controls/aroma_control_progress.c ^
      ../src/controls/aroma_control_textbox.c ^
      ../src/controls/aroma_control_threads.c ^
      ^
      ../src/libs/aroma_array.c ^
      ../src/libs/aroma_freetype.c ^
      ../src/libs/aroma_graph.c ^
      ../src/libs/aroma_input.c ^
      ../src/libs/aroma_languages.c ^
      ../src/libs/aroma_libs.c ^
      ../src/libs/aroma_memory.c ^
      ../src/libs/aroma_png.c ^
      ../src/libs/aroma_zip.c ^
      ^
      ../src/main/aroma.c ^
      ../src/main/aroma_ui.c ^
  	^
  -I../include ^
  -I../src ^
  -o ../out/aromafm ^
  -lm -lpthread -lrt
echo.
cd ..

echo =================================
echo Ctrl+C Break, Any Key - Build ZIP
echo.
pause

makezip.bat

echo =============================
echo FINISHED
echo.

pause
