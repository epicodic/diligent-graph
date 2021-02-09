#pragma once

#include <vector>
#include <set>

#include <dg/input/keys.hpp>


namespace dg {

/// keys pressed simultaneously
struct KeySequenceSimKeys
{
    std::set<int> all_keys;
    int last_key = 0;

    bool operator==(const KeySequenceSimKeys& rhs) const
    {
        return all_keys == rhs.all_keys && last_key == rhs.last_key;
    }

    bool operator<(const KeySequenceSimKeys& rhs) const
    {
        return all_keys<rhs.all_keys || last_key < rhs.last_key;
    }
};

using KeySequence = std::vector<KeySequenceSimKeys>;

KeySequence keySequenceFromString(const std::string& s);
std::string keySequenceToString(const KeySequence& key_seq);


}
