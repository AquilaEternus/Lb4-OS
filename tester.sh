#!/bin/bash
#Lab 4 Test Script
#By: Jose Hernandez
#This is a simple bash script that writes out the contents of the current
#directory to file and then reads them to write them in the terminal.

EXISTING_OUTFILE="testdata"
if [-f $EXISTING_OUTFILE] ; then
	rm $EXISTING_OUTFILE
fi

OUTFILE="testdata"
for i in *
do
	echo "$i" >> $OUTFILE
done

INPUT="testdata"
while IFS= read -r line
do
	echo "$line" 
done < "$INPUT"
