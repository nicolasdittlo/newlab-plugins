#!/bin/bash

mv ./Installer/BL_Air-v7.0.0-installer.pkg ./Installer/BL_Air-v7.0.0-installer_unsigned.pkg

productsign --sign "Developer ID Installer: Nicolas Dittlo (R6C6L89AV4)" ./Installer/BL_Air-v7.0.0-installer_unsigned.pkg ./Installer/BL_Air-v7.0.0-installer.pkg

rm ./Installer/BL_Air-v7.0.0-installer_unsigned.pkg
