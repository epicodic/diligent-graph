#pragma once

#include <functional>
#include <type_traits>

// extends std::hash by missing features such as hash for enums and c-arrays

namespace dg {

template <typename T, typename=void>
class hash : public std::hash<T> {}; // NOLINT


template <class T>
inline std::size_t hash_value(const T& v) // NOLINT
{
	hash<T> hasher;
    return hasher(v);
}


template <class T>
inline void hash_combine(std::size_t& seed, const T& v) // NOLINT
{
	hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

// hash enums with their underlying types
template<typename T>
struct hash<T, typename std::enable_if<std::is_enum<T>::value>::type> : public hash<typename std::underlying_type<T>::type>
{
};

// hash arrays by combining their items
template<typename T, int N>
struct hash<T[N]> {
std::size_t operator()(const T* v) const
{
	std::size_t seed = 0;
    for(int i=0; i<N; ++i)
    	 hash_combine(seed, v[i]);
	return seed;
}
};

}
