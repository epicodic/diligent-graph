#pragma once

#include <typeinfo>
#include <type_traits>

namespace dg {


/// The type of the integral TypeId, that can be retrieved by typeId<T>()
typedef int TypeId;

namespace detail {

struct TypeIdHelperBase
{
	static TypeId nextId(const std::type_info& type);
};

template <typename T>
struct TypeIdHelper : public TypeIdHelperBase
{
	static TypeId id() {
		static TypeId id = nextId(typeid(T));
		return id;
	}
};

}

/**
 * Generates unique IDs for different types.
 * Useful for checking if two types are identical, storing type information, etc.
 */
template<typename T>
TypeId type_id() // NOLINT
{
	// make type id for unqualified type
	return detail::TypeIdHelper< typename std::remove_cv<T>::type >::id();
}

}
