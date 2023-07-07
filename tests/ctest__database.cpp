#define CATCH_CONFIG_FAST_COMPILE
#include <catch2/catch.hpp>
#include <momuma/spdlog.h>

#include "Database-Sqlite3.h"


const std::array<fs::path, 2> TEST_MEDIA = {
	fs::path(TESTING_PATH) / "Bamboo Hit.mp3",
	fs::path(TESTING_PATH) / "Hare Hare Yukai.mp3",
};

static inline Momuma::Database::Sqlite3 make_database(void)
{
	return Momuma::Database::Sqlite3("");
}


TEST_CASE("db test", "[query]")
{
	spdlog::critical("Hello world");
	auto db = make_database();
	REQUIRE(db.get_database_location().empty());
}
