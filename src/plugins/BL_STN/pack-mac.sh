#!/bin/bash

cp -R ./Builds/MacOSX/build/Release/BL_Air.vst ../../../pack/Mac/BL_Air/vst2
cp -R ./Builds/MacOSX/build/Release/BL_Air.vst3 ../../../pack/Mac/BL_Air/vst3
cp -R ./Builds/MacOSX/build/Release/BL_Air.component ../../../pack/Mac/BL_Air/au
cp -R ./Builds/MacOSX/build/Release/BL_Air.aaxplugin ../../../pack/Mac/BL_Air/aax
cp ../../../doc/air/BL_Air_manual.pdf ../../../pack/Mac/BL_Air
cp ./Installer/BL_Air-v7.0.1-installer.pkg ../../../pack/Mac/BL_Air
