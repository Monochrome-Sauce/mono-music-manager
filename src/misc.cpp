#include <set>
#include <vector>

#include "misc.h"


namespace Momuma
{

// https://stackoverflow.com/questions/1976007/what-characters-are-forbidden-in-windows-and-linux-directory-names
// and
// https://stackoverflow.com/a/31976060

bool is_filename_forbidden(std::filesystem::path filename)
{
	// all of this just to (potentially) support Windows portability...
	
	static const std::set<std::filesystem::path> FORBIDDEN_NAMES = {
		"", ".", "..",
		
		// Windows (case insensitive, can have any file extension)
		"CON", "PRN", "AUX", "NUL", "CONIN$", "CONOUT$",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
		
		// Mac (none)
		// Linux (none)
	};
	constexpr std::u8string_view FORBIDDEN_CHARS = { u8"<>:\"/\\|?*" };
	
	if (FORBIDDEN_NAMES.contains(filename.stem())) {
		return true;
	}
	
	const std::u8string u8str = filename.u8string();
	if (u8str.ends_with(' ') || u8str.ends_with('.')) {
		return true;
	}
	
	for (const char8_t chr : u8str) {
		if (chr <= 31) {
			return true;
		}
		if (FORBIDDEN_CHARS.find(chr) != FORBIDDEN_CHARS.npos) {
			return true;
		}
	}
	
	return false;
}

}
