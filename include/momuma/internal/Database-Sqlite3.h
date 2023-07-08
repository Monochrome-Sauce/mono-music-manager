#ifndef DATABASE_SQLITE3_H
#define DATABASE_SQLITE3_H

#define SQLITE_OMIT_DEPRECATED
#include <sqlite3.h>

#include "IDatabase.h"


namespace Momuma::Database
{

class Sqlite3 final : public IDatabase
{
public:
	enum class StorageType { DISK, MEMORY };
	
	sqlite3 *_handle;
	
	
	/* #Creates (if doesn't already exist) new database inside of the folder `fullFolderPath`.
	! @param fullFolderPath: path to the root folder of the database, created if doesn't exist.
	! @param storage: store the database on disk or in memory.
	*/
	Sqlite3(const FilePath &fullFolderPath, StorageType storage = StorageType::DISK);
	
	Sqlite3(Sqlite3&&);
	
	virtual ~Sqlite3(void) override;
	
	[[nodiscard]] explicit operator bool(void) const;
	
	FilePath get_database_location(void) noexcept override;
	
	int get_playlists(sigc::slot<IterFlag(std::string)> callback) override;
	
	bool remove_playlist(const std::string &playlist) override;
	
	int get_media_paths(
		const std::string &playlist, sigc::slot<IterFlag(FilePath)> callback
	) override;
	
	bool set_playlist_data(const std::string &playlist, const NameList &title) override;
	
private:
	const FilePath m_path;
	
	int close_handle(void);
};

}

#endif /* DATABASE_SQLITE3_H */
