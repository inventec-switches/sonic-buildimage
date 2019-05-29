#!/bin/bash
## This script is to apply inventec own patch for SONiC

ROOT=$(pwd)
INV_PATCH_DIR=$ROOT/inv_patch

# Find each submodule need to patch
for entry in $INV_PATCH_DIR/*
do
  #echo "$(basename $entry)"
  SUB_MODULE="$(basename $entry)"

  if [ "$SUB_MODULE" = "README" ]; then
      continue;
  elif [ "$SUB_MODULE" = "sonic-buildimage" ]; then
      echo "=== Apply patch to $SUB_MODULE ==="
      seriesfile="${INV_PATCH_DIR}/${SUB_MODULE}/series"
      exec < ${seriesfile}
      while read line
      do
          content=$(echo $line | awk /#/'{print $1}')
          if [ "$content" = "#" ]; then
              continue
          fi
          git am --whitespace=nowarn ${INV_PATCH_DIR}/${SUB_MODULE}/$line
      done
  else
      cd src/$SUB_MODULE
      CURRENT_HEAD="$(git rev-parse HEAD)"
      TARGET_VERSION="$(awk '/#/{ print $NF}' $INV_PATCH_DIR/$SUB_MODULE/series)"

      if [ "$CURRENT_HEAD" = "$TARGET_VERSION" ]
      then
          echo "=== Apply patch to $SUB_MODULE ==="
            seriesfile="${INV_PATCH_DIR}/${SUB_MODULE}/series"
            exec < ${seriesfile}
            while read line
            do
              content=$(echo $line | awk /#/'{print $1}')
              if [ "$content" = "#" ]; then
                continue
              fi
              git am --whitespace=nowarn ${INV_PATCH_DIR}/${SUB_MODULE}/$line
            done
      else
          echo "error in $SUB_MODULE:Target version $TARGET_VERSION not match current $CURRENT_HEAD"
      fi
  fi
  cd $ROOT
done


