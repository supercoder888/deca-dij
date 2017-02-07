#!/bin/bash

notlost=0
TOTALFILES=0
DECAMIN=1

echo "DECAMIN|FILES|REAL|USER|SYS|TotalBlocks|BlocksProcessed|BlocksSVM|BlocksSig|GIVEUP|FP" | tee -a $2

while [ $notlost -eq 0 ]; do
	/bin/echo 3 > /proc/sys/vm/drop_caches
	/usr/bin/time -f "%E|%U|%S" ../deca -o deca/ -m ../../jpeg.model -v --deca --min $DECAMIN $1 > deca.out 2> time.out
	TIME=$(cat time.out)
	NOWFILES=$(ls deca/ | wc -l)
	if [ $TOTALFILES -eq 0 ]; then
		TOTALFILES=$NOWFILES
		#echo "Totalfiles set to $TOTALFILES"
	fi
	#if [ $TOTALFILES -gt $NOWFILES ]; then
		#echo "The number of files has changed at $NOWFILES"
		#notlost=1
	#fi
	if [ $NOWFILES -eq 0 ]; then
		notlost=1
	fi
	#echo "Current number of files: $NOWFILES"
	#echo "Current min: $DECAMIN"
	TOTALBLOCKS=$(cat deca.out | tail -n 7 | grep "Total blocks" | awk -F"    " '{print $2}')
	BLOCKSPROCESSED=$(cat deca.out | tail -n 6 | grep "Blocks processed" | awk -F"             " '{print $2}')
	BLOCKSSVM=$(cat deca.out | tail -n 6 | grep "SVM" | awk -F"       " '{print $2}')
	BLOCKSSIG=$(cat deca.out | tail -n 6 | grep "signature" | awk -F"  " '{print $3}')
	GIVEUP=$(cat deca.out | tail -n 6 | grep "Give-ups" | awk -F"  " '{print $3}')
	FP=$(cat deca.out | tail -n 6 | grep "False-positive" | awk -F"  " '{print $3}')
	echo "$DECAMIN|$NOWFILES|$TIME|$TOTALBLOCKS|$BLOCKSPROCESSED|$BLOCKSSVM|$BLOCKSSIG|$GIVEUP|$FP" | tee -a $2
	rm -r deca/*
	DECAMIN=$(($DECAMIN + 10))
done
