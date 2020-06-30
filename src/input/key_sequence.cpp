#include <dg/input/key_sequence.hpp>

#include <list>

#include <dg/input/string_utils.hpp>
#include <dg/input/keys.hpp>

namespace dg {

KeySequence keySequenceFromString(const std::string& s)
{
	KeySequence seq;

	std::wstring ws_w = string_to_wstring(s);
	std::wstring ws;

	// remove spaces first
	for(auto& c : ws_w)
		if(!std::isspace(c))
			ws.push_back(c);

	std::list<std::wstring> wparts;

	// first split sequence at commas
	int pos = 0;
	int token0 = 0;
	while(pos < ws.size())
	{
		bool is_last_in_token = pos+1 == ws.size() || ws[pos+1] == L',';
		if(ws[pos] == L',' && !is_last_in_token)
		{
			// if this is the last , in the token then , belongs to the token as in Ctrl+,,Alt+.
			// otherwise, we found a separator

			std::wstring wtoken = ws.substr(token0, pos-token0);
			wparts.push_back(wtoken);

			++pos;
			token0 = pos;
		}
		else
			++pos;
	}

	std::wstring wtoken = ws.substr(token0, pos-token0);
	wparts.push_back(wtoken);

	for(const std::wstring& wpart : wparts)
	{
		KeySequenceSimKeys simkeys;

		int pos = 0;
		int token0 = 0;
		while(pos < wpart.size())
		{
			bool is_last_in_token = pos+1 == wpart.size();
			if(wpart[pos] == L'+' && !is_last_in_token)
			{
				std::wstring wtoken = wpart.substr(token0, pos-token0);

				int key = keyFromName(wstring_to_string(wtoken));
				if(!key)
					DG_THROW("Unknown key in sequence: " + wstring_to_string(wtoken));
				simkeys.all_keys.insert(key);

				++pos;
				token0 = pos;
			}
			else
				++pos;
		}
		std::wstring wtoken = wpart.substr(token0, pos-token0);
		int key = keyFromName(wstring_to_string(wtoken));
		if(!key)
			DG_THROW("Unknown key in sequence: " + wstring_to_string(wtoken));
		simkeys.all_keys.insert(key);
		simkeys.last_key = key;

		seq.push_back(simkeys);
	}

	return seq;

}


}
