#!/bin/bash

mv ./Installer/NL-Denoiser-v7.0.0-installer.pkg ./Installer/NL-Denoiser-v7.0.0-installer_unsigned.pkg

productsign --sign "Developer ID Installer: Nicolas Dittlo (R6C6L89AV4)" ./Installer/NL-Denoiser-v7.0.0-installer_unsigned.pkg ./Installer/NL-Denoiser-v7.0.0-installer.pkg

rm ./Installer/NL-Denoiser-v7.0.0-installer_unsigned.pkg
