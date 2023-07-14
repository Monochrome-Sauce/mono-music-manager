#ifndef MONO_MUSIC_MANAGER__SIGC
#define MONO_MUSIC_MANAGER__SIGC

#include <sigc++/sigc++.h>

namespace sigc
{
	constexpr bool PROPAGATE = false;
	constexpr bool BEFORE = false;
	constexpr bool AFTER = !BEFORE;
}

#endif /* MONO_MUSIC_MANAGER__SIGC */
