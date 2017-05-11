#include "TObjString.h"
#include "TDCacheFile.h"

static map<string, TFile *> glmap_id2rootfile;

//======================================================================

TFile *openRootFile(const std::string& rootfn, const std::string& option="")
{
  TFile *rootfile = NULL;

  // Now check to see if this file has already been opened...
  map<string,TFile*>::const_iterator it = glmap_id2rootfile.find(rootfn);
  if( it != glmap_id2rootfile.end() ) {
    rootfile = it->second;
    if (!rootfile->IsOpen())
      if (rootfn.find(":"))
	rootfile = TFile::Open(rootfn.c_str(),option.c_str());
      else
	rootfile->Open(rootfn.c_str(),option.c_str());
    else
      rootfile->cd();
  }  else {

    if (strstr(rootfn.c_str(),"dcache") ||
	strstr(rootfn.c_str(),"dcap")      ) {
      rootfile = new TDCacheFile(rootfn.c_str(),option.c_str());
    } else if (rootfn.find(":"))
      rootfile = TFile::Open(rootfn.c_str(),option.c_str());
    else
      rootfile = new TFile(rootfn.c_str(),option.c_str());
  
    if( rootfile->IsZombie() ) {
      cerr << "File failed to open, " << rootfn << endl;
      rootfile = NULL;
    } else {
      glmap_id2rootfile.insert(pair<string,TFile*>(rootfn,rootfile));
    }
  }
  return rootfile;
}                                                        // openRootFile

//======================================================================

void closeFiles(void)
{
  TFile *rootfile = NULL;

  // Now check to see if this file has already been opened...
  map<string,TFile*>::const_iterator it;
  for( it = glmap_id2rootfile.begin(); it != glmap_id2rootfile.end(); it++ ) {
    rootfile = it->second;
    if (rootfile)
      rootfile->Close();
  }

}

//======================================================================

void save2File(TNamed *obj, string outspec)
{
  TFile *rootfile;
  string rootfn,newname;
  TNamed *target;

#if 0
  //This filename input can be an alias
  //
  if( rootfn[0] == '@') {  // reference to an alias defined in ALIAS section
    rootfn = extractAlias(rootfn.substr(1));
    if( !rootfn.size()) return;
  }
#endif

  if( outspec.find(":") != string::npos) {
    vector<string> v_tokens;
    Tokenize(outspec,v_tokens,":");

    rootfn = v_tokens[0];
    newname = v_tokens[1];
  } else
    rootfn = outspec;


  rootfile = openRootFile(rootfn,"UPDATE");
  if (!rootfile)
    rootfile = openRootFile(rootfn,"RECREATE"); // try overwriting whatever's there.

  if (rootfile) {
    target = obj;
    if( newname.size() ) {
      cout<<"Writing object "<<newname<<" to file "<<rootfn<<endl;
      target = (TNamed *)obj->Clone(newname.c_str());
      //target->SetTitle(histo->GetTitle());
    } else
      cout<<"Writing object "<<obj->GetName()<<" to file "<<rootfn<<endl;

    if (obj->InheritsFrom("TH1"))
      ((TH1 *)target)->SetDirectory(rootfile);
    target->Write();
    //rootfile->Close();
  }
}                                                           // save2File

//======================================================================

void saveAs(TNamed *obj, const TString& path)
{
  TDirectory *savedir=gDirectory;

  TObjArray *tokens = path.Tokenize(":");
  TString filename = ((TObjString *)(*tokens)[0])->GetString();

  if ((tokens->GetEntriesFast() != 2) ||
      (!filename.EndsWith(".root"))     ) {
    cerr << "ERROR: Expecting saveas=filename.root:objectname" << endl;
    return;
  }

  TFile *file = openRootFile(std::string(filename.Data()), "UPDATE");
  if (!file) {
    cerr << " Couldn't open root file " << filename << endl;
    exit(-1);
  }
  TString objname = ((TObjString *)(*tokens)[1])->GetString();

  file->cd();

  if (gl_verbose)
    cout<<"Saving "<<TString(obj->GetName())<<" as "<<objname<<" to "<<filename<<endl;

  obj->Write(objname);

  file->Flush();

  savedir->cd();
}
