#ifndef MONO_MUSIC_MANAGER__INTERNAL__CONSTANTS_H
#define MONO_MUSIC_MANAGER__INTERNAL__CONSTANTS_H

#include <filesystem>


namespace Momuma
{

enum class Platform {
	ALL, LINUX, MAC, WINDOW
};

bool is_filename_forbidden(std::filesystem::path filename);

}

#endif /* MONO_MUSIC_MANAGER__INTERNAL__CONSTANTS_H */
