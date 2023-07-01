#ifndef UTILS__CONSTANTS_H
#define UTILS__CONSTANTS_H

#include <filesystem>


namespace Momuma
{

enum class Platform {
	ALL, LINUX, MAC, WINDOW
};

bool is_filename_forbidden(std::filesystem::path filename);

}

#endif /* UTILS__CONSTANTS_H */
