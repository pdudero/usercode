#include "TKey.h"
#include "TRegexp.h"
#include "TObjArray.h"

// multi-ID to vector of histo IDs
static map<string, vector<string> >     glmap_mid2hid;

//======================================================================

void regexMatch( TObject    *obj,
		 TDirectory *dir,
		 TObjArray  *Args,
		 TObjArray  *Matches)
{
  TObjString *sre = (TObjString *)(*Args)[0];
  TRegexp re(sre->GetString(),kFALSE);
  if (re.Status() != TRegexp::kOK) {
    cerr << "The regexp " << sre->GetString() << " is invalid, Status() = ";
    cerr << re.Status() << endl;
    exit(-1);
  }

  TString path( (char*)strstr( dir->GetPath(), ":" ) );
  path.Remove( 0, 2 ); // gets rid of ":/"

  TString fullspec = TString(dir->GetPath()) + "/" + obj->GetName();

  if ((fullspec.Index(re) != kNPOS) &&
      (obj->InheritsFrom("TH1"))) {
    // we have a match
    // Check to see if it's already in memory
    map<string,string>::const_iterator it = glmap_objpaths2id.find(dir->GetPath());
    if (it != glmap_objpaths2id.end()) {
      cout << "Object " << fullspec << " already read in, here it is" << endl;
      map<string,wTH1 *>::const_iterator hit = glmap_id2histo.find(it->second);

      // Is this okay? It's going to get wrapped again...
      Matches->AddLast(hit->second->histo());
    } else {
      // success, record that you read it in.
      Matches->AddLast(obj);
    }
  }
}                                                          // regexMatch

//======================================================================

void recurseDirs( TDirectory *thisdir,
		  void (*doFunc)(TObject *, TDirectory *,TObjArray *, TObjArray *),
		  TObjArray *Args,
		  TObjArray *Output)
{
  assert(doFunc);

  //thisdir->cd();

  // loop over all keys in this directory

  TIter nextkey( thisdir->GetListOfKeys() );
  TKey *key;
  while ( (key = (TKey*)nextkey())) {

    TObject *obj = key->ReadObj();

    if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      // it's a subdirectory, recurse
      //cout << "Checking path: " << ((TDirectory *)obj)->GetPath() << endl;
      recurseDirs( (TDirectory *)obj, doFunc, Args, Output );
    } else {
      doFunc(obj, thisdir, Args, Output);
    }
  } // key loop
}                                                         // recurseDirs

//======================================================================

void getHistosFromRE(const string&   mhid,
		     const string&   filepath,
		     const string&   sre,
		     vector<wTH1*>&  v_wth1)
{
  TRegexp re(sre.c_str(),kTRUE);
  cout<<"Searching for regexp "<<sre<<" in "<<filepath<<endl;
  if (re.Status() != TRegexp::kOK) {
    cerr << "The regexp " << sre << " is invalid, Status() = ";
    cerr << re.Status() << endl;
    exit(-1);
  }

  TFile *rootfile = NULL;
  map<string,TFile*>::const_iterator it = glmap_id2rootfile.find(filepath);
  if (it != glmap_id2rootfile.end())
    rootfile = it->second;
  else
    rootfile = new TFile(filepath.c_str());

  if (rootfile->IsZombie()) {
    cerr << "File failed to open, " << filepath << endl;
    return;
  } else {
    glmap_id2rootfile.insert(pair<string,TFile*>(filepath,rootfile));

    TObjArray *Args    = new TObjArray();
    TObjArray *Matches = new TObjArray();
    TObjString objsre(sre.c_str());
    Args->AddFirst(&objsre);
    recurseDirs(rootfile, &regexMatch, Args, Matches);
    delete Args;

    vector<string> v_hidi;

    // Add the matches to the global map of histos
    for (int i=0; i<Matches->GetEntriesFast(); i++) {
      wTH1 *wth1 = new wTH1((TH1 *)((*Matches)[i]));
      string hidi= mhid+"_"+int2str(i);
      glmap_id2histo.insert(pair<string,wTH1 *>(hidi,wth1));
      v_wth1.push_back(wth1);
      v_hidi.push_back(hidi);
      //glmap_objpaths2id.insert(pair<string,string>(fullspec,hidi));
    }

    glmap_mid2hid.insert(pair<string,vector<string> >(mhid,v_hidi));

    delete Matches;
  }
}                                                     // getHistosFromRE

//======================================================================

bool                              // returns true if success
processMultiHistSection(FILE *fp,
			string& theline,
			bool& new_section)
{
  vector<string> v_tokens;
  vector<wTH1 *> v_wth1;
  string mhid;

  cout << "Processing multihist section" << endl;

  new_section=false;

  while (getLine(fp,theline,"multihist")) {
    if (!theline.size()) continue;
    if (theline[0] == '#') continue; // comments are welcome

    if (theline[0] == '[') {
      new_section=true;
      return true;
    }

    string key, value;
    if (!getKeyValue(theline,key,value)) continue;

    //--------------------
    if (key == "id") {
    //--------------------
      if (mhid.size()) {
	cerr << "no more than one id per histo section allowed " << value << endl;
	break;
      }

      mhid = value;

    //------------------------------
    } else if (key == "pathglob") {
    //------------------------------
      glob_t globbuf;
      
      if (!mhid.size()) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }

      Tokenize(value,v_tokens,":");
      if ((v_tokens.size() != 2) ||
	  (!v_tokens[0].size())  ||
	  (!v_tokens[1].size())    ) {
	cerr << "malformed pathglob 'fileglob:regex' " << value << endl;
	exit(-1);
      }

      // File globbing pattern can select multiple files
      // regular expression pattern can select multiple histos within each file.
      // Aliases containing glob pattern not yet implemented.
      //
      string fileglob = v_tokens[0];

      // Check for file alias
      if (fileglob[0] == '@') {
	assert(0);
	string rootfn = extractAlias(fileglob.substr(1));
	if (!rootfn.size()) {
	  exit(-1);
	}
	getHistosFromRE(mhid,rootfn,v_tokens[1], v_wth1);
      } else {
	int stat = glob (fileglob.c_str(), GLOB_MARK, NULL, &globbuf);
	if (stat) {
	  switch (stat) {
	  case GLOB_NOMATCH: cerr << "No file matching glob pattern "; break;
	  case GLOB_NOSPACE: cerr << "glob ran out of memory "; break;
	  case GLOB_ABORTED: cerr << "glob read error "; break;
	  default: cerr << "unknown glob error stat=" << stat << " "; break;
	  }
	  cerr << fileglob << endl;
	  exit(-1);
	}

	for (size_t i=0; i<globbuf.gl_pathc; i++) {
	  char *path = globbuf.gl_pathv[i];
	  if (!strncmp(&path[strlen(path)-6],".root",5)) {
	    cerr << "non-root file found in glob, skipping: " << path << endl;
	  } else {
	    getHistosFromRE(mhid,string(path),v_tokens[1], v_wth1);
	  }
	}
	globfree(&globbuf);
      }
    } else if (!v_wth1.size()) {  // all other keys must have "path" defined
      cerr << "key 'pathglob' or 'vartable' must be defined before key " << key << endl;
      break;
    }

    else {
      for (size_t i=0; i<v_wth1.size(); i++)
	processCommonHistoParams(key,value,*(v_wth1[i]));
    }
  }

  return (v_wth1.size());
}                                             // processMultiHistSection

//======================================================================
