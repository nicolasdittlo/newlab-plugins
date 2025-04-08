# newlab-plugins

### This is the new BlueLab | Audio Plugins sources repository.

This is a work in progress rework of the BlueLab plugins.
This rework includes:
- port to JUCE
- optimizing code
- small fixes
- rewrite the code more cleanly

The plugins here are for sale on the site www.bluelab-plugins.com
So please do not build them and put the builds online.

### To build the plugins on Linux:

cd src/libs
git clone https://github.com/nicolasdittlo/JUCE.git  
build Projucer  

download fftw-3.3.10.tar.gz and extract it in src/libs  
cd fftw-3.3.10  
mkdir build-linux  
cd build-linux  
cmake .. -DBUILD_STATIC_LIBS=ON -DENABLE_FLOAT=ON -D CMAKE_C_FLAGS="-fPIC"  
make  

cd src/lib  
git clone https://github.com/nicolasdittlo/nanovg.git  

find vstsdk2_4.zip and unzip it in src/libs  

launch Projucer  
open src/plugins/BL_Denoiser/BL_Denoiser.jucer  
save the project to generate the Makefile  

cd src/plugins/BL_Denoiser/Builds/LinuxMakefile  
export CONFIG=Release

make
