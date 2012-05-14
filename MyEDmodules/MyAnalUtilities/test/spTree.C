#include "TTree.h"
static map<string, TTree *>       glmap_id2tree;

//======================================================================

TTree *findTree(const string& tid)
{
  cout << "looking for " << tid << endl;
  map<string,TTree *>::const_iterator it = glmap_id2tree.find(tid);
  if (it == glmap_id2tree.end()) {
    cerr << "Tree ID " << tid << " not found" << endl;
    return NULL;
  }
  return it->second;
}                                                            // findTree

//======================================================================

void fillHistoFromTreeVar(std::string& treedrawspec,
			  wTH1 *&wth1)
{
  vector<string> v_tokens;
  string tid, drawspec;
  Tokenize(treedrawspec,v_tokens,":",true);
  if( (v_tokens.size() < 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[2].size())    ) {
    cerr << "malformed root tree draw spec treeid:\"varexp\",\"selection\",option" << treedrawspec << endl;
    return;
  } else {
    tid = v_tokens[0];
    for (size_t i=2; i<v_tokens.size(); i++) {
      drawspec += v_tokens[i];
    }
    cout<<"drawspec="<<drawspec<<endl;
    TTree *tree = findTree(tid);
    if (tree) {
      Tokenize(drawspec,v_tokens,"\"");
      string varexp = v_tokens[0] + ">>+" + wth1->histo()->GetName(); // append to pre-existing histo
      cout<<"varexp="<<varexp<<endl;
      switch(v_tokens.size()) {
      case 1: tree->Draw(varexp.c_str(),"","goff"); break;
      case 3: tree->Draw(varexp.c_str(),v_tokens[2].c_str(),"goff"); break;
      case 5: tree->Draw(varexp.c_str(),v_tokens[2].c_str(),v_tokens[4].c_str()); break;
      default:
	cerr << "malformed root tree draw spec treeid:varexp,selection,option";
	cerr << treedrawspec << endl;
	break;
      }
    }
  }
}                                                // fillHistoFromTreeVar

//======================================================================

TTree *getTreeFromSpec(const string& tid,
		       const string& spec)
{
  TTree  *tree    = NULL;
  TFile *rootfile = NULL;
  vector<string> v_tokens;
  string fullspec;

  if( gl_verbose)
    cout << "processing " << spec << endl;

  string tspec;
  string rootfn;

  fullspec = spec;

  Tokenize(fullspec,v_tokens,":");
  if( (v_tokens.size() != 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[1].size())    ) {
    cerr << "malformed root tree path file:folder/subfolder/.../histo " << fullspec << endl;
    return NULL;
  } else {
    rootfn = v_tokens[0];
    tspec  = v_tokens[1];
  }

#if 0
  map<string,string>::const_iterator it = glmap_objpath2id.find(fullspec);
  if( it != glmap_objpath2id.end() ) {
    // Allow the possibility to run the script a second time in root
    if( gl_verbose)
      cout << "Object " << fullspec << " already read in, here it is" << endl;
    map<string,tree *>::const_iterator hit = glmap_id2histo.find(it->second);
    if( hit == glmap_id2histo.end() ) {
      if( gl_verbose)
	cout << "oops, sorry, I lied." << endl;
      return NULL;
    }
    tree = hit->second;
  } else {
#endif
    rootfile = openRootFile(rootfn);
    if (rootfile) {
      tree = (TTree *)rootfile->Get(tspec.c_str());
      if( !tree) {
	cerr << "couldn't find " << tspec << " in " << rootfn << endl;
      } else {
	// success, record that you read it in.
	if( gl_verbose) cout << "Found " << fullspec << endl;
	glmap_objpath2id.insert(pair<string,string>(fullspec,tid));
	glmap_id2objpath.insert(pair<string,string>(tid,fullspec));
	glmap_id2tree.insert(pair<string,TTree *>(tid,tree));
      }
    }
    //}
  return tree;
}                                                     // getTreeFromSpec

//======================================================================

bool                              // returns true if success
processTreeSection(FILE *fp,
		   string& theline,
		   bool& new_section)
{
  string *tid  = NULL;
  TTree  *t1   = NULL;

  cout << "Processing Tree section" << endl;

  new_section=false;

  while (getLine(fp,theline,"Tree")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      break;
    }

    string key, value;
    if (!getKeyValue(theline,key,value)) continue;

    //--------------------
    if (key == "id") {
    //--------------------
      if (tid != NULL) {
	cerr << "no more than one id per F1 section allowed " << value << endl;
	break;
      }
      tid = new string(value);
      
      map<string, TTree *>::const_iterator tit = glmap_id2tree.find(*tid);
      if (tit != glmap_id2tree.end()) {
	cerr << "Tree id " << *tid << " already defined" << endl;
	break;
      }
    //------------------------------
    } else if( key == "path" ) {
    //------------------------------
      if( !tid ) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if( t1 ) {
	cerr << "histo already defined" << endl; continue;
      }
      t1 = getTreeFromSpec(*tid,value);
      if( !t1 ) continue;
    }
    else {
      cerr << "unknown key " << key << endl;
    }
  }

  if (tid) delete tid;
  return (t1 != NULL);
}                                                  // processTreesection

//======================================================================
