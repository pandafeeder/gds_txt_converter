#!/usr/bin/env bash

for file in `ls *.gds`; do
	echo "testing $file"
	../build/gds2txt -g -i $file -o $file.txt
	../build/gds2txt -t -i $file.txt -o $file.gds
	diff $file $file.gds
	if [ $? -ne 0 ]; then
		echo "$file failed"
	else
		echo "$file passed"
	fi
	rm $file.gds $file.txt
done
