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
