#!/bin/bash
AROMA_LOCAL_PATH=$(readlink -f $1)
AROMA_OUT_PATH=$(readlink -f $2)
AROMA_BINARY=$AROMA_OUT_PATH/aroma_filemanager
AROMA_ZIP_FILE=aromafm_$3.zip

if [ ! -x $AROMA_BINARY ]; then
    echo "0"
    exit
fi

rm -rf "$AROMA_OUT_PATH/zip_src"
rm -f "$AROMA_OUT_PATH/$AROMA_ZIP_FILE"
mkdir -p "$AROMA_OUT_PATH/zip_src"

cp -r $AROMA_LOCAL_PATH/assets/assets $AROMA_OUT_PATH/zip_src
mkdir -p $AROMA_OUT_PATH/zip_src/META-INF/com/google/android
mv $AROMA_BINARY $AROMA_OUT_PATH/zip_src/META-INF/com/google/android/update-binary

cd $AROMA_OUT_PATH/zip_src
zip -r9q $AROMA_OUT_PATH/$AROMA_ZIP_FILE .
echo 1
