#include "TChain.h"
#include "TPRegexp.h"
#include "TGraph.h"

static map<string, TChain *>       glmap_id2chain;

//======================================================================

TChain *findChain(const string& tid)
{
  map<string,TChain *>::const_iterator it = glmap_id2chain.find(tid);
  if (it == glmap_id2chain.end()) {
    cerr << "Tree ID " << tid << " not found" << endl;
    cerr << "Available ids are: ";
    for (it = glmap_id2chain.begin(); it != glmap_id2chain.end(); it++)
      cerr << it->first << " ";
    cerr << endl;
    return NULL;
  }
  if( gl_verbose)
    cout << "Found tree " << tid << endl;
  return it->second;
}                                                            // findChain

//======================================================================

void fillHistoFromTreeVar(std::string& treedrawspec,
			  int  index,
			  wTH1 *&wth1)
{
  // Sample treedrawspec:
  // mytree:"TDCwinstart[%d]:runnum>>winstrt%d(70,202000,209000)","evtnum==1","prof P"
  //
  vector<string> v_tokens;
  string tid;
  TString drawspec;
  Tokenize(treedrawspec,v_tokens,":",true);
  if( (v_tokens.size() < 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[2].size())    ) {
    cerr << "malformed root tree draw spec treeid:\"varexp\",\"selection\",option: " << treedrawspec << endl;
    return;
  }

  tid = v_tokens[0];
  for (size_t i=2; i<v_tokens.size(); i++) {
    drawspec += v_tokens[i];
  }
  int fmtcnt = drawspec.CountChar('%');

  if (fmtcnt) { // use index for tree array var
    switch(fmtcnt) {
    case 1: drawspec = Form(drawspec,index); break;
    case 2: drawspec = Form(drawspec,index,index); break;
    case 3: drawspec = Form(drawspec,index,index,index); break;
    case 4: drawspec = Form(drawspec,index,index,index,index); break;
    case 5: drawspec = Form(drawspec,index,index,index,index,index); break;
    case 6: drawspec = Form(drawspec,index,index,index,index,index,index); break;
    default:
      cerr << "More than six fmt specifiers in drawspec found, fix me! " << drawspec <<endl;
      exit(-1);
    }
  }
  if( gl_verbose)
    cout<<"drawspec="<<drawspec<<endl;

  TChain *chain = findChain(tid);
  assert (chain);

  // can't use comma as delimiter since histo with binning spec may be supplied
  TObjArray *tokens = drawspec.Tokenize("\"");
  TString hname;

  TString varexp = ((TObjString *)(*tokens)[0])->GetString();

  if (varexp.Contains(">>")) {
    TObjArray *rematches = TPRegexp(">>(\\w+)").MatchS(varexp); // get histo name
    cout << varexp << " " << rematches->GetEntriesFast() << endl;
    assert(rematches->GetEntriesFast() ==2);
    hname = ((TObjString *)(*rematches)[1])->GetString();

    if (!wth1 && gDirectory->Get(hname)) {
      cerr << "Error: histo name in treedraw spec "<<hname;
      cerr <<" already exists as an object in the current directory! "<<endl;
      exit(-1);
    }
    if (wth1 && !hname.EqualTo(wth1->histo()->GetName())) {
      cerr << "Error: histo name in treedraw spec "<<hname;
      cerr <<" doesn't match named histo "<<wth1->histo()->GetName()<<endl;
      exit(-1);
    }
  } else { // add histo name
    assert (wth1);
    hname = TString(wth1->histo()->GetName());
    varexp = varexp + ">>+" + hname; // append to pre-existing histo
  }

  if( gl_verbose)
    cout<<"varexp="<<varexp<<", hname="<<hname<<endl;
  switch(tokens->GetEntriesFast()) {
  case 1:
    chain->Draw(varexp,"","goff");
    break;
  case 3:
    {
      TString cut = ((TObjString *)(*tokens)[2])->GetString();
      chain->Draw(varexp,cut,"goff"); 
    }
    break;
  case 4: // assume the cut string is blank
    {
      TString gopt = ((TObjString *)(*tokens)[3])->GetString();
      gopt = gopt + " goff";
      chain->Draw(varexp,"",gopt);
    }
    break;
  case 5:
    {
      TString cut  = ((TObjString *)(*tokens)[2])->GetString();
      TString gopt = ((TObjString *)(*tokens)[4])->GetString();
      gopt = gopt + " goff";
      chain->Draw(varexp,cut,gopt);
    }
    break;
  default:
    cerr << "malformed root tree draw spec treeid:varexp,selection,option";
    for (int i=0; i<tokens->GetEntriesFast(); i++)
      cerr << i<<": "<< ((TObjString *)(*tokens)[i])->GetString() << " ";
    cerr << endl;
    break;
  }
  if (!wth1) {
    wth1 = new wTH1((TH1*)gDirectory->Get(hname));
    assert(wth1);
    wth1->histo()->UseCurrentStyle();
  }
}                                                // fillHistoFromTreeVar

//======================================================================

void fillGraphFromTreeVar(std::string& treedrawspec,int index,wGraph_t *&pwg)
{
  // Sample treedrawspec:
  // mytree:"TDCwinstart[%d]:runnum","evtnum==1","P"
  //
  vector<string> v_tokens;
  string tid;
  TString drawspec;
  Tokenize(treedrawspec,v_tokens,":",true);
  if( (v_tokens.size() < 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[2].size())    ) {
    cerr << "malformed root tree draw spec treeid:\"varexp\",\"selection\",option: " << treedrawspec << endl;
    return;
  }

  tid = v_tokens[0];
  for (size_t i=2; i<v_tokens.size(); i++) {
    drawspec += v_tokens[i];
  }
  int fmtcnt = drawspec.CountChar('%');

  if (fmtcnt) { // use index for tree array var
    switch(fmtcnt) {
    case 1: drawspec = Form(drawspec,index); break;
    case 2: drawspec = Form(drawspec,index,index); break;
    case 3: drawspec = Form(drawspec,index,index,index); break;
    case 4: drawspec = Form(drawspec,index,index,index,index); break;
    case 5: drawspec = Form(drawspec,index,index,index,index,index); break;
    case 6: drawspec = Form(drawspec,index,index,index,index,index,index); break;
    default:
      cerr << "More than six fmt specifiers in drawspec found, fix me! " << drawspec <<endl;
      exit(-1);
    }
  }
  if( gl_verbose)
    cout<<"drawspec="<<drawspec<<endl;

  TChain *chain = findChain(tid);
  assert (chain);

  // Expect the user to use commas to delimit the draw options, but 
  // we can't use comma as delimiter since user may also specify
  // histo with binning spec with commas
  //
  TObjArray *tokens = drawspec.Tokenize("\"");
  TString hname;

  TString varexp = ((TObjString *)(*tokens)[0])->GetString();

  switch(tokens->GetEntriesFast()) {
  case 1:
    if( gl_verbose)
      cout<<"chain->Draw(\""<<varexp<<"\",\"\",\"goff\")"<<endl;
    chain->Draw(varexp,"","goff");
    break;
  case 3:
    {
      TString cut = ((TObjString *)(*tokens)[2])->GetString();
      cout<<"chain->Draw(\""<<varexp<<"\",\""<<cut<<"\",\"goff\")"<<endl;
      chain->Draw(varexp,cut,"goff"); 
    }
    break;
  case 4: // assume the cut string is blank
    {
      TString gopt = ((TObjString *)(*tokens)[3])->GetString();
      gopt = gopt + " goff";
      cout<<"chain->Draw(\""<<varexp<<"\",\"\","<<gopt<<"\")"<<endl;
      chain->Draw(varexp,"",gopt);
    }
    break;
  case 5:
    {
      TString cut  = ((TObjString *)(*tokens)[2])->GetString();
      TString gopt = ((TObjString *)(*tokens)[4])->GetString();
      gopt = gopt + " goff";
      cout<<"chain->Draw(\""<<varexp<<"\",\""<<cut<<"\",\""<<gopt<<"\")"<<endl;
      chain->Draw(varexp,cut,gopt);
    }
    break;
  default:
    cerr << "malformed root tree draw spec treeid:varexp,selection,option";
    for (int i=0; i<tokens->GetEntriesFast(); i++)
      cerr << i<<": "<< ((TObjString *)(*tokens)[i])->GetString() << " ";
    cerr << endl;
    break;
  }

  assert(chain->GetSelectedRows());

  if (!pwg)
    pwg = new wGraph_t();

  assert(pwg);
  pwg->gr = new TGraph(chain->GetSelectedRows(),
		       chain->GetV2(), chain->GetV1());
  pwg->gr->Sort();
}                                                // fillGraphFromTreeVar

//======================================================================

TChain *getChainFromGlobslist(const string& tid,
			      const string& treename,
			      const string& globslist)
{
  TChain  *chain    = NULL;
  vector<string> v_tokens;
  string fullspec;

  if( gl_verbose)
    cout << "processing " << globslist << endl;

  string tspec;
  string rootfn;

  Tokenize(globslist,v_tokens,",");
  int ntok1 = (int)v_tokens.size();
  for (int i=0; i<ntok1; i++) {

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
    } else
#endif
    {
      if (!chain)
	chain = new TChain(treename.c_str());

      if (!chain->Add(v_tokens[i].c_str())) {
	cerr << "couldn't find " << treename << " in " << v_tokens[i] << endl;
	delete chain;
	return NULL;
      }
    }
  } // list loop

  // success, record that you read it in.
  if( gl_verbose) cout << "Found " << treename << " in " << globslist << endl;
  glmap_objpath2id.insert(pair<string,string>(globslist,tid));
  glmap_id2objpath.insert(pair<string,string>(tid,globslist));
  glmap_id2chain.insert(pair<string,TChain *>(tid,chain));

  return chain;
}                                               // getChainFromGlobslist

//======================================================================

bool                              // returns true if success
processTreeSection(FILE *fp,
		   string& theline,
		   bool& new_section)
{
  string *tid  = NULL;
  TChain  *t1   = NULL;
  vector<string> v_tokens;

  string treename;

  if (gl_verbose)
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
      
      map<string, TChain *>::const_iterator tit = glmap_id2chain.find(*tid);
      if (tit != glmap_id2chain.end()) {
	cerr << "Tree id " << *tid << " already defined" << endl;
	break;
      }
    //------------------------------
    } else if( key == "treename" ) {
    //------------------------------
      if( !tid ) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if( t1 ) {
	cerr << "Tree already defined" << endl; continue;
      }
      treename = value;

    //------------------------------
    } else if( key == "globslist" ) {
    //------------------------------
      t1 = getChainFromGlobslist(*tid,treename,value);
      if( !t1 )
	exit(-1);

    //------------------------------
    } else if( key == "copytree" ) {
    //------------------------------
      if( !tid ) {
	cerr << "id key must be defined first in the section" << endl; continue;      }
    
      Tokenize(value,v_tokens,",");
      
      if (v_tokens.size() != 2) {
	cerr << "copytree syntax expected: copytree=treeid,cutstring: " << value << endl; continue; }

      TChain *t2 = findChain(v_tokens[0]);
      if (!t2) {
	cerr << "tree " << v_tokens[0] << " must be defined previously" << endl; continue;    }
      if (gl_verbose)
	cout<<"Begin CopyTree of "<<v_tokens[0]<<" with selection "<<v_tokens[1]<<flush;
      
      t1 = (TChain *)(t2->CopyTree(v_tokens[1].c_str()));

      if( !t1 ) {
	cerr << "CopyTree failed" << endl; exit(-1); }

      if (gl_verbose)
	cout<<"...Done."<<endl;

    //------------------------------
    } else if( key == "save2file" ) {
    //------------------------------
      if( !t1 ) {
	cerr << "save2file: must define tree first" << endl; continue; }

      TFile *rootfile = openRootFile(value,"RECREATE");
      
      t1->SetDirectory(rootfile);
      t1->Write();
      rootfile->Flush();

      if (gl_verbose)
	cout << "Tree written to file " << value << endl;

      rootfile->Close();

    //------------------------------
    } else if( key == "unbinnedfit" ) {
    //------------------------------
      if( !tid ) {
	cerr << "id key must be defined first in the section" << endl; continue;      }
      if( !t1 ) {
	cerr << "Tree must already be defined using 'globslist'" << endl; continue;   }

      int fitresult=-99;

      Tokenize(value,v_tokens,",");
      switch(v_tokens.size()) {
      case 2: fitresult = t1->UnbinnedFit(v_tokens[0].c_str(),
					  v_tokens[1].c_str());
	break;
      case 3: fitresult = t1->UnbinnedFit(v_tokens[0].c_str(),
					  v_tokens[1].c_str(),
					  v_tokens[2].c_str()); 
	break;
      case 4: fitresult = t1->UnbinnedFit(v_tokens[0].c_str(),  // funcname
					  v_tokens[1].c_str(),  // varexp
					  v_tokens[2].c_str(),  // selection
					  v_tokens[3].c_str()); // option
	break;
      default:
	cerr << "unbinnedfit: expect 2-4 arguments separated by commas: " << value <<endl;
	exit(-1);
      }
      cout << "fit result returned = " << fitresult << endl;
      cout << "Number of selected entries in the fit = " << t1->GetSelectedRows() << endl;
    }
    else {
      cerr << "unknown key " << key << endl;
    }
  }

  if (tid) delete tid;
  return (t1 != NULL);
}                                                  // processTreesection

//======================================================================
