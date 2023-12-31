#include "filedialog.h"

namespace FileDialog {
	// Opens the native file explorer interface
	// returns a std::filesystem::path pointing to the selected file
	std::optional<fs::path> open_file_dialog(HWND hwnd) {

		OPENFILENAME ofn;       // common dialog box structure
		char szFile[260];       // buffer for file name		

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
		// use the contents of szFile to initialize itself.
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		// Display the Open dialog box. 
		if (GetOpenFileName(&ofn) == TRUE) {
			return std::filesystem::path(ofn.lpstrFile);
		}

		return std::nullopt;
	}
}