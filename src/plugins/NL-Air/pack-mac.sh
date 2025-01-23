#!/bin/bash

cp -R ./Builds/MacOSX/build/Release/NL-Air.vst ../../../pack/Mac/NL-Air/vst2
cp -R ./Builds/MacOSX/build/Release/NL-Air.vst3 ../../../pack/Mac/NL-Air/vst3
cp -R ./Builds/MacOSX/build/Release/NL-Air.component ../../../pack/Mac/NL-Air/au
cp -R ./Builds/MacOSX/build/Release/NL-Air.aaxplugin ../../../pack/Mac/NL-Air/aax
cp ../../../doc/air/NL-Air_manual.pdf ../../../pack/Mac/NL-Air
cp ./Installer/NL-Air-v7.0.0-installer.pkg ../../../pack/Mac/NL-Air
