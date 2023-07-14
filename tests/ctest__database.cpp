#include <momuma/spdlog.h>

#include "catch2_main.h"
#include "Database-Sqlite3.h"


namespace Db = Momuma::Database;


[[nodiscard]] static inline
Db::Sqlite3 make_database(void)
{
	spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
	Db::Sqlite3 db(TESTING_PATH, Db::Sqlite3::StorageType::DISK);
	
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
		[&list](std::string playlist) -> Momuma::IDatabase::IterFlag
		{
			list.push_back(std::move(playlist));
			return Momuma::IDatabase::IterFlag::NEXT;
		}
	);
	REQUIRE(itemCount == 3);
	REQUIRE(itemCount == std::ssize(list));
}
