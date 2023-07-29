#include <momuma/spdlog.h>

#include "catch2_main.h"
#include "Database-Sqlite3.h"


[[nodiscard]] static inline
Momuma::Database::Sqlite3 make_database(void)
{
	spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
	Momuma::Database::Sqlite3 db(TESTING_PATH, Momuma::Database::StorageType::DISK);
	
	REQUIRE(static_cast<bool>(db));
	REQUIRE(db.get_database_location() == TESTING_PATH);
	return db;
}

TEST_CASE("fill database")
{
	auto db = make_database();
	
	REQUIRE(db.create_playlist("bob"));
	REQUIRE(db.create_playlist("bob"));
	REQUIRE(db.create_playlist("שלום עולם"));
	REQUIRE(db.create_playlist("мусика"));
	
	std::vector<std::string> list;
	const int itemCount = db.get_playlists(
		[&list](std::string playlist) -> Momuma::Database::IterFlag
		{
			list.push_back(std::move(playlist));
			return Momuma::Database::IterFlag::NEXT;
		}
	);
	REQUIRE(itemCount == 3);
	REQUIRE(itemCount == std::ssize(list));
}
