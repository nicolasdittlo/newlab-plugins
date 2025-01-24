#!/bin/bash

cp ./Builds/LinuxMakefile/build/BL_Air.so ../../../pack/Linux/BL_Air-linux-amd64/vst2
cp -R ./Builds/LinuxMakefile/build/BL_Air.vst3 ../../../pack/Linux/BL_Air-linux-amd64/vst3
cp -R ./Builds/LinuxMakefile/build/BL_Air.lv2 ../../../pack/Linux/BL_Air-linux-amd64/lv2
cp ../../../doc/air/BL_Air_manual.pdf ../../../pack/Linux/BL_Air-linux-amd64
