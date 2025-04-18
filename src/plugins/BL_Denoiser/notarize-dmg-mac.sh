#!/bin/bash

codesign --sign "Developer ID Application: Nicolas Dittlo (R6C6L89AV4)" ../../../pack/Mac/BL_Denoiser-v7.0.1.dmg

# see: https://www.technotes.omnis.net/Technical%20Notes/Deployment/macOS%20notarization/2.Submitting%20app%20for%20notarization.html
xcrun notarytool submit "../../../pack/Mac/BL_Denoiser-v7.0.1.dmg" --keychain-profile "NotaryProfile" --wait

xcrun stapler staple "../../../pack/Mac/BL_Denoiser-v7.0.1.dmg"
