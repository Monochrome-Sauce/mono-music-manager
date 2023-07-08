#include "momuma/momuma.h"


namespace Momuma
{

[[nodiscard]] extern
bool init(void)
{
	return !setenv("LC_NUMERIC", "C", true);
}

extern
void deinit(void)
{
}


Momuma::Momuma(const fs::path &rootFolder) :
	m_player { }, m_database { rootFolder }
{
}

Momuma::operator bool(void)
{
	return static_cast<bool>(m_player) && static_cast<bool>(m_database);
}

fs::path Momuma::get_location(void)
{
	return m_database.get_database_location();
}

}
