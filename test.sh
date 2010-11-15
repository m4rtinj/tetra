#!/bin/bash

cd v3a
bin/Release/v3a > test.txt
cd ..

cd v5
bin/Release/v5 > test.txt
cd ..

vim -d v3a/test.txt v5/test.txt
exit
