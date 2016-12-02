#!/bin/bash
AROMA_LOCAL_PATH=$(readlink -f $1)
AROMA_OUT_PATH=$(readlink -f $2)
AROMA_BINARY=$AROMA_OUT_PATH/aroma_filemanager
AROMA_ZIP_FILE=aromafm_$3.zip

echo
echo Making Aroma Installer Zip...

if [ ! -x $AROMA_BINARY ]; then
    echo File $AROMA_OUT_PATH/aroma_filemanager does not exist
    exit
fi

rm -rf "$AROMA_OUT_PATH/zip_src"
rm -f "$AROMA_OUT_PATH/$AROMA_ZIP_FILE"
mkdir -p "$AROMA_OUT_PATH/zip_src"

cp -r $AROMA_LOCAL_PATH/assets/assets $AROMA_OUT_PATH/zip_src
mkdir -p $AROMA_OUT_PATH/zip_src/META-INF/com/google/android
mv $AROMA_BINARY $AROMA_OUT_PATH/zip_src/META-INF/com/google/android/update-binary

## remove old zip
[ -f $AROMA_OUT_PATH/$AROMA_ZIP_FILE ] && rm $AROMA_OUT_PATH/$AROMA_ZIP_FILE

cd $AROMA_OUT_PATH/zip_src
zip -r9q $AROMA_OUT_PATH/$AROMA_ZIP_FILE .


echo "Install ----> $AROMA_OUT_PATH/$AROMA_ZIP_FILE"

echo Complete
