#pragma once

#include <string>
#include <locale>
#include <codecvt>

namespace dg {


inline std::wstring wstr_toupper(std::wstring s, const std::locale& loc) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [&loc](wchar_t c){ return std::toupper(c, loc); }
                  );
    return s;
}

inline  std::string str_toupper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); }
                  );
    return s;
}

inline std::wstring string_to_wstring(const std::string& s)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf16conv;
	return utf16conv.from_bytes(s);
}

inline std::string wstring_to_string(const std::wstring& s)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf16conv;
	return utf16conv.to_bytes(s);
}

}
