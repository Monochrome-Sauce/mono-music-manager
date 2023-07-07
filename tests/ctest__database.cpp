#include <momuma/spdlog.h>

#include "catch2_main.h"
#include "Database-Sqlite3.h"


namespace Db = Momuma::Database;


[[nodiscard]] static inline
Db::Sqlite3 make_database(void)
{
	return Db::Sqlite3(TESTING_PATH, Db::Sqlite3::StorageType::MEMORY);
}

TEST_CASE("db test", "[query]")
{
	spdlog::critical("Hello world");
	auto db = make_database();
	REQUIRE(db.get_database_location().empty());
}
