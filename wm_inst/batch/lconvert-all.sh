#!/bin/bash

for i in `ls *.xlf`
do
    lconvert -i ${i} -o "${i%.xlf}.ts"
done
