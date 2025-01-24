#!/bin/bash

cp ./Builds/LinuxMakefile/build/BL_Denoiser.so ../../../pack/Linux/BL_Denoiser-linux-amd64/vst2
cp -R ./Builds/LinuxMakefile/build/BL_Denoiser.vst3 ../../../pack/Linux/BL_Denoiser-linux-amd64/vst3
cp -R ./Builds/LinuxMakefile/build/BL_Denoiser.lv2 ../../../pack/Linux/BL_Denoiser-linux-amd64/lv2
cp ../../../doc/denoiser/BL_Denoiser_manual.pdf ../../../pack/Linux/BL_Denoiser-linux-amd64
