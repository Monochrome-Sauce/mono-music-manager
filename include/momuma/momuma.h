#ifndef MONO_MUSIC_MANAGER__MOMUMA_H
#define MONO_MUSIC_MANAGER__MOMUMA_H

#include "Database-Sqlite3.h"
#include "MpvPlayer.h"


namespace Momuma
{

namespace fs = std::filesystem;

/* #Initializes Momuma.
! The `Momuma::init()` function should be called to initialize Momuma before calling
any other `Momuma` functions.
! @return `true` if Momuma could be initialized.
*/
[[nodiscard]] extern bool init(void);

/* #Clean up any resources created by Momuma upon initialization.
! After this call Momuma (including this method) should not be used anymore.
*/
extern void deinit(void);


class Momuma
{
public:
	Momuma(const fs::path &rootFolder);
	Momuma(Momuma &&other) = default;
	
	Momuma(const Momuma&) = delete;
	Momuma& operator=(const Momuma&) = delete;
	
	[[nodiscard]] explicit operator bool(void);
	
	[[nodiscard]] MpvPlayer& get_player(void) { return m_player; }
	[[nodiscard]] Database::Sqlite3& get_database(void) { return m_database; }
	
	[[nodiscard]]
	fs::path get_location(void);
	
private:
	MpvPlayer m_player;
	Database::Sqlite3 m_database;
};

}

#endif /* MONO_MUSIC_MANAGER__MOMUMA_H */
