#!/bin/bash

cd v3
bin/Release/v3 ../data/fuvoka_640000_mod.bdf > ../test.txt
cd ..

cd v4
bin/Release/v4 ../data/fuvoka_640000_mod.bdf >> ../test.txt
cd ..

cd v5
bin/Release/v5 ../data/fuvoka_640000_mod.bdf >> ../test.txt
cd ..

cd v3
bin/Release/v3 ../data/szivocso_vol_tetra_hm.nas >> ../test.txt
cd ..

cd v4
bin/Release/v4 ../data/szivocso_vol_tetra_hm.nas >> ../test.txt
cd ..

cd v5
bin/Release/v5 ../data/szivocso_vol_tetra_hm.nas >> ../test.txt
cd ..

exit
