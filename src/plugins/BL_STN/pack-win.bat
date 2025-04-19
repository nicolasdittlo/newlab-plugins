copy .\Installer\BL_Air-*-Installer.exe ..\..\..\pack\Windows\BL_Air
copy ..\..\..\doc\air\BL_Air_manual.pdf ..\..\..\pack\Windows\BL_Air
copy .\Builds\VisualStudio2019\x64\Release\VST\BL_Air.dll ..\..\..\pack\Windows\BL_Air\vst2
xcopy .\Builds\VisualStudio2019\x64\Release\VST3\BL_Air.vst3 ..\..\..\pack\Windows\BL_Air\vst3\BL_Air.vst3\ /E /Y
xcopy .\Builds\VisualStudio2019\x64\Release\AAX\BL_Air.aaxplugin ..\..\..\pack\Windows\BL_Air\aax\BL_Air.aaxplugin\ /E /Y
