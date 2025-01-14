#!/bin/bash

cp ./Builds/LinuxMakefile/build/NL-Air.so ../../../pack/Linux/NL-Air-linux-amd64/vst2
cp -R ./Builds/LinuxMakefile/build/NL-Air.vst3 ../../../pack/Linux/NL-Air-linux-amd64/vst3
cp -R ./Builds/LinuxMakefile/build/NL-Air.lv2 ../../../pack/Linux/NL-Air-linux-amd64/lv2
cp ../../../doc/denoiser/NL-Air_manual.pdf ../../../pack/Linux/NL-Air-linux-amd64
