#ifndef MONO_MUSIC_MANAGER__BITSET_H
#define MONO_MUSIC_MANAGER__BITSET_H

#include <bitset>
#include <concepts>
#include <type_traits>


namespace Momuma
{

// a bitset based on a type
template<typename T>
struct Bitset : public std::bitset<sizeof(T)>
{
	using Unsigned = std::make_unsigned_t<T>;
	using Signed = std::make_signed_t<T>;
	
	constexpr inline
	Bitset(const T val) :
		std::bitset<sizeof (T)> { static_cast<Unsigned>(val) }
	{}
	
	[[nodiscard]] constexpr inline
	bool contains(const T val) const
	{
		return (*this & Bitset(val)).any();
	}
};

}

#endif /* UTILS__BITSET_H */
