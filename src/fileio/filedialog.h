#pragma once
#include <filesystem>
#include <fstream>
#include <windows.h>
#include <windef.h>
#include <handleapi.h>
#include <optional>

namespace FileDialog {
	namespace fs = std::filesystem;

	// TODO: specify/restrict file types
	std::optional<fs::path> open_file_dialog(HWND hwnd);
}
