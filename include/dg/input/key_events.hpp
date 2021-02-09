#pragma once

#include <string>
#include <dg/input/keys.hpp>

namespace dg {

struct KeyEvent
{
    int key;
    std::string text; ///< text that this key generates (coded as UTF8), is empty for non-text keys
    std::uint32_t unicode;
};


}
