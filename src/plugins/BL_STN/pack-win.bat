copy .\Installer\BL_STN-*-Installer.exe ..\..\..\pack\Windows\BL_STN
copy ..\..\..\doc\air\BL_STN_manual.pdf ..\..\..\pack\Windows\BL_STN
copy .\Builds\VisualStudio2019\x64\Release\VST\BL_STN.dll ..\..\..\pack\Windows\BL_STN\vst2
xcopy .\Builds\VisualStudio2019\x64\Release\VST3\BL_STN.vst3 ..\..\..\pack\Windows\BL_STN\vst3\BL_STN.vst3\ /E /Y
xcopy .\Builds\VisualStudio2019\x64\Release\AAX\BL_STN.aaxplugin ..\..\..\pack\Windows\BL_STN\aax\BL_STN.aaxplugin\ /E /Y
