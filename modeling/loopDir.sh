#!/bin/bash
# Just a wrapper for getFileFeatures.py to be able to process
# a directory. For individual files use getFileFeatures.py directly

if [ ! $2 ]; then
rm jpg.feat 2> /dev/null
rm nonJpg.feat 2> /dev/null
# loop a directory to get jpg and non-jpg features
find $1 | while read f; do
        if [[ $(file -i "$f") == *jpeg* ]]; then
		python getFileFeatures.py 1 "$f" >> jpg.feat
	else
		python getFileFeatures.py -1 "$f" >> nonJpg.feat
	fi
  done

fi

liblinear-predict jpg.feat ../jpeg.model jpg.predict
liblinear-predict nonJpg.feat ../jpeg.model nonJpg.predict
