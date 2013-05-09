#include <sstream>
#include "TKey.h"
#include "TObjArray.h"

#ifndef LINELEN
#define LINELEN 512
#endif

#include "utils.C"

static unsigned gl_linect=0;

//======================================================================

bool getLine(FILE *fp, string& theline, const string& callerid="")
{ 
  char linein[LINELEN];

  if (!feof(fp) && fgets(linein,LINELEN,fp)) {
    if (strlen(linein) && (linein[strlen(linein)-1] == '\n'))
      linein[strlen(linein)-1] = '\0';

    if (gl_inloop) {
      TString tstrline(linein); 
      int nfmt = tstrline.CountChar('%');
      if (nfmt) {
	switch(nfmt){
	case 1: Form(tstrline,gl_loopindex); break;
	case 2: Form(tstrline,gl_loopindex,gl_loopindex); break;
	case 3: Form(tstrline,gl_loopindex,gl_loopindex,gl_loopindex); break;
	case 4: Form(tstrline,gl_loopindex,gl_loopindex,gl_loopindex,gl_loopindex); break;
	case 5: Form(tstrline,
		     gl_loopindex,gl_loopindex,
		     gl_loopindex,gl_loopindex,gl_loopindex); break;
	case 6: Form(tstrline,
		     gl_loopindex,gl_loopindex,gl_loopindex,
		     gl_loopindex,gl_loopindex,gl_loopindex); break;
	default:
	  cerr << "Too many % fmt specifiers, fix me!" << endl;
	  exit(-1);
	}
      }
      theline=string(tstrline.Data());
    } else
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
  Tokenize(theline,v_tokens,"=",true);

  if ((v_tokens.size() < 3) ||
      (!v_tokens[0].size()) ||
      (!v_tokens[2].size())    ) {
    cerr << "malformed key=value line " << theline << endl;
    return false;
  }

  key = v_tokens[0];

  // allow for '=' in the value, but not in the key!
  //
  for (unsigned i=2; i<v_tokens.size(); i++) {
    value+=v_tokens[i];
  }

  for( int i=0; 
       expandAliii && (value.find('@') != string::npos);
       i++) {
    string temp=value;
    extern void expandAliii(const string& input, 
			    string& output);
      
    expandAliii(temp,value);
    //cout << value << endl;
    if( !value.size()) return false;
    if( i>=10 ) {
      cerr << "Potential alias mutual self-reference cycle detected, please fix!" << endl;
      cerr << "line = " << theline << endl;
      exit(-1);
    }
  }

  return true;
}                                                         // getKeyValue

//======================================================================

string buildStringFromSpecifiers(const string& formatstr,
				 const string& canvastitle="",
				 const string& rootfilepath="",
				 const string& fullhistopath="")
{
  string tgtstring;
  size_t len = formatstr.length();
  size_t pos,pos0 = 0;

  do {
    pos = formatstr.find('%',pos0);
    if (pos == string::npos) {
      tgtstring += formatstr.substr(pos0); // no more format codes, finish up
      break;
    } else if (pos>pos0) {
      tgtstring+=formatstr.substr(pos0,pos-pos0);
    }
    // expand format codes
    if (pos != len-1) {  // make sure '%' wasn't the last character
      pos0=pos+1;

      switch (formatstr[pos0]) { 
      case 'C':
	tgtstring += canvastitle;
	break;

      case 'P': {               // full path contained in glmap
	tgtstring +=
	  rootfilepath.substr(0,rootfilepath.find_last_of('.'));
      }	break;

      case 'F': {               // filename stripped of path info
	string datafile;
	size_t pos1 = rootfilepath.find_last_of('/');
	datafile =  (pos1 != string::npos) ?
	  rootfilepath.substr(pos1+1,rootfilepath.find_last_of('.')-pos1-1) :
	  rootfilepath.substr(0,rootfilepath.find_last_of('.'));
	tgtstring += datafile;
      }	break;

      case 'H':               // full path of histo inside the root file
	tgtstring += fullhistopath;
	break;

      case 'h': {              // name of histo stripped of path info
	string histoname;
	size_t pos1 = fullhistopath.find_last_of('/');
	histoname =  (pos1 != string::npos) ?
	  fullhistopath.substr(pos1+1,fullhistopath.find_last_of('.')-pos1-1) :
	  fullhistopath.substr(0,fullhistopath.find_last_of('.'));
	tgtstring += histoname;
	}
	break;
      default:
	cerr<<"Unrecognized format code %"<<formatstr[pos0]<<endl;
	break;
      }
      pos0++;
    }
  } while (pos0<len);

  return tgtstring;
}                                                     // buildStringFromSpecifiers

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
