static map<string, TFile *> glmap_id2rootfile;

TFile *openRootFile(const std::string& rootfn)
{
  TFile *rootfile = NULL;

  // Now check to see if this file has already been opened...
  map<string,TFile*>::const_iterator it = glmap_id2rootfile.find(rootfn);
  if( it != glmap_id2rootfile.end() )
    rootfile = it->second;
  else {
    rootfile = new TFile(rootfn.c_str());
  
    if( rootfile->IsZombie() ) {
      cerr << "File failed to open, " << rootfn << endl;
      rootfile = NULL;
    } else {
      glmap_id2rootfile.insert(pair<string,TFile*>(rootfn,rootfile));
    }
  }
  return rootfile;
}
