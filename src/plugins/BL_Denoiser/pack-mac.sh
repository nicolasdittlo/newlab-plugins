#!/bin/bash

cp -R ./Builds/MacOSX/build/Release/BL_Denoiser.vst ../../../pack/Mac/BL_Denoiser/vst2
cp -R ./Builds/MacOSX/build/Release/BL_Denoiser.vst3 ../../../pack/Mac/BL_Denoiser/vst3
cp -R ./Builds/MacOSX/build/Release/BL_Denoiser.component ../../../pack/Mac/BL_Denoiser/au
cp -R ./Builds/MacOSX/build/Release/BL_Denoiser.aaxplugin ../../../pack/Mac/BL_Denoiser/aax
cp ../../../doc/denoiser/BL_Denoiser_manual.pdf ../../../pack/Mac/BL_Denoiser
cp ./Installer/BL_Denoiser-v7.0.0-installer.pkg ../../../pack/Mac/BL_Denoiser
