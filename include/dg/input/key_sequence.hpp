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

	bool operator==(const KeySequenceSimKeys& rhs)
	{
		return all_keys == rhs.all_keys && last_key == rhs.last_key;
	}
};

typedef std::vector<KeySequenceSimKeys> KeySequence;

KeySequence keySequenceFromString(const std::string& s);


}
