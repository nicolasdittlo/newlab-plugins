/* Copyright (C) 2025 Nicolas Dittlo <newlab.plugins@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "BinaryData.h"
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

class ManualPdfViewer
{
public:
    static void openEmbeddedPdf()
    {
        // Create a temporary file
        auto tempFile = juce::File::createTempFile("embedded_pdf.pdf");

        // Write the binary data to the file
        tempFile.replaceWithData(BinaryData::NL_manual_pdf, BinaryData::NL_manual_pdfSize);

        // Open the file using the system's default PDF viewer
        if (!tempFile.startAsProcess())
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Error",
                "Failed to open the PDF.");
        }
    }
};
