#include <dg/core/type_id.hpp>

#include <map>
#include <string>

namespace dg {

namespace detail {

TypeId TypeIdHelperBase::nextId(const std::type_info& type)
{
    static TypeId next_id = 0;
    static std::map<std::string, TypeId> known_ids;

    std::string type_str = type.name();
    auto it = known_ids.find(type_str);
    if(it!=known_ids.end())
        return it->second;

    ++next_id;
    known_ids[type_str] = next_id;

    return next_id;
}

}
}
