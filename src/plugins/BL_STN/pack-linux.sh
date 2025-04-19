#!/bin/bash

cp ./Builds/LinuxMakefile/build/BL_STN.so ../../../pack/Linux/BL_STN-linux-amd64/vst2
cp -R ./Builds/LinuxMakefile/build/BL_STN.vst3 ../../../pack/Linux/BL_STN-linux-amd64/vst3
cp -R ./Builds/LinuxMakefile/build/BL_STN.lv2 ../../../pack/Linux/BL_STN-linux-amd64/lv2
cp ../../../doc/air/BL_STN_manual.pdf ../../../pack/Linux/BL_STN-linux-amd64
