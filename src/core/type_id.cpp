#include <dg/core/type_id.hpp>

#include <map>
#include <string>

namespace dg {

namespace detail {

TypeId TypeIdHelperBase::nextId(const std::type_info& type)
{
	static TypeId next_id = 0;
	static std::map<std::string, TypeId> known_ids;

	std::string typeStr = type.name();
	auto it = known_ids.find(typeStr);
	if(it!=known_ids.end())
		return it->second;

	++next_id;
	known_ids[typeStr] = next_id;

	return next_id;
}

}
}
