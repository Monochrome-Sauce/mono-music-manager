#include <momuma/spdlog.h>

#include "catch2_main.h"
#include "Database-Sqlite3.h"


namespace Db = Momuma::Database;


[[nodiscard]] static inline
Db::Sqlite3 make_database(void)
{
	Db::Sqlite3 db(TESTING_PATH, Db::Sqlite3::StorageType::MEMORY);
	REQUIRE(static_cast<bool>(db));
	return db;
}

TEST_CASE("create database", "[]")
{
	auto db = make_database();
	REQUIRE(db.get_database_location() == TESTING_PATH);
}
