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
	Sqlite3(Sqlite3&&);
	
	/* #Creates (if doesn't already exist) new database inside of the folder `fullFolderPath`.
	*/
	Sqlite3(const FilePath &fullFolderPath);
	
	virtual ~Sqlite3(void) override;
	
	FilePath get_database_location(void) noexcept override;
	
	int get_playlists(sigc::slot<IterFlag(std::string)> callback) override;
	
	bool remove_playlist(const std::string &playlist) override;
	
	int get_media_paths(
		const std::string &playlist, sigc::slot<IterFlag(FilePath)> callback
	) override;
	
	bool set_playlist_data(const std::string &playlist, const NameList &title) override;
	
private:
	sqlite3 *m_handle;
	const FilePath m_path;
};

}


#endif /* DATABASE_SQLITE3_H */
