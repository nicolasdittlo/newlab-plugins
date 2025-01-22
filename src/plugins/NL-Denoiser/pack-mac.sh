#!/bin/bash

cp -R ./Builds/MacOSX/build/Release/NL-Denoiser.vst ../../../pack/Mac/NL-Denoiser/vst2
cp -R ./Builds/MacOSX/build/Release/NL-Denoiser.vst3 ../../../pack/Mac/NL-Denoiser/vst3
cp -R ./Builds/MacOSX/build/Release/NL-Denoiser.component ../../../pack/Mac/NL-Denoiser/au
cp -R ./Builds/MacOSX/build/Release/NL-Denoiser.aaxplugin ../../../pack/Mac/NL-Denoiser/aax
cp ../../../doc/denoiser/NL-Denoiser_manual.pdf ../../../pack/Mac/NL-Denoiser
cp ./Installer/NL-Denoiser-v7.0.0-installer.pkg ../../../pack/Mac/NL-Denoiser
