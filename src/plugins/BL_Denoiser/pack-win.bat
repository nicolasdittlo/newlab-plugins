copy .\Installer\BL_Denoiser-*-Installer.exe ..\..\..\pack\Windows\BL_Denoiser
copy ..\..\..\doc\denoiser\BL_Denoiser_manual.pdf ..\..\..\pack\Windows\BL_Denoiser
copy .\Builds\VisualStudio2019\x64\Release\VST\BL_Denoiser.dll ..\..\..\pack\Windows\BL_Denoiser\vst2
xcopy .\Builds\VisualStudio2019\x64\Release\VST3\BL_Denoiser.vst3 ..\..\..\pack\Windows\BL_Denoiser\vst3\BL_Denoiser.vst3\ /E /Y
xcopy .\Builds\VisualStudio2019\x64\Release\AAX\BL_Denoiser.aaxplugin ..\..\..\pack\Windows\BL_Denoiser\aax\BL_Denoiser.aaxplugin\ /E /Y
