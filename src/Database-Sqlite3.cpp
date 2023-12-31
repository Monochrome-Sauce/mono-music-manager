#include <fmt/compile.h>

#include "Database-Sqlite3.h"
#include "momuma/spdlog.h"


#define ASSERT_SQLITE_COLUMN(sqliteStmt, colIndex, colType, colName) \
do { \
	assert(sqliteStmt.column_count() >= colIndex); \
	assert(sqliteStmt.column_type(colIndex) == colType); \
	assert(!strcmp(sqliteStmt.column_name(colIndex), colName)); \
} while (0)


namespace Momuma::Database
{

// Thin and exception-safe wrapper over a `sqlite3_stmt*`.
struct SqliteStmt
{
	sqlite3_stmt *_p;
	
	SqliteStmt(SqliteStmt &&other) : _p { other._p }
	{
		assert(this != &other);
		other._p = nullptr;
	}
	SqliteStmt(const SqliteStmt &other) = delete;
	
	SqliteStmt(void) : _p { nullptr } {}
	~SqliteStmt(void) { (void)this->finalize(); }
	
	[[nodiscard]] inline bool operator==(const SqliteStmt &other) const { return _p == other._p; }
	
	[[nodiscard]] inline int bind_text(int iParam, const std::string &value)
	{
		return sqlite3_bind_text(_p,
			iParam, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT
		);
	}
	
	[[nodiscard]] inline int column_bytes(int iCol) { return sqlite3_column_bytes(_p, iCol); }
	[[nodiscard]] inline int column_count(void) { return sqlite3_column_count(_p); }
	[[nodiscard]] inline const char* column_text(int iCol) { return reinterpret_cast<const char*>(sqlite3_column_text(_p, iCol)); }
	[[nodiscard]] inline const char* column_name(int iCol) { return sqlite3_column_name(_p, iCol); }
	[[nodiscard]] inline int column_type(int iCol) { return sqlite3_column_type(_p, iCol); }
	
	[[nodiscard]] inline int finalize(void) { return sqlite3_finalize(_p); }
	[[nodiscard]] inline int prepare(sqlite3 *const db, const std::string &query)
	{
		assert(_p == nullptr);
		const auto len = static_cast<int>(std::ssize(query));
		return sqlite3_prepare_v2(db, query.c_str(), len, &_p, nullptr);
	}
	[[nodiscard]] inline int reset(void) { return sqlite3_reset(_p); }
	[[nodiscard]] inline int step(void) { return sqlite3_step(_p); }
};



// Contains all table names, with the similarly named namespace holding the columns.
namespace Tab
{
	constexpr const char FILES[] = "files";
	namespace Files
	{
		constexpr const char PLAYLIST_ID[] = "playlist_id"; // not null, ref: > playlists.id
		constexpr const char INDEX[] = "index"; // not null
		constexpr const char NAME[] = "name"; // not null
	}
	
	constexpr const char PLAYLISTS[] = "playlists";
	namespace Playlists
	{
		constexpr const char ID[] = "id"; // pk, increment, not null
		constexpr const char NAME[] = "name"; // unique, not null
	}
	
	constexpr const char DB_VERSION[] = "database_version";
	namespace DbVersion
	{
		constexpr char VERSION[] = "version";
		constexpr char NOTES[] = "notes";
	}
}

namespace Directory
{
	constexpr const char DB_FILE[] = "sqlite3.db"; // the database file name
	
	// directory containing the directories of media files
	constexpr const char PLAYLISTS[] = "Playlists";
}


[[nodiscard]] static
int create_and_open_database(
	sqlite3 *&handleRef, const fs::path &fullFolderPath, const StorageType storage
) {
	assert(sqlite3_libversion_number() == SQLITE_VERSION_NUMBER);
	assert(strncmp(sqlite3_sourceid(), SQLITE_SOURCE_ID, std::size(SQLITE_SOURCE_ID)) == 0);
	assert(strcmp(sqlite3_libversion(), SQLITE_VERSION) == 0);
	
	SPDLOG_DEBUG("Creating root directory: '{}'", fullFolderPath);
	fs::create_directory(fullFolderPath);
	fs::create_directory(fullFolderPath / Directory::PLAYLISTS);
	
	int sqliteOpenBits = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOFOLLOW;
	if (storage == StorageType::MEMORY) {
		SPDLOG_DEBUG("Creating in-memory {:s}", Directory::DB_FILE);
		sqliteOpenBits |= SQLITE_OPEN_MEMORY;
	}
	
	return sqlite3_open_v2(
		(fullFolderPath / Directory::DB_FILE).c_str(),
		&handleRef, sqliteOpenBits, nullptr
	);
}

/* #Creates the SQL database tables
! @return: error code returned by the first failed sqlite3 query.
*/
[[nodiscard]] static
int create_database_tables(sqlite3 &db)
{
	std::string query;
	query += fmt::format(
		R"(CREATE TABLE IF NOT EXISTS [{}] (
			[{}] INTEGER NOT NULL REFERENCES [{}({})],
			[{}] INTEGER NOT NULL, [{}] TEXT NOT NULL
		) STRICT;)",
		Tab::FILES,
		Tab::Files::PLAYLIST_ID, Tab::PLAYLISTS, Tab::Playlists::ID,
		Tab::Files::INDEX, Tab::Files::NAME
	);
	query += fmt::format(
		R"(CREATE TABLE IF NOT EXISTS [{}] (
			[{}] INTEGER NOT NULL, [{}] TEXT NOT NULL UNIQUE,
			PRIMARY KEY([{}] AUTOINCREMENT)
		) STRICT;)",
		Tab::PLAYLISTS,
		Tab::Playlists::ID, Tab::Playlists::NAME,
		Tab::Playlists::ID
	);
	query += fmt::format(
		R"(CREATE TABLE IF NOT EXISTS [{}] (
			[{}] INTEGER NOT NULL, [{}] TEXT,
			PRIMARY KEY([{}])
		) STRICT;)",
		Tab::DB_VERSION,
		Tab::DbVersion::VERSION, Tab::DbVersion::NOTES,
		Tab::DbVersion::VERSION
	);
	
	const int code = sqlite3_exec(&db, query.c_str(), nullptr, nullptr, nullptr);
	return code;
}

Sqlite3::Sqlite3(const fs::path &fullFolderPath, const StorageType storage) :
	_handle { nullptr },
	m_path { fullFolderPath }
{
	int err = create_and_open_database(_handle, fullFolderPath, storage);
	if (err != SQLITE_OK) {
		SPDLOG_ERROR("Failed to create database ({:d}): {:s}", err, sqlite3_errstr(err));
		this->close_handle();
		return;
	}
	
	err = create_database_tables(*_handle);
	if (err != SQLITE_OK) {
		SPDLOG_ERROR("Failed to initiate database ({:d}): {:s}", err, sqlite3_errstr(err));
		this->close_handle();
		return;
	}
}

Sqlite3::Sqlite3(Sqlite3 &&other) :
	_handle { other._handle },
	m_path { std::move(other.m_path) }
{
	assert(this != &other);
	other._handle = nullptr;
}

Sqlite3::~Sqlite3(void)
{
	this->close_handle();
};

Sqlite3::operator bool(void) const
{
	return _handle != nullptr;
}

fs::path Sqlite3::get_database_location(void) noexcept
{
	return m_path;
}

int Sqlite3::get_playlists(sigc::slot<IterFlag(std::string)> callback)
{
	static const std::string query = fmt::format(
		"SELECT [{}] FROM [{}];",
		Tab::Playlists::NAME, Tab::PLAYLISTS
	);
	
	SqliteStmt stmt;
	if (const int rc = stmt.prepare(_handle, query); rc != SQLITE_OK) {
		SPDLOG_ERROR("sqlite3_prepare() failed ({:d}): {:s}", rc, sqlite3_errstr(rc));
		return -1;
	}
	
	int rcode = 0, iterations = 0;
	while ((rcode = stmt.step()) == SQLITE_ROW) {
		constexpr int COLUMN = 0;
		ASSERT_SQLITE_COLUMN(stmt, COLUMN, SQLITE3_TEXT, Tab::Playlists::NAME);
		
		const char *playlist = stmt.column_text(COLUMN);
		if (playlist == nullptr && sqlite3_errcode(_handle) == SQLITE_NOMEM) {
			throw std::bad_alloc();
		}
		const int len = stmt.column_bytes(COLUMN);
		
		++iterations;
		const IterFlag res = callback(std::string(&playlist[0], &playlist[len]));
		if (res == IterFlag::STOP) { return iterations; }
	}
	
	if (rcode != SQLITE_DONE) {
		SPDLOG_ERROR("sqlite3_step() failed ({:d}): {:s}", rcode, sqlite3_errstr(rcode));
		return -1;
	}
	return iterations;
}

bool Sqlite3::create_playlist(const std::string &playlist)
{
	constexpr int BOUND_PARAM = 1;
	static const std::string query = fmt::format(
		R"(INSERT INTO [{}] ([{}]) VALUES(?{:d});)",
		Tab::PLAYLISTS, Tab::Playlists::NAME, BOUND_PARAM
	);
	
	SqliteStmt stmt;
	if (const int rc = stmt.prepare(_handle, query); rc != SQLITE_OK) {
		SPDLOG_ERROR("sqlite3_prepare() failed ({:d}): {:s}", rc, sqlite3_errstr(rc));
		return false;
	}
	if (stmt.bind_text(BOUND_PARAM, playlist) == SQLITE_NOMEM) {
		throw std::bad_alloc();
	}
	
	const int rc = stmt.step();
	SPDLOG_DEBUG("Adding playlist '{}' - ({:d}): {:s}", playlist, rc, sqlite3_errstr(rc));
	switch (rc)
	{
	case SQLITE_CONSTRAINT: // happens when the playlist already exists
	case SQLITE_DONE:
		return true;
	default:
		return false;
	}
}

bool Sqlite3::remove_playlist(const std::string &/*playlist*/)
{
	assert(!bool("Not implemented"));
	return false;
}

int Sqlite3::get_media_paths(const std::string &playlist, sigc::slot<IterFlag(fs::path)> callback)
{
	// location to insert `playlist` in the query (to avoid an SQL injection).
	constexpr int BOUND_PARAM = 1;
	static const std::string query = fmt::format(
		R"(SELECT [{}] FROM [{}] WHERE [{}] = (
			SELECT [{}] FROM [{}] WHERE [{}] = ?{:d}
		) ORDER BY [{}] ASC;)",
		Tab::Files::NAME, Tab::FILES, Tab::Files::PLAYLIST_ID,
		Tab::Playlists::ID, Tab::PLAYLISTS, Tab::Playlists::NAME, BOUND_PARAM,
		Tab::Files::INDEX
	);
	
	SqliteStmt stmt;
	if (const int rc = stmt.prepare(_handle, query); rc != SQLITE_OK) {
		SPDLOG_ERROR("sqlite3_prepare() failed ({:d}): {:s}", rc, sqlite3_errstr(rc));
		return -1;
	}
	if (stmt.bind_text(BOUND_PARAM, playlist) == SQLITE_NOMEM) { throw std::bad_alloc(); }
	
	const fs::path base = this->get_database_location() / Directory::PLAYLISTS / playlist;
	
	int rcode = 0, iterations = 0;
	while ((rcode = stmt.step()) == SQLITE_ROW) {
		constexpr int COLUMN = 0;
		ASSERT_SQLITE_COLUMN(stmt, COLUMN, SQLITE3_TEXT, Tab::Files::NAME);
		
		const char *filename = stmt.column_text(COLUMN);
		if (filename == nullptr && sqlite3_errcode(_handle) == SQLITE_NOMEM) {
			throw std::bad_alloc();
		}
		const int len = stmt.column_bytes(COLUMN);
		
		++iterations;
		const IterFlag res = callback(base / fs::path(&filename[0], &filename[len]));
		if (res == IterFlag::STOP) { return iterations; }
	}
	
	if (rcode != SQLITE_DONE) {
		SPDLOG_ERROR("sqlite3_step() failed ({:d}): {:s}", rcode, sqlite3_errstr(rcode));
		return -1;
	}
	return iterations;
}

bool Sqlite3::set_playlist_data(const std::string&, const std::vector<fs::path>&)
{
	assert(!bool("Not implemented"));
	return false;
}

int Sqlite3::close_handle(void)
{
	const int err = sqlite3_close_v2(_handle);
	if (err == SQLITE_OK) {
		_handle = nullptr;
	}
	else {
		SPDLOG_CRITICAL("sqlite3_close_v2() failed ({:d}): {:s}", err, sqlite3_errstr(err));
	}
	return err;
}

}
