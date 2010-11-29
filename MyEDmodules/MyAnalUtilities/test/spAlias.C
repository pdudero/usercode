static map<string, string>          glmap_aliii;        // better than aliases

//======================================================================

const string& extractAlias(const string& input)
{
  map<string,string>::const_iterator it;
  it = glmap_aliii.find(input);
  if (it == glmap_aliii.end()) {
    cerr<<"alias "<<input<<" not found, define reference in ALIAS section first."<<endl;
    return nullstr;
  }
  return it->second;
}                                                        // extractAlias

//======================================================================

void expandAliii(const string& input, 
		 string& output)
{
  const bool include_delimiters = true;
  const string ispunc("\",./<>?;:'[]{}\\|`~!#$%^&*()_-+=");

  output.clear();
  vector<string> v_tokens;
  Tokenize(input,v_tokens, ispunc, include_delimiters);
  for (size_t i=0; i<v_tokens.size(); i++) {
    if (v_tokens[i][0] == '@')
      output += extractAlias(v_tokens[i].substr(1));
    else
      output += v_tokens[i];
  }
  //cout << "expanded "<<input<<"==>"<<output<<endl;
}                                                         // expandAliii

//======================================================================

bool                                        // returns true if success
processAliasSection(FILE *fp,string& theline, bool& new_section)
{
  vector<string> v_tokens;

  cout << "Processing alias section" << endl;

  new_section=false;

  while (getLine(fp,theline,"alias")) {

    if (!theline.size())   continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    string key, value;
    if (!getKeyValue(theline,key,value)) continue;

    //--------------------
    if (key=="includefile") { // reserved for putting aliii in a separate file
    //--------------------
      FILE *fp = fopen(value.c_str(),"r");
      if (!fp) {
	cerr<<"Error, couldn't open alias include file '"<<value<<"'"<<endl;
	exit(-1);
      }
      cout << "Loading include file '" << value << "'" << endl;
      processAliasSection(fp,theline,new_section);

    } else {
      map<string,string>::const_iterator it;
      it = glmap_aliii.find(key);
      if (it != glmap_aliii.end()) {
	cerr << "Error in ALIAS section: key '" << key;
	cerr << "' already defined." << endl;
      } else {
	// Alias can include other aliii, but certain characters if
	// encountered are considered to bound the alias key, rather
	// than be a part of it, such as '/' and ':'
	string aspec = value;
	if (aspec.find('@') != string::npos) {
	  string temp=aspec;
	  expandAliii(temp,aspec);
	  if (!aspec.size()) continue;
	}

	glmap_aliii.insert(pair<string,string>(key,aspec));
	cout << "alias '" << key << "' added" << endl;
      }
    }
  } // while loop

  return true;
}                                                 // processAliasSection

//======================================================================