#!/bin/bash

cp -R ./Builds/MacOSX/build/Release/BL_STN.vst ../../../pack/Mac/BL_STN/vst2
cp -R ./Builds/MacOSX/build/Release/BL_STN.vst3 ../../../pack/Mac/BL_STN/vst3
cp -R ./Builds/MacOSX/build/Release/BL_STN.component ../../../pack/Mac/BL_STN/au
cp -R ./Builds/MacOSX/build/Release/BL_STN.aaxplugin ../../../pack/Mac/BL_STN/aax
cp ../../../doc/STN/BL_STN_manual.pdf ../../../pack/Mac/BL_STN
cp ./Installer/BL_STN-v7.0.4-installer.pkg ../../../pack/Mac/BL_STN
