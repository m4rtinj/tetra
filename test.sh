#!/bin/bash

cd v4
bin/Release/v4 > test.txt
cd ..

cd v5
bin/Release/v5 > test.txt
cd ..

gvim -d v4/test.txt v5/test.txt
exit
