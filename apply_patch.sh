#!/bin/bash
## This script is to apply inventec own patch for SONiC

ROOT=$(pwd)
INV_PATCH_DIR=$ROOT/inv_patch

# Find each submodule need to patch
for entry in $INV_PATCH_DIR/*
do
  echo "$(basename $entry)"
  SUB_MODULE="$(basename $entry)"

  cd src/$SUB_MODULE

  CURRENT_HEAD="$(git rev-parse HEAD)"

  TARGET_VERSION="$(awk '/#/{ print $NF}' $INV_PATCH_DIR/$SUB_MODULE/series)"

  if [ "$CURRENT_HEAD" = "$TARGET_VERSION" ]
  then
        seriesfile="${INV_PATCH_DIR}/${SUB_MODULE}/series"
        exec < ${seriesfile}
        while read line
        do
          content=$(echo $line | awk /#/'{print $1}')
          if [ "$content" = "#" ]; then
            continue
          fi
          git am ${INV_PATCH_DIR}/${SUB_MODULE}/$line
        done
  else
      echo "error in $SUB_MODULE:Target version $TARGET_VERSION not match current $CURRENT_HEAD"
  fi
  cd $ROOT
done


