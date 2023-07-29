#ifndef MONO_MUSIC_MANAGER__INTERNAL__DATABASE_SQLITE3_H
#define MONO_MUSIC_MANAGER__INTERNAL__DATABASE_SQLITE3_H

#define SQLITE_OMIT_DEPRECATED
#include <filesystem>
#include <optional>
#include <sigc++/slot.h>
#include <sqlite3.h>
#include <vector>


namespace Momuma::Database
{

namespace fs = std::filesystem;
enum class IterFlag : bool { STOP = false, NEXT = !STOP, };
enum class StorageType { DISK, MEMORY };

class Sqlite3
{
public:
	
	sqlite3 *_handle;
	
	
	/* #Creates (if doesn't already exist) new database inside of the folder `fullFolderPath`.
	! @param fullFolderPath: path to the root folder of the database, created if doesn't exist.
	! @param storage: store the database on disk or in memory.
	*/
	Sqlite3(const fs::path &fullFolderPath, StorageType storage = StorageType::DISK);
	
	Sqlite3(Sqlite3&&);
	
	~Sqlite3(void);
	
	[[nodiscard]] explicit operator bool(void) const;
	
	// #Returns the full path to the database's root folder.
	fs::path get_database_location(void) noexcept;
	
	/* #Queries the names of saved playlists.
	! @param callback: callback function which will receive `std::string` s in
	the stored playing order. The callback can return `false` to stop half-way.
	! @return: the number of times `callback()` was called. `-1` on failure.
	*/
	int get_playlists(sigc::slot<IterFlag(std::string)> callback);
	
	/* #Creates a new playlist in the database.
	! @param playlist: name of the new playlist.
	! @return: 'false' if the playlist doesn't exist at the end of the operation.
	*/
	bool create_playlist(const std::string &playlist);
	
	/* #Removes the given playlist from the database.
	! @param playlist: name of a playlist to remove.
	! @return: `true` on success. If `false` is returned, the existing list didn't change.
	*/
	bool remove_playlist(const std::string &playlist);
	
	/* #Queries a list of absolute paths to media files.
	! @param playlist: name of a playlist to extract the media from.
	! @param callback: callback function which will receive `FilePath` s in
	the stored playing order. The callback can return `false` to stop half-way.
	! @return: the number of times `callback()` was called. `-1` on failure.
	*/
	int get_media_paths(
		const std::string &playlist, sigc::slot<IterFlag(fs::path)> callback
	);
	
	/* #Sets a list of absolute paths to media files, replacing the existing list.
	! @param playlist: name of a playlist to modify.
	! @param paths: list of absolute paths to the media files. The order is preserved.
	! @return: `true` on success. If `false` is returned, the existing list didn't change.
	*/
	bool set_playlist_data(const std::string &playlist, const std::vector<fs::path> &paths);
	
private:
	const fs::path m_path;
	
	int close_handle(void);
};

}

#endif /* MONO_MUSIC_MANAGER__INTERNAL__DATABASE_SQLITE3_H */
