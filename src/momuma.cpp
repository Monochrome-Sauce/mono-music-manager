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

fs::path Momuma::get_location(void)
{
	return m_database.get_database_location();
}

}
