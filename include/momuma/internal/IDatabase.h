#ifndef IDATABASE_H
#define IDATABASE_H

#include <filesystem>
#include <optional>
#include <sigc++/slot.h>
#include <vector>


namespace Momuma
{

/* #Abstract base class defining an interface for different Database implementations.
! Many methods return an `optional` in the case of a failed query.
*/
class IDatabase
{
protected:
	using FilePath = std::filesystem::path;
	using PathList = std::vector<FilePath>;
	using NameList = std::vector<std::string>;
	
public:
	enum class IterFlag : bool { STOP = false, NEXT = !STOP, };
	
	IDatabase(const IDatabase&) = delete;
	IDatabase(IDatabase&&) = delete;
	
	IDatabase(void) {}
	virtual ~IDatabase(void) {}
	
	// #Returns the full path to the database's root folder.
	[[nodiscard]] virtual
	FilePath get_database_location(void) noexcept = 0;
	
	/* #Queries the names of saved playlists.
	! @param callback: callback function which will receive `std::string` s in
	the stored playing order. The callback can return `false` to stop half-way.
	! @return: the number of times `callback()` was called. `-1` on failure.
	*/
	[[nodiscard]] virtual
	int get_playlists(sigc::slot<IterFlag(std::string)> callback) = 0;
	
	/* #Creates a new playlist in the database.
	! @param playlist: name of the new playlist.
	! @return: 'false' if the playlist doesn't exist at the end of the operation.
	*/
	[[nodiscard]] virtual
	bool create_playlist(const std::string &playlist) = 0;
	
	/* #Removes the given playlist from the database.
	! @param playlist: name of a playlist to remove.
	! @return: `true` on success. If `false` is returned, the existing list didn't change.
	*/
	[[nodiscard]] virtual
	bool remove_playlist(const std::string &playlist) = 0;
	
	/* #Queries a list of absolute paths to media files.
	! @param playlist: name of a playlist to extract the media from.
	! @param callback: callback function which will receive `FilePath` s in
	the stored playing order. The callback can return `false` to stop half-way.
	! @return: the number of times `callback()` was called. `-1` on failure.
	*/
	[[nodiscard]] virtual
	int get_media_paths(
		const std::string &playlist, sigc::slot<IterFlag(FilePath)> callback
	) = 0;
	
	
	
	/* #Sets a list of absolute paths to media files, replacing the existing list.
	! @param playlist: name of a playlist to modify.
	! @param mediaPath: list of absolute paths to the media files. The order is preserved.
	! @return: `true` on success. If `false` is returned, the existing list didn't change.
	*/
	[[nodiscard]] virtual
	bool set_playlist_data(const std::string &playlist, const NameList &title) = 0;
};

}

#endif /* IDATABASE_H */
