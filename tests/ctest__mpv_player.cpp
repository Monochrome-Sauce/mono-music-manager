#include <momuma/spdlog.h>

#include "catch2_main.h"
#include "MpvPlayer.h"


const std::array<fs::path, 2> TEST_MEDIA = {
	fs::path(TESTING_PATH) / "Bamboo Hit.mp3",
	fs::path(TESTING_PATH) / "Hare Hare Yukai.mp3",
};


static inline void check_mpv_error(mpv_error err)
{
	spdlog::error("Mpv error: ({:d}) {:s}", err, mpv_error_string(err));
	REQUIRE(err == MPV_ERROR_SUCCESS);
}

static inline Momuma::MpvPlayer make_player(void)
{
	mpv_error err;
	Momuma::MpvPlayer player(err);
	REQUIRE(err == MPV_ERROR_SUCCESS);
	return player;
}

template<typename TimeMin, typename TimeMax>
[[nodiscard]] static inline
bool test_query_duration(const fs::path &path, TimeMin min, TimeMax max)
{
	chrono::microseconds t = Momuma::MpvPlayer::query_duration(path);
	SPDLOG_INFO("{} | {:s}", chrono::duration_cast<chrono::milliseconds>(t), path);
	return min < t && t <= max;
}

TEST_CASE("Query media duration", "[query, play]")
{
	using fsec = chrono::duration<double>;
	REQUIRE(test_query_duration(TEST_MEDIA[0], fsec(1.8), fsec(1.9)));
	REQUIRE(test_query_duration(TEST_MEDIA[1], fsec(213), fsec(214)));
}

TEST_CASE("Append track and play", "[play, append]")
{
	auto player = make_player();
	check_mpv_error(player.append_media(TEST_MEDIA[0]));
	player.set_play(true);
	sleep(2);
}
