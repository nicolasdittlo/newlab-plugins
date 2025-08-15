#!/bin/bash

mv ./Installer/BL_STN-v7.0.4-installer.pkg ./Installer/BL_STN-v7.0.4-installer_unsigned.pkg

productsign --sign "Developer ID Installer: Nicolas Dittlo (R6C6L89AV4)" ./Installer/BL_STN-v7.0.4-installer_unsigned.pkg ./Installer/BL_STN-v7.0.4-installer.pkg

rm ./Installer/BL_STN-v7.0.4-installer_unsigned.pkg
