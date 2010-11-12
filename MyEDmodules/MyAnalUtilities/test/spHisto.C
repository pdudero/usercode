
//======================================================================

TH1 *findHisto(const string& hid, const string& errmsg="")
{
  map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
  if (it == glmap_id2histo.end()) {
    // Try finding the first
    cerr << "Histo ID " << hid << " not found. " << errmsg << endl;
    return NULL;
  }
  return it->second->histo();
}                                                           // findHisto

//======================================================================

void printHisto2File(TH1 *histo, string filename)
{
  FILE *fp = fopen(filename.c_str(),"w");
  if (histo->InheritsFrom("TH3")) {
    TH3 *h3 = (TH3 *)histo;
    int totaln = (int)h3->GetEntries();
    fprintf(fp,"#%s\t%s\t%s\tw\t%%\n",
	    h3->GetXaxis()->GetTitle(),
	    h3->GetYaxis()->GetTitle(),
	    h3->GetZaxis()->GetTitle());
    for (int ix=1; ix<=h3->GetNbinsX(); ix++)
      for (int iy=1; iy<=h3->GetNbinsY(); iy++)
	for (int iz=1; iz<=h3->GetNbinsZ(); iz++)
	  fprintf(fp,"%d\t%d\t%d\t%d\t%f\n",
		  (int)h3->GetXaxis()->GetBinCenter(ix),
		  (int)h3->GetYaxis()->GetBinCenter(iy),
		  (int)h3->GetZaxis()->GetBinCenter(iz),
		  (int)h3->GetBinContent(ix,iy,iz),
		  100.*h3->GetBinContent(ix,iy,iz)/(float)totaln);
    cout << "File " << filename << " written with contents of ";
    cout << h3->GetName() << endl;
  } else if (histo->InheritsFrom("TH2")) {
    TH2 *h2 = (TH2 *)histo;
    fprintf(fp,"#x\ty\tz\n");
    for (int ix=1; ix<=h2->GetNbinsX(); ix++)
      for (int iy=1; iy<=h2->GetNbinsY(); iy++)
	fprintf(fp,"%d\t%d\t%d\n",
		(int)h2->GetXaxis()->GetBinCenter(ix),
		(int)h2->GetYaxis()->GetBinCenter(iy),
		(int)h2->GetBinContent(ix,iy));
    cout << "File " << filename << " written with contents of ";
    cout << h2->GetName() << endl;
  } else {
    TH1 *h1 = (TH1 *)histo;
    fprintf(fp,"#x\ty\n");
    for (int ix=1; ix<=h1->GetNbinsX(); ix++)
      fprintf(fp,"%d\t%d\n",
	      (int)h1->GetBinCenter(ix),
	      (int)h1->GetBinContent(ix));
    cout << "File " << filename << " written with contents of ";
    cout << h1->GetName() << endl;
  }
  fclose(fp);
}                                                     // printHisto2File

//======================================================================

void saveHisto2File(TH1 *histo, string rootfn)
{
  TFile *rootfile;

  //This filename input can be an alias
  //
  if (rootfn[0] == '@') {  // reference to an alias defined in ALIAS section
    rootfn = extractAlias(rootfn.substr(1));
    if (!rootfn.size()) return;
  }

  // Check to see if the file has already been opened

  map<string,TFile*>::const_iterator it = glmap_id2rootfile.find(rootfn);
  if (it != glmap_id2rootfile.end())
    rootfile = it->second;
  else {
    rootfile = new TFile(rootfn.c_str(),"RECREATE");
  }

  if (rootfile->IsZombie()) {
    cerr << "File failed to open, " << rootfn << endl;
  } else {
    cout<<"Writing histo "<<histo->GetName()<<" to file "<<rootfn<<endl;
    glmap_id2rootfile.insert(pair<string,TFile*>(rootfn,rootfile));
    histo->SetDirectory(rootfile);
    histo->Write();
  }
}                                                      // saveHisto2File

//======================================================================

wTH1 *getHistoFromSpec(const string& hid,
		       const string& spec)
{
  wTH1  *wth1     = NULL;
  TFile *rootfile = NULL;
  vector<string> v_tokens;
  string fullspec;     // potentially expanded from aliases.

  cout << "processing " << spec << endl;

  string hspec;
  string rootfn;

  // Expand aliii first
  if (spec.find('@') != string::npos) {
    //assert(0);
    string temp=spec;
    expandAliii(temp,fullspec);
    if (!fullspec.size()) return NULL;
  } else {
    fullspec = spec;
  }

  Tokenize(fullspec,v_tokens,":");
  if ((v_tokens.size() != 2) ||
      (!v_tokens[0].size())  ||
      (!v_tokens[1].size())    ) {
    cerr << "malformed root histo path file:folder/subfolder/.../histo " << fullspec << endl;
    return NULL;
  } else {
    rootfn = v_tokens[0];
    hspec  = v_tokens[1];
  }

  map<string,string>::const_iterator it = glmap_objpaths2id.find(fullspec);
  if (it != glmap_objpaths2id.end()) {
    // Allow the possibility to run the script a second time in root
    cout << "Object " << fullspec << " already read in, here it is" << endl;
    map<string,wTH1 *>::const_iterator hit = glmap_id2histo.find(it->second);
    if (hit == glmap_id2histo.end()) {
      cout << "oops, sorry, I lied." << endl;
      return NULL;
    }
    wth1 = hit->second;
  } else {
    // Now check to see if this file has already been opened...
    map<string,TFile*>::const_iterator it = glmap_id2rootfile.find(rootfn);
    if (it != glmap_id2rootfile.end())
      rootfile = it->second;
    else
      rootfile = new TFile(rootfn.c_str());

    if (rootfile->IsZombie()) {
      cerr << "File failed to open, " << rootfn << endl;
    } else {
      glmap_id2rootfile.insert(pair<string,TFile*>(rootfn,rootfile));
      TH1 *h1 = (TH1 *)rootfile->Get(hspec.c_str());
      if (!h1) {
	cerr << "couldn't find " << hspec << " in " << rootfn << endl;
      } else {
	// success, record that you read it in.
	cerr << "Found " << fullspec << endl;
	glmap_objpaths2id.insert(pair<string,string>(fullspec,hid));
	wth1 = new wTH1(h1);
	glmap_id2histo.insert(pair<string,wTH1 *>(hid,wth1));
      }
    }
  }
  return wth1;
}                                                    // getHistoFromSpec

//======================================================================

void processCommonHistoParams(const string& key, 
			      const string& value,
			      wTH1& wh)
{
  static float xmin=1e99, xmax=-1e99;
  static float ymin=1e99, ymax=-1e99;
  static float zmin=1e99, zmax=-1e99;

  vector<string> v_tokens;

  // Allow user to define a set of common parameters to be used multiple times
  //
  if (key == "applystyle") {
    map<string,TStyle *>::const_iterator it = glmap_id2style.find(value);
    if (it == glmap_id2style.end()) {
      cerr << "Style " << value << " not found, ";
      cerr << "must be defined first" << endl;
      return;
    } else {
      wh.SaveStyle(it->second);
    }
  }
  else if (key == "draw")        wh.SetDrawOption(value);
  else if (key == "title")       wh.histo()->SetTitle(value.c_str());

  else if (key == "markercolor") wh.SetMarker(str2int(value));
  else if (key == "markerstyle") wh.SetMarker(0,str2int(value));
  else if (key == "markersize")  wh.SetMarker(0,0,str2int(value));
  else if (key == "linecolor")   wh.SetLine(str2int(value));
  else if (key == "linestyle")   wh.SetLine(0,str2int(value));
  else if (key == "linewidth")   wh.SetLine(0,0,str2int(value));
  else if (key == "fillcolor")   wh.histo()->SetFillColor(str2int(value));
  else if (key == "leglabel")    wh.SetLegendEntry(value);

  else if (key == "rebin")       ((TH1 *)wh.histo())->Rebin(str2int(value));

  else if (key == "rebinx") {
    if (wh.histo()->InheritsFrom("TH2")) ((TH2 *)wh.histo())->RebinX(str2int(value));
    else                                 ((TH1 *)wh.histo())->Rebin(str2int(value));
  }
  else if (key == "rebiny") {
    if (wh.histo()->InheritsFrom("TH2")) ((TH2 *)wh.histo())->RebinY(str2int(value));
    else {
      cerr << "\trebiny: not defined for 1-d histograms" << endl;
    }
  } 

  // axes
  else if (key == "xtitle")       wh.SetXaxis(value);
  else if (key == "ytitle")       wh.SetYaxis(value);
  else if (key == "ztitle")       wh.SetZaxis(value);
  else if (key == "xtitlesize")   wh.SetXaxis("",false,str2flt(value));
  else if (key == "ytitlesize")   wh.SetYaxis("",false,str2flt(value));
  else if (key == "ztitlesize")   wh.SetZaxis("",false,str2flt(value));
  else if (key == "xtitleoffset") wh.SetXaxis("",false,0,str2flt(value));
  else if (key == "ytitleoffset") wh.SetYaxis("",false,0,str2flt(value));
  else if (key == "ztitleoffset") wh.SetZaxis("",false,0,str2flt(value));
  else if (key == "xtitlefont")   wh.SetXaxis("",false,0,0,str2int(value));
  else if (key == "ytitlefont")   wh.SetYaxis("",false,0,0,str2int(value));
  else if (key == "ztitlefont")   wh.SetZaxis("",false,0,0,str2int(value));
  else if (key == "xlabelsize")   wh.SetXaxis("",false,0,0,0,str2flt(value));
  else if (key == "ylabelsize")   wh.SetYaxis("",false,0,0,0,str2flt(value));
  else if (key == "zlabelsize")   wh.SetZaxis("",false,0,0,0,str2flt(value));
  else if (key == "xlabelfont")   wh.SetXaxis("",false,0,0,0,0,str2int(value));
  else if (key == "ylabelfont")   wh.SetYaxis("",false,0,0,0,0,str2int(value));
  else if (key == "zlabelfont")   wh.SetZaxis("",false,0,0,0,0,str2int(value));
  else if (key == "xndiv")  wh.SetXaxis("",false,0,0,0,0,0,1e99,-1e99,str2int(value));
  else if (key == "yndiv")  wh.SetYaxis("",false,0,0,0,0,0,1e99,-1e99,str2int(value));
  else if (key == "zndiv")  wh.SetZaxis("",false,0,0,0,0,0,1e99,-1e99,str2int(value));

  else if (key == "xmin") {
    xmin = str2flt(value);
    wh.SetXaxis("",false,0,0,0,0,0,xmin,xmax);
  }
  else if (key == "xmax") {
    xmax = str2flt(value);
    wh.SetXaxis("",false,0,0,0,0,0,xmin,xmax);
  }
  else if (key == "ymin") {
    ymin = str2flt(value);
    wh.SetYaxis("",false,0,0,0,0,0,ymin,ymax);
  }
  else if (key == "ymax") {
    ymax = str2flt(value);
    wh.SetYaxis("",false,0,0,0,0,0,ymin,ymax);
  }
  else if (key == "zmin") {
    zmin = str2flt(value);
    wh.SetZaxis("",false,0,0,0,0,0,zmin,zmax);
  }
  else if (key == "zmax") {
    zmax = str2flt(value);
    wh.SetZaxis("",false,0,0,0,0,0,zmin,zmax);
  }

  else if (key == "xbinlabels") {
    Tokenize(value,v_tokens,",");
    if (!v_tokens.size()) {
      cerr << "expect comma-separated list of bin labels ";
      cerr << value << endl;
    }
    cout << "Loaded " << v_tokens.size() << " x-axis bin labels" << endl;
    for (int ibin=1; ibin<=min((int)v_tokens.size(),wh.histo()->GetNbinsX()); ibin++)
      wh.histo()->GetXaxis()->SetBinLabel(ibin,v_tokens[ibin-1].c_str());
  }

  else if (key == "ybinlabels") {
    Tokenize(value,v_tokens,",");
    if (!v_tokens.size()) {
      cerr << "expect comma-separated list of bin labels ";
      cerr << value << endl;
    }
    for (int ibin=1; ibin<=min((int)v_tokens.size(),wh.histo()->GetNbinsY()); ibin++)
      wh.histo()->GetYaxis()->SetBinLabel(ibin,v_tokens[ibin-1].c_str());
  }

  // stats box
  else if (key == "statson")    wh.SetStats(str2int(value) != 0);
  else if (key == "statsx1ndc") wh.SetStats(wh.statsAreOn(),str2flt(value));
  else if (key == "statsy1ndc") wh.SetStats(wh.statsAreOn(),0.0,str2flt(value));
  else if (key == "statsx2ndc") wh.SetStats(wh.statsAreOn(),0.0,0.0,str2flt(value));
  else if (key == "statsy2ndc") wh.SetStats(wh.statsAreOn(),0.0,0.0,0.0,str2flt(value));

  else if ((key == "errorson") &&
	   str2int(value)     ) wh.histo()->Sumw2();

  else if (key == "erroroption") {
    // only for TProfiles.
    // allowed options are
    // ' ' (default, e=spread/sqrt(n)
    // 's' (e=spread)
    // others -> see ROOT documentation!
    //
    if (wh.histo()->InheritsFrom("TProfile")) {
      ((TProfile *)wh.histo())->SetErrorOption(value.c_str());
    }
  }
  else if (key == "print2file") {
    printHisto2File(wh.histo(),value);
  }
  else if (key == "save2file") {
    saveHisto2File(wh.histo(),value);
  }
  else if ((key == "normalize") &&
	   str2int(value)) {
    TH1 *h1 = (TH1 *)wh.histo();
    if (h1->Integral() > 0.0)
      h1->Scale(1./h1->Integral());
    else
      cerr << h1->GetName() << " integral is ZERO, cannot normalize." << endl;
  }
  else if (key == "scaleby") {
    // Expect value=@samplename(integlumi_invpb)
    Tokenize(value,v_tokens,"()");
    if (v_tokens.size() != 2) {
      cerr << "invalid scaleby specifier, " << value << endl;
      return;
    }
    
    double sf = getSampleScaleFactor(v_tokens[0],
				     (double)str2flt(v_tokens[1]));
    TH1 *h1 = (TH1 *)wh.histo();
    h1->Scale(sf);
  }
  else if (key == "fits") {
    Tokenize(value,v_tokens,","); 
    for (size_t i=0; i<v_tokens.size(); i++) {
      TF1 *tf1 = findTF1(v_tokens[i]);
      if (!tf1) {
	cerr << "TF1 " << v_tokens[i] << "must be defined first" << endl;
      }
      wh.loadFitFunction(tf1);
    }
  }
  else {
    cerr << "unknown key " << key << endl;
  }
}                                            // processCommonHistoParams

//======================================================================

void processCommonHistoParams(const string& key, 
			      const string& value,
			      const std::vector<wTH1 *>& v_wh)
{
  for (size_t i=0; i<v_wh.size(); i++) {
    processCommonHistoParams(key,value,*(v_wh[i]));
  }
}

//======================================================================

bool                              // returns true if success
processHistoSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;

  wTH1   *wth1 = NULL;
  string *hid  = NULL;

  cout << "Processing histo section" << endl;

  new_section=false;

  while (getLine(fp,theline,"histo")) {
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
      if (hid != NULL) {
	cerr << "no more than one id per histo section allowed " << value << endl;
	break;
      }

      hid = new string(value);

    //------------------------------
    } else if (key == "clone") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(value);
      if (it == glmap_id2histo.end()) {
	cerr << "Histo ID " << value << " not found,";
	cerr << "clone must be defined after the clonee" << endl;
	break;
      }
      wth1 = it->second->Clone(string(it->second->histo()->GetName())+"_clone",
			       string(it->second->histo()->GetTitle()));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wth1));

    //------------------------------
    } else if (key == "path") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (wth1) {
	cerr << "histo already defined" << endl; continue;
      }
      wth1  = getHistoFromSpec(*hid,value);
      if (!wth1) continue;

    //------------------------------
    } else if (key == "hprop") {    // fill a histogram with a per-bin property of another
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (wth1) {
	cerr << "histo already defined" << endl; continue;
      }

      // Tokenize again to get the histo ID and the desired property
      string hprop = value;
      Tokenize(hprop,v_tokens,":");
      if ((v_tokens.size() != 2) ||
	  (!v_tokens[0].size())  ||
	  (!v_tokens[1].size())    ) {
	cerr << "malformed hid:property specification " << hprop << endl; continue;
      }

      string tgthid = v_tokens[0];
      string prop   = v_tokens[1];

      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(tgthid);
      if (it == glmap_id2histo.end()) {
	cerr << "Histo ID " << tgthid << " not found, histo must be defined first" << endl;
	break;
      }
      TH1 *tgth1 = it->second->histo();
      wth1       = it->second->Clone(string(tgth1->GetName())+"_"+prop,
				     string(tgth1->GetTitle())+" ("+prop+")");
      TH1 *h1=wth1->histo();
      h1->Clear();
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wth1));
      
      if (!prop.compare("errors")) {
	int nbins = h1->GetNbinsX()*h1->GetNbinsY()*h1->GetNbinsZ();
	for (int ibin=1; ibin<=nbins; ibin++)
	  h1->SetBinContent(ibin,tgth1->GetBinError(ibin));
      }
      else {
	cerr << "Unrecognized property: " << prop << endl;
	break;
      }

    } else if (!wth1) {  // all other keys must have "path" defined
      cerr << "key 'path' or 'clone' must be defined before key " << key << endl;
      break;
    }

    else {
      processCommonHistoParams(key,value,*wth1);
    }
  }
  return (wth1 != NULL);
}                                                 // processHistoSection

//======================================================================
