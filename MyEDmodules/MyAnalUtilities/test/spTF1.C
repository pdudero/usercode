#include "TF1.h"
#include "TRegexp.h"
#include "TObjString.h"
#include "TString.h"

static map<string, TF1 *>       glmap_id2tf1;

//======================================================================

TF1 *findTF1(const string& fid)
{
  cout << "looking for " << fid << endl;
  map<string,TF1 *>::const_iterator it = glmap_id2tf1.find(fid);
  if (it == glmap_id2tf1.end()) {
    cerr << "TF1 ID " << fid << " not found" << endl;
    return NULL;
  }
  return it->second;
}                                                             // findTF1

//======================================================================
// Regex match a TF1 in a directory
//
void regexMatchTF1( TObject    *obj,
		    TDirectory *dir,
		    TObjArray  *Args,   // list of regexes to match
		    TObjArray  *Matches)
{
  for (int i=0; i<Args->GetEntriesFast(); i++) {
    TObjString *sre = (TObjString *)(*Args)[i];
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
	(obj->InheritsFrom("TF1"))) {
      // we have a match
      // Check to see if it's already in memory
      map<string,string>::const_iterator it = glmap_objpath2id.find(dir->GetPath());
      if (it != glmap_objpath2id.end()) {
	if (gl_verbose)
	  cout << "Object " << fullspec << " already read in, here it is" << endl;
	map<string,TF1 *>::const_iterator hit = glmap_id2tf1.find(it->second);

	// Is this okay? It's going to get wrapped again...
	TObjString *rpath = new TObjString(fullspec);
	Matches->AddLast(rpath);
	Matches->AddLast(hit->second);
      } else {
	// success, record that you read it in.
	TObjString *rpath = new TObjString(fullspec);
	Matches->AddLast(rpath);
	Matches->AddLast(obj);
      }
      break; // don't let the object match more than one regex
    } // if we have a match
  } // Arg loop
}                                                       // regexMatchTF1

//======================================================================

void getTF1sFromRE(const string&     fid,
		   const string&   filepath,
		   const string&   sre,
		   vector<std::pair<string,TF1*> >&  v_tf1)
{
  if (gl_verbose)
    cout<<"Searching for regexp "<<sre<<" in "<<filepath;

  // allow for multiple regexes in OR combination
  //
  vector<string> v_regexes;
  Tokenize(sre,v_regexes,"|");
  if (!v_regexes.size())
    v_regexes.push_back(sre);

  // Build validated TRegexp arguments in preparation for directory recursion
  //
  TObjArray *Args = new TObjArray();
  for (size_t i=0; i<v_regexes.size(); i++) {
    TRegexp re(v_regexes[i].c_str(),kTRUE);
    if (re.Status() != TRegexp::kOK) {
      cerr << "The regexp " << v_regexes[i] << " is invalid, Status() = ";
      cerr << re.Status() << endl;
      exit(-1);
    }
    else {
      Args->AddLast(new TObjString(v_regexes[i].c_str()));
    }
  }

  // Get the root file
  //
  TFile *rootfile = openRootFile(filepath);

  if (!rootfile) {
    cerr << "File failed to open, " << filepath << endl;
    Args->Delete();
    delete Args;
    return;
  }

  // Do the recursion, collect matches
  //
  TObjArray *Matches = new TObjArray();
  recurseDirs(rootfile, &regexMatchTF1, Args, Matches);
  Args->Delete();
  delete Args;

  // Returns two objects per match: 
  // 1. the (string) path that was matched and
  // 2. the object whose path matched
  //
  int nx2matches = Matches->GetEntriesFast();
  if (gl_verbose) cout << "... " << nx2matches/2 << " match(es) found.";

  // Add the matches to the global map of TF1s
  //
  int istart = v_tf1.size();

  for (int i=0; i<nx2matches; i+=2) {
    TString fullspec = ((TObjString *)(*Matches)[i])->GetString();
    TF1 *tf1 = new TF1(*(TF1 *)((*Matches)[i+1]));
    tf1->UseCurrentStyle();
#if 0
    tf1->SetLineColor(((i/2)%9)+1);
    tf1->SetLineStyle((i/18)+1);
    tf1->SetLineWidth(2);
    tf1->SetLegendEntry(tf1->GetName());
#endif
    string fidi= fid+"_"+int2str(istart+(i/2));
    v_tf1.push_back(std::pair<string,TF1 *>(fidi,tf1));

    //glmap_objpath2id.insert(pair<string,string>(fullspec,fidi));
    glmap_id2tf1.insert(pair<string,TF1 *>(fidi,tf1));
    glmap_id2objpath.insert(pair<string,string>(fidi,string(fullspec.Data())));
  }

  //Matches->Delete(); // need the TF1s!
  delete Matches;

  if (gl_verbose) cout << endl;
}                                                     // getTF1sFromRE

//======================================================================

bool                              // returns true if success
processTF1Section(FILE *fp,
		  string& theline,
		  bool& new_section)
{
  string *fid  = NULL;
  TF1    *f1   = NULL;
  double xmin=0.0, xmax=1.0; // same as root defaults
  int lcol=-1,lwid=-1,lsty=-1;
  vector<string> v_tokens;
  vector<string> parstrs;
  vector<std::pair<string, TF1 *> > v_tf1s;

  cout << "Processing TF1 section" << endl;

  new_section=false;

  while (getLine(fp,theline,"TF1")) {
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
      if (fid != NULL) {
	cerr << "no more than one id per F1 section allowed " << value << endl;
	break;
      }
      fid = new string(value);
      
      map<string, TF1 *>::const_iterator fit = glmap_id2tf1.find(*fid);
      if (fit != glmap_id2tf1.end()) {
	cerr << "Function id " << *fid << " already defined" << endl;
	break;
      }

    //------------------------------
    } else if (key == "formula") {
    //------------------------------
      if (!fid) {
	cerr << "id key must be defined before formula key" << endl; continue;
      }
      if (v_tf1s.size()) {
	cerr << "TF1 " << *fid << " already defined" << endl; continue;
      }
      f1 = new TF1(fid->c_str(),value.c_str());

      v_tf1s.push_back(std::pair<string,TF1 *>(*fid,f1));
      glmap_id2tf1.insert(pair<string,TF1 *>(*fid,f1));

    //------------------------------
    } else if (key == "compilemacro") {
    //------------------------------
      if (!fid) {
	cerr << "id key must be defined before compilemacro key" << endl; continue;
      }
      int error;
      cout << "Compiling macro " << *fid << " --> " << value << endl;
      
      f1 = (TF1 *)gROOT->Macro(value.c_str(), &error, kTRUE);
      if (error) {
	static const char *errorstr[] = {
	  "kNoError","kRecoverable","kDangerous","kFatal","kProcessing" };
	cerr << "ERROR: error returned from macro: " << errorstr[error] << endl;
      }
      else if( !(f1 && f1->InheritsFrom("TF1")) ) {
	cerr<<"Macro "<<value<<" does not return a TF1 object"<<endl;
	exit(-1);
      } else {
	v_tf1s.push_back(std::pair<string,TF1 *>(*fid,f1));
	glmap_id2tf1.insert(pair<string,TF1 *>(*fid,f1));
      }

    //------------------------------
    } else if (key == "initpars") {
    //------------------------------

      if (!fid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }

      if (!v_tf1s.size()) {
	cerr << "ERROR: TF1s must be created prior to fixpar key" << endl;
	exit(-1);
      }

      Tokenize(value,parstrs,",");

      for (size_t i=0; i<v_tf1s.size(); i++) {
	f1 = v_tf1s[i].second;
	for (size_t j=0; j<parstrs.size(); j++) {
	  f1->SetParameter(j,str2flt(parstrs[j]));
	}
      }

    //------------------------------
    } else if (key == "fixpar") { // fix one parameter
    //------------------------------

      if (!fid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }

      if (!v_tf1s.size()) {
	cerr << "ERROR: TF1s must be created prior to fixpar key" << endl;
	exit(-1);
      }

      Tokenize(value,parstrs,",");
      if( (parstrs.size() != 2) ||
	  (str2int(parstrs[0]) > v_tf1s[0].second->GetNpar()) )	{
	cerr<<"ERROR: expect fixpar=index,value syntax, where index < nparameters"<<endl;
	exit(-1);
      }
      for (size_t i=0; i<v_tf1s.size(); i++) {
	f1 = v_tf1s[i].second;
	f1->FixParameter(str2int(parstrs[0]),
			 str2flt(parstrs[1]));
      }

    //------------------------------
    } else if (key == "setparlimits") { // SetParLimits for one parameter
    //------------------------------

      if (!fid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }

      if (!v_tf1s.size()) {
	cerr << "ERROR: TF1s must be created prior to fixpar key" << endl;
	exit(-1);
      }

      Tokenize(value,parstrs,",");
      if( (parstrs.size() != 3) ||
	  (str2int(parstrs[0]) > v_tf1s[0].second->GetNpar()) ||
	  (str2flt(parstrs[2]) < str2flt(parstrs[1])) )	{
	cerr<<"ERROR: expect fixpar=index,min,max syntax, where index < nparameters, min <= max"<<endl;
	exit(-1);
      }
      for (size_t i=0; i<v_tf1s.size(); i++) {
	f1 = v_tf1s[i].second;
	f1->SetParLimits(str2int(parstrs[0]),
			 str2flt(parstrs[1]),
			 str2flt(parstrs[2]) );
      }

    //------------------------------
    } else if (key == "setpar") { // set one parameter
    //------------------------------

      if (!fid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }

      if (!v_tf1s.size()) {
	cerr << "ERROR: TF1s must be created prior to setpar key" << endl;
	exit(-1);
      }

      Tokenize(value,parstrs,",");
      if( (parstrs.size() != 2) ||
	  (str2int(parstrs[0]) > v_tf1s[0].second->GetNpar()) )	{
	cerr<<"ERROR: expect setpar=index,value syntax, where index < nparameters"<<endl;
	exit(-1);
      }
      for (size_t i=0; i<v_tf1s.size(); i++) {
	f1 = v_tf1s[i].second;
	f1->SetParameter(str2int(parstrs[0]),
			 str2flt(parstrs[1]));
      }

    //------------------------------
    } else if (key == "pathglob") { // multiple TF1s defined over potentially multiple files
    //------------------------------
      glob_t globbuf;
      
      if (!fid) {
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
      // regular expression pattern can select multiple TF1s within each file.
      //
      string fileglob = v_tokens[0];
      string stregex  = v_tokens[1];

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
      if (gl_verbose) cout<<globbuf.gl_pathc<<" files match the glob pattern"<<endl;
      for (size_t i=0; i<globbuf.gl_pathc; i++) {
	char *path = globbuf.gl_pathv[i];
	if (!strncmp(&path[strlen(path)-6],".root",5)) {
	  cerr << "non-root file found in glob, skipping: " << path << endl;
	} else {
	  getTF1sFromRE(*fid,string(path),stregex, v_tf1s);
	}
      }
      if (gl_verbose) cout << v_tf1s.size() << " total matches found." << endl;
      globfree(&globbuf);

      glmap_mobj2size.insert(pair<string,unsigned>(*fid,v_tf1s.size()));
    }

    //------------------------------
    else if (key == "range") {
    //------------------------------

      if (!fid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }

      if (!v_tf1s.size()) {
	cerr << "ERROR: TF1s must be created prior to range key" << endl;
	exit(-1);
      }

      Tokenize(value,v_tokens,",-");
      if ( (v_tokens.size() != 2) || 
	   (!v_tokens[0].size())  ||
	   (!v_tokens[1].size())  ||
	   (str2flt(v_tokens[0]) > str2flt(v_tokens[1]))
	 ) {
	cerr << "malformed TF1 range spec range=xmin[,|-]xmax: " << value << endl;
	exit(-1);
      }
      else {
	xmin = str2flt(v_tokens[0]);
	xmax = str2flt(v_tokens[1]);
	for (size_t i=0; i<v_tf1s.size(); i++) {
	  f1 = v_tf1s[i].second;
	  f1->SetRange(xmin,xmax);
	}
      }
    }

    //------------------------------
    else if (key == "linecolor") lcol = str2int(value);
    else if (key == "linestyle") lsty = str2int(value);
    else if (key == "linewidth") lwid = str2int(value);
    //------------------------------

  } // line loop

  for (size_t i=0; i<v_tf1s.size(); i++) {
    if (lsty>=0) v_tf1s[i].second->SetLineStyle(lsty);
    if (lcol>=0) v_tf1s[i].second->SetLineColor(lcol);
    if (lwid>=0) v_tf1s[i].second->SetLineWidth(lwid);
  }

  if (fid) delete fid;
  if (v_tf1s.size()&&gl_verbose) {
    cout << "Dump of first TF1: " << endl;
    v_tf1s[0].second->Print();
  }
    
  return (v_tf1s.size());
}                                                   // processTF1section

//======================================================================
