#!/bin/bash

codesign --sign "Developer ID Application: Nicolas Dittlo (R6C6L89AV4)" ../../../pack/Mac/BL_Air-v7.0.2.dmg

# see: https://www.technotes.omnis.net/Technical%20Notes/Deployment/macOS%20notarization/2.Submitting%20app%20for%20notarization.html
xcrun notarytool submit "../../../pack/Mac/BL_Air-v7.0.2.dmg" --keychain-profile "NotaryProfile" --wait

xcrun stapler staple "../../../pack/Mac/BL_Air-v7.0.2.dmg"

# to see logs
# xcrun notarytool log 2667f8c3-dfee-4357-9703-dfb5f7648b50 --keychain-profile "NotaryProfile"
