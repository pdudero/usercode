#include <sstream>
#include <vector>
#include <algorithm> // std::search
#include <string>

//======================================================================

inline unsigned int str2uint(const std::string& str) {
  return (unsigned int)strtoul(str.c_str(),NULL,0);
}

inline int str2int(const std::string& str) {
  return (int)strtol(str.c_str(),NULL,0);
}

inline float str2flt(const std::string& str) {
  return (float)strtod(str.c_str(),NULL);
}

inline double str2dbl(const std::string& str) {
  return strtod(str.c_str(),NULL);
}

inline std::string int2str(int i) {
  std::ostringstream ss;
  ss << i;
  return ss.str();
}

//======================================================================
// Got this from
// http://www.velocityreviews.com/forums/t286357-case-insensitive-stringfind.html
//
bool ci_equal(char ch1, char ch2)
{
  return (toupper((unsigned char)ch1) ==
          toupper((unsigned char)ch2));
}

size_t ci_find(const std::string& str1, const std::string& str2)
{
  std::string::const_iterator pos = search(str1.begin(), str1.end(),
					   str2.begin(), str2.end(),
					   ci_equal);
  if (pos == str1.end())
    return std::string::npos;
  else
    return (pos-str1.begin());
}

//======================================================================
// Got this from
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
// returns one token (the whole string) if none of the delimiters are found.
//
void Tokenize(const std::string& str,
	      std::vector<std::string>& tokens,
	      const std::string& delimiters = " ",
	      bool include_delimiters=false)
{
  std::string src=str;
  tokens.clear();

  // Skip delimiters at beginning.
  std::string::size_type lastPos = src.find_first_not_of(delimiters, 0);

  if (include_delimiters && lastPos>0)
    tokens.push_back(src.substr(0,lastPos));

  // Find first delimiter.
  std::string::size_type pos = src.find_first_of(delimiters, lastPos);

  while (pos != std::string::npos || lastPos != std::string::npos) {
    // Found a token, add it to the vector.
    tokens.push_back(src.substr(lastPos, pos - lastPos));

    lastPos = src.find_first_not_of(delimiters, pos);

    if (include_delimiters && pos!=std::string::npos) {
      tokens.push_back(src.substr(pos, lastPos-pos));
    } //else skip delimiters.

    // Find next delimiter
    pos = src.find_first_of(delimiters, lastPos);

  }
  //cout << "tokens.size() = " << tokens.size() <<endl;
}                                                            // Tokenize

//======================================================================

std::string stripDirsAndSuffix(const std::string& input)
{
  std::string output;
  size_t startpos=input.find_last_of('/');
  size_t endpos  =input.find_last_of('.');
  if (startpos==std::string::npos) startpos = 0;
  else startpos++;
  if (endpos==std::string::npos) output=input.substr(startpos);
  else                           output=input.substr(startpos,endpos-startpos);

  return output;
}

//======================================================================
