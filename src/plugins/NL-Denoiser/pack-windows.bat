copy .\Installer\NL-Denoiser-*-Installer.exe ..\..\..\pack\Windows\NL-Denoiser
copy ..\..\..\doc\denoiser\NL-Denoiser_manual.pdf ..\..\..\pack\Windows\NL-Denoiser
copy .\Builds\VisualStudio2019\x64\Release\VST\NL-Denoiser.dll ..\..\..\pack\Windows\NL-Denoiser\vst2
xcopy .\Builds\VisualStudio2019\x64\Release\VST3\NL-Denoiser.vst3 ..\..\..\pack\Windows\NL-Denoiser\vst3\NL-Denoiser.vst3\ /E /Y
xcopy .\Builds\VisualStudio2019\x64\Release\AAX\NL-Denoiser.aaxplugin ..\..\..\pack\Windows\NL-Denoiser\aax\NL-Denoiser.aaxplugin\ /E /Y
