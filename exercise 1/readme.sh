#!/bin/bash

make readme
rm -f readme2.txt

for i in {1..100};
do
    ./pthreads 2.71 $i >> readme2.txt
done
