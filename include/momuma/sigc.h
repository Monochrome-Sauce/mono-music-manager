#ifndef MONO_MUSIC_MANAGER__LIBS__CALLBACK
#define MONO_MUSIC_MANAGER__LIBS__CALLBACK

#include <sigc++/signal.h>

namespace sigc
{
	constexpr bool PROPAGATE = false;
	constexpr bool BEFORE = false;
	constexpr bool AFTER = !BEFORE;
}

#endif /* MONO_MUSIC_MANAGER__LIBS__CALLBACK */
