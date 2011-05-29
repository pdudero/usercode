#include <sstream>

#ifndef LINELEN
#define LINELEN 512
#endif

static unsigned gl_linect=0;

//======================================================================

inline unsigned int str2int(const string& str) {
  return (unsigned int)strtoul(str.c_str(),NULL,0);
}

inline float str2flt(const string& str) {
  return (float)strtod(str.c_str(),NULL);
}

inline string int2str(int i) {
  ostringstream ss;
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

size_t ci_find(const string& str1, const string& str2)
{
  string::const_iterator pos = search(str1.begin(), str1.end(),
				      str2.begin(), str2.end(), ci_equal);
  if (pos == str1.end())
    return string::npos;
  else
    return (pos-str1.begin());
}

//======================================================================
// Got this from
// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
// returns one token (the whole string) if none of the delimiters are found.
//
void Tokenize(const string& str,
	      vector<string>& tokens,
	      const string& delimiters = " ",
	      bool include_delimiters=false)
{
  string src=str;
  tokens.clear();

  // Skip delimiters at beginning.
  string::size_type lastPos = src.find_first_not_of(delimiters, 0);
  if (include_delimiters && lastPos>0)
    tokens.push_back(src.substr(0,lastPos));

  // Find first "non-delimiter".
  string::size_type pos = src.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(src.substr(lastPos, pos - lastPos));

    lastPos = src.find_first_not_of(delimiters, pos);

    if (include_delimiters && pos!=string::npos) {
      tokens.push_back(src.substr(pos, lastPos-pos));
    } //else skip delimiters.

    // Find next delimiter
    pos = src.find_first_of(delimiters, lastPos);
  }
}                                                            // Tokenize

//======================================================================

bool getLine(FILE *fp, string& theline, const string& callerid="")
{ 
  char linein[LINELEN];

  if (!feof(fp) && fgets(linein,LINELEN,fp)) {
    if (strlen(linein) && (linein[strlen(linein)-1] == '\n'))
      linein[strlen(linein)-1] = '\0';
    theline = string(linein);
  } else return false;

  //cout << theline <<  ", callerid = " << callerid << endl;

  gl_linect++;

  return true;
}                                                             // getLine

//======================================================================

bool getKeyValue(const string& theline,
		 string& key,
		 string& value,
		 bool expandAliii=true)
{ 
  vector<string> v_tokens;
  Tokenize(theline,v_tokens,"=");

  if ((v_tokens.size() < 2) ||
      (!v_tokens[0].size()) ||
      (!v_tokens[1].size())    ) {
    cerr << "malformed key=value line " << theline << endl;
    return false;
  }

  key = v_tokens[0];

  // allow for '=' in the value, but not in the key!
  //
  for (unsigned i=1; i<v_tokens.size(); i++) {
    if (value.size()) value += "=";
    value+=v_tokens[i];
  }

  for( int i=0; 
       expandAliii && (value.find('@') != string::npos);
       i++) {
    string temp=value;
    extern void expandAliii(const string& input, 
			    string& output);
      
    expandAliii(temp,value);
    if( !value.size()) return false;
    if( i>=10 ) {
      cerr << "Potential alias mutual self-reference cycle detected, please fix!" << endl;
      exit(-1);
    }
  }

  return true;
}                                                         // getKeyValue

//======================================================================

string stripDirsAndSuffix(const string& input)
{
  string output;
  size_t startpos=input.find_last_of('/');
  size_t endpos  =input.find_last_of('.');
  if (startpos==string::npos) startpos = 0;
  else startpos++;
  if (endpos==string::npos) output=input.substr(startpos);
  else                      output=input.substr(startpos,endpos-startpos);

  return output;
}

//======================================================================
