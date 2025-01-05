#!/bin/bash

cp ./Builds/LinuxMakefile/build/NL-Denoiser.so ../../../pack/Linux/NL-Denoiser-linux-amd64/vst2
cp -R ./Builds/LinuxMakefile/build/NL-Denoiser.vst3 ../../../pack/Linux/NL-Denoiser-linux-amd64/vst3
cp -R ./Builds/LinuxMakefile/build/NL-Denoiser.lv2 ../../../pack/Linux/NL-Denoiser-linux-amd64/lv2
cp ../../../doc/denoiser/NL-Denoiser_manual.pdf ../../../pack/Linux/NL-Denoiser-linux-amd64
