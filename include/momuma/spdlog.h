// used to add custom formatters in a central location

#ifndef MONO_MUSIC_MANAGER__SPDLOG_H
#define MONO_MUSIC_MANAGER__SPDLOG_H

#ifdef NDEBUG
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif
#include <spdlog/spdlog.h>


#define DECLARE_FORMATTER_FOR(type) \
template<> \
struct fmt::formatter<type> \
{ \
	fmt::formatter<std::string> formatter; \
	\
	template <typename T_ParseContext> \
	constexpr auto parse(T_ParseContext& ctx) { return formatter.parse(ctx); } \
	\
	template <typename T_FormatContext> \
	constexpr auto format(const type& val, T_FormatContext& ctx) \
	{ \
		return formatter.format(TO_STR_FROM(val), ctx); \
	} \
};


#ifdef _GLIBMM_USTRING_H
	#include <glibmm/ustring.h>
	#define TO_STR_FROM(value) ((value).raw())
	DECLARE_FORMATTER_FOR(Glib::ustring)
	#undef TO_STR_FROM
#endif


#include <filesystem>
namespace fs = std::filesystem;
#define TO_STR_FROM(value) ((value).native())
DECLARE_FORMATTER_FOR(fs::path)
#undef TO_STR_FROM


#include <chrono>
namespace chrono = std::chrono;

#define TO_STR_FROM(value) (std::to_string((value).count()) + " s")
DECLARE_FORMATTER_FOR(chrono::seconds)
#undef TO_STR_FROM

#define TO_STR_FROM(value) (std::to_string((value).count()) + " ms")
DECLARE_FORMATTER_FOR(chrono::milliseconds)
#undef TO_STR_FROM

#define TO_STR_FROM(value) (std::to_string((value).count()) + " μs")
DECLARE_FORMATTER_FOR(chrono::microseconds)
#undef TO_STR_FROM

#define TO_STR_FROM(value) (std::to_string((value).count()) + " ns")
DECLARE_FORMATTER_FOR(chrono::nanoseconds)
#undef TO_STR_FROM

#define TO_STR_FROM(value) (std::to_string((value).count()) + " s")
DECLARE_FORMATTER_FOR(chrono::duration<double>)
#undef TO_STR_FROM


#undef DECLARE_FORMATTER_FOR
#endif /* MONO_MUSIC_MANAGER__SPDLOG_H */
