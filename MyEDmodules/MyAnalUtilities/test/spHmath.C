bool parseBinRange ( const string& binrange,
		     int& lobin,
		     int& hibin )
{
  vector<string> v_tokens;

  if (!binrange.compare("*")) {
    lobin=0; hibin=-1;
  } else {
    Tokenize(binrange,v_tokens,"-");
    if (v_tokens.size() != 2) {
      cerr << "Error, expecting binrange of form 'lobin-hibin'";
      cerr << binrange << endl;
      return false;
    }
    
    lobin=str2int(v_tokens[0]);
    hibin=str2int(v_tokens[1]);
    if (lobin > hibin) {
      cerr << "Error, expecting binrange of form 'lobin-hibin'";
      cerr << binrange << endl;
      return false;
    }
  }
  return true;
}                                                       // parseBinRange

//======================================================================

bool parseBinSpec( const string& binspec,
		   string& histname,
		   string& binrange,
		   int& lobin,
		   int& hibin )
{
  vector<string> v_tokens;

  Tokenize(binspec,v_tokens,":");
  if (v_tokens.size() != 2) {
    cerr << "Error, expecting binspec of form 'histo_id:lobin-hibin'";
    cerr << binspec << endl;
    return false;
  }

  histname = v_tokens[0];
  binrange = v_tokens[1];

  return parseBinRange(binrange,lobin,hibin);
}                                                        // parseBinSpec

//======================================================================

TH1 *IntegrateLeft(TH1 *h)
{
  string newname = string(h->GetName())+"_integleft";
  TH1 *hcum = (TH1 *)h->Clone(newname.c_str());
  hcum->Reset();

  int nbins = hcum->GetNbinsX();
  double htotal = h->Integral(1,nbins+1);

  // Include the overflow bin
  for (int i=1; i<=nbins+1 ; i++) { // includes overflow
    double integral = h->Integral(1,i);
    hcum->SetBinContent(i,integral/htotal);
  }

  return hcum;
}                                                       // IntegrateLeft

//======================================================================

TH1 *IntegrateRight(TH1 *h,double skipbinatx=-9e99)
{
  string newname = string(h->GetName())+"_integright";
  TH1 *hcum = (TH1 *)h->Clone(newname.c_str());
  hcum->Reset();

  int nbins = hcum->GetNbinsX();
  double htotal = h->Integral(1,nbins+1);
  int   skipbin = nbins+2;
  float skipcontent = 0.0;

  if (skipbinatx > -9e99) {
    for (int i=1; i<=nbins+1 ; i++) { // includes overflow
      if ((h->GetXaxis()->GetBinLowEdge(i) < skipbinatx) &&
	  (h->GetXaxis()->GetBinUpEdge(i)  > skipbinatx)   ) {
	skipbin      = i;
	skipcontent  = h->GetBinContent(i);
	break;
      }
    }
  }

  // Include the overflow bin
  for (int i=1; i<=nbins+1 ; i++) { // includes overflow
    double integral = h->Integral(i,nbins+1);
    if (i > skipbin) skipcontent=0.0;
    hcum->SetBinContent(i,(integral-skipcontent)/htotal);
    //hcum->SetBinError(i,sqrt(integral-skipcontent)/htotal);
  }

  return hcum;
}                                                      // IntegrateRight


//======================================================================

void projectX(const string& binspec,
	      const string& hid,
	      vector<pair<string,wTH1 *> >& outhistos)
{
  int lobin;
  int hibin;
  string histname,binrange;
  if (parseBinSpec(binspec,histname,binrange,lobin,hibin))
    exit(-1);

  TH2 *tmph2 = (TH2 *)findHisto(histname,"histo operand must be defined before math ops");
  if (!tmph2) exit(-1);

  string newname = hid + "_" + string(tmph2->GetName())+"_Ybins"+binrange;
  
  wTH1 *wh = new wTH1((TH1 *)tmph2->ProjectionX(newname.c_str(),lobin,hibin));
  outhistos.push_back(pair<string,wTH1 *>(hid,wh));
  glmap_id2histo.insert(pair<string,wTH1 *>(hid,wh));
}                                                            // projectX

//======================================================================

void projectY(const string& binspec,
	      const string& hid,
	      vector<pair<string,wTH1 *> >& outhistos)
{
  int lobin;
  int hibin;
  string histname,binrange;
  if (parseBinSpec(binspec,histname,binrange,lobin,hibin))
    exit(-1);

  TH2 *tmph2 = (TH2 *)findHisto(histname,"single histo operand not found");

  vector<TH2 *> v_hist;
  if (tmph2)
    v_hist.push_back(tmph2);
  else {
    string multihist1 = histname+"_0";
    tmph2 = (TH2 *)findHisto(multihist1,
    "multi histo operand not found, histo operand must be defined before math ops");
    if (tmph2) {
      v_hist.push_back(tmph2);
      for (int i=1; tmph2; i++) {
	string multihist1 = histname+"_"+int2str(i);
	tmph2 = (TH2 *)findHisto(multihist1, "hit the end of histo multiset");
	if (tmph2) v_hist.push_back(tmph2);
      }
    } else
      exit(-1);
  }

  for (size_t i=0; i<v_hist.size(); i++) {
    string newname = hid + "_" + string(v_hist[i]->GetName())+"_Xbins"+binrange;

    wTH1 *wh = new wTH1((TH1 *)v_hist[i]->ProjectionY(newname.c_str(),lobin,hibin));
    string hid2 = hid;
    if (v_hist.size() > 1) hid2 += "_"+int2str(i);
    glmap_id2histo.insert(pair<string,wTH1 *>(hid2,wh));
    outhistos.push_back(pair<string,wTH1 *>(hid2,wh));
  }
}                                                            // projectY

//======================================================================

void projectYX(const string& binspec,
	       const string& hid,
	       vector<pair<string,wTH1 *> >& outhistos)
{
  int lobin;
  int hibin;
  string histname,binrange;
  if (parseBinSpec(binspec,histname,binrange,lobin,hibin))
    exit(-1);

  TH1 *tmph = findHisto(histname,"histo operand must be defined before math ops");
  if (!tmph) exit(-1);
  if (!tmph->InheritsFrom("TH3")) {
    cerr << "operation projectyx only defined for 3D histograms, ";
    cerr << hid << endl; 
    exit(-1);
  }
  TH3 *h3 = (TH3 *)tmph;

  string newname = hid + "_" + string(h3->GetName())+"_Zbins"+binrange;

  h3->GetZaxis()->SetRange(lobin,hibin);
  tmph = (TH1 *)h3->Project3D("yx");

  // for multiple projections, otherwise root overwrites 
  //
  wTH1 *wh = new wTH1((TH1 *)tmph->Clone(newname.c_str()));
  outhistos.push_back(pair<string,wTH1 *>(hid,wh));
  glmap_id2histo.insert(pair<string,wTH1 *>(hid,wh));
}                                                           // projectYX

//======================================================================

void sliceHistos(const string slicetype,
		 const string& binspec,
		 const string& hid,
		 vector<pair<string,wTH1 *> >& outhistos)
{
  int lobin;
  int hibin;
  string histname,binrange;
  if (parseBinSpec(binspec,histname,binrange,lobin,hibin))
    exit(-1);

  TH2 *tmph2 = (TH2 *)findHisto
    (histname, "histo operand must be defined before math ops");

  if (!tmph2)
    exit(-1);

  // Save all the histos for possible future reference:
  
  string newname;
  TObjArray *aSlices = new TObjArray();
  if (slicetype.find("x")!=string::npos) {
    newname = hid + "_" + string(tmph2->GetName())+"_Xbins"+binrange;
    tmph2->FitSlicesX(0,lobin,hibin,0, "QNR",aSlices);
  } else if (slicetype.find("y")!=string::npos) {
    newname = hid + "_" + string(tmph2->GetName())+"_Ybins"+binrange;
    tmph2->FitSlicesY(0,lobin,hibin,0,"QNR",aSlices);
  }
  else return;
      
  // aSlice elements point to "dead histos walking"
  TH1 *h1 = (TH1 *)(*aSlices)[1];
  wTH1 *wh=new wTH1((TH1 *)h1->Clone());
  outhistos.push_back(pair<string,wTH1 *>(hid+"mean",wh));
  glmap_id2histo.insert(pair<string,wTH1 *>(hid+"mean",wh));

  h1 = (TH1 *)(*aSlices)[2]; 
  wh = new wTH1((TH1 *)h1->Clone()); 
  outhistos.push_back(pair<string,wTH1 *>(hid+"rms",wh));
  glmap_id2histo.insert(pair<string,wTH1 *>(hid+"rms",wh));

  h1 = (TH1 *)(*aSlices)[aSlices->GetEntriesFast()-1];
  wh = new wTH1((TH1 *)h1->Clone());
  outhistos.push_back(pair<string,wTH1 *>(hid+"chi2",wh));
  glmap_id2histo.insert(pair<string,wTH1 *>(hid+"chi2",wh));

}                                                         // sliceHistos

//======================================================================

wTH1 *catMultiHist(const string& myhid,
		   const string& multihid)
{
  wTH1D *rethist(0);
  map<string,TH1 *> m_histos2cat;

  int totalbins=0;
  for (int i=0; ; i++) {
    string histid = multihid+"_"+int2str(i);
    TH1 *h1 = findHisto(histid, "hit the end of histo multiset");
    if (h1) {
      totalbins += h1->GetNbinsX();
      m_histos2cat.insert(pair<string,TH1 *>(string(h1->GetName()),h1));
    }
    else
      break;
  }
  if (m_histos2cat.size()) {
    rethist = new wTH1D(myhid.c_str(),myhid.c_str(),totalbins,1,totalbins);
    TAxis *xax = rethist->histo()->GetXaxis();
    int glbin=1;
    map<string,TH1 *>::const_iterator it;
    for (it=m_histos2cat.begin();it != m_histos2cat.end(); it++) {
      TH1 *h = it->second;
      int nbins = h->GetNbinsX();
      string hname = it->first;
      if (nbins > 1) {
	for (int ibin=1; ibin<=nbins; ibin++,glbin++) {
	  string binlabel = hname+int2str(ibin);
	  xax->SetBinLabel(glbin,binlabel.c_str());
	  rethist->histo()->SetBinContent(glbin,h->GetBinContent(ibin));
	}
      } else {
	xax->SetBinLabel(glbin,hname.c_str());
	rethist->histo()->SetBinContent(glbin++,h->GetBinContent(1));
      }
    }
  }

  return (wTH1 *)rethist;
}                                                        // catMultiHist

//======================================================================

void x2DizeHistos(const vector<string>& inhistids,
		  const string& hid,
		  vector<pair<string,wTH1 *> >& outhistos)
{
  vector<TH1*> v_histos2dize;

  for (unsigned i=0; i<inhistids.size(); i++) {
    TH1 *tmph1 = findHisto(inhistids[i],
			   "histo operand must be defined before math ops");
    if (!tmph1) exit(-1);
    v_histos2dize.push_back(tmph1);
  }
  if (gl_verbose)
    cout << "x-2d-izing " << v_histos2dize.size() << " histos" << endl;
  int nbinsx = v_histos2dize.size();
  if (nbinsx) {
    TAxis   *xax = v_histos2dize[0]->GetXaxis();
    int   nbinsy = xax->GetNbins();
    TH1 *h1 = new TH2D(hid.c_str(),hid.c_str(),
		       v_histos2dize.size(),1.0,(float)v_histos2dize.size(),
		       nbinsy,xax->GetXmin(),xax->GetXmax());
    for (int i=1; i<=nbinsx; i++) {
      TH1 *src = v_histos2dize[i-1];
      for (int j=1; j<=xax->GetNbins(); j++)
	((TH2D *)h1)->SetBinContent(i,j,src->GetBinContent(j));
    }
    wTH1 *wh = new wTH1(h1);
    outhistos.push_back(pair<string,wTH1 *>(hid,wh));
    glmap_id2histo.insert(pair<string,wTH1 *>(hid,wh));
  }
}                                                        // x2DizeHistos

//======================================================================

void rebinVariable1D(const vector<string>& inhistids,
		     string binrange,
		     float minStatsPerBin,
		     const string& newsuffix,
		     vector<pair<string,wTH1 *> >& outhistos)
{
  if (gl_verbose)
    cout<<"Rebinning histos with "<<minStatsPerBin<<" min. stats per bin"<<endl;

  vector<TH1F *> inhistos;
  vector<float>  sums(inhistids.size(),0.);

  for (size_t i=0; i<inhistids.size(); i++) {
    TH1F *tmph1 = (TH1F *)findHisto(inhistids[i],
		      "histo operand must be defined before math ops");
    if (!tmph1) exit(-1);
    inhistos.push_back(tmph1);
  }

  if (!inhistos.size()) return;

  int lobin=1;                        // excludes underflow by default
  int hibin=inhistos[0]->GetNbinsX(); // excludes overflow by default

  if (binrange.size())
    if (!parseBinRange(binrange,lobin,hibin))
      exit(-1);

  vector<int>    oldbinnums;
  vector<double> newbinvals;

  bool   donewbin = true;
  size_t nnewbins = 0;
  float    minsum,maxval;

  if (gl_verbose) {
    cout << "New binmap: " << endl;
    printf("ibin\tnewibin\tx_bin");
    for (size_t i=0; i<inhistos.size(); i++) 
      printf("\tscale\ty_bin\tCum");
    printf("\tS_min\n");
  }

  stringstream str;
  for (int ibin=lobin; ibin <= hibin; ibin++) {
    minsum = FLT_MAX;
    maxval = -FLT_MAX;
    if (donewbin) {
      newbinvals.push_back(inhistos[0]->GetXaxis()->GetBinLowEdge(ibin));
      oldbinnums.push_back(ibin);
    }

    nnewbins = newbinvals.size();
    str.str("");
    str << ibin << "\t" << nnewbins << "\t" << newbinvals[nnewbins-1];

    for (size_t i=0; i<inhistos.size(); i++) {
      TH1F *inhist = inhistos[i];
      // To deal with weighted histograms, scale the content of each bin to
      // the number of *effective* entries in the histo:
      //
      double scale= 1.0;
      if (inhist->Integral()!=0.0) {
	//cout << inhist->GetEffectiveEntries() << "\t" << inhist->Integral() << endl;
	scale= inhist->GetEffectiveEntries()/inhist->Integral();
      }
      if (donewbin) {
	sums[i] = 0.0;
      }
      float val = scale*inhist->GetBinContent(ibin);
      maxval    = std::max(val,maxval);
      sums[i]  += val;
      minsum    = std::min(sums[i],minsum);

      str<<"\t"<<scale<<"\t"<<val<<"\t"<<sums[i];
    }

    str << "\t" << minsum;

    donewbin = false;
    if( minsum > minStatsPerBin) donewbin = true;
    if( gl_verbose && donewbin) {
      str << "\tX";
      cout << str.str() << endl;
    }
  }

  cout << str.str() << endl;
  newbinvals.push_back(inhistos[0]->GetXaxis()->GetXmax());

  // now create new varbin histos
  //
  if (gl_verbose) cout << "New histos: ";
  for (size_t i=0; i<inhistos.size(); i++) {
    TH1F   *inhist = inhistos[i];
    string newname = string(inhist->GetName())+"_"+newsuffix;
    TH1   *newhist = inhist->Rebin(nnewbins,
				   newname.c_str(),
				   &newbinvals.front());
    wTH1     *wh = new wTH1(newhist);
    string newid = inhistids[i]+"_"+newsuffix;
    if (gl_verbose) cout << "("<<newid<<","<<newname<<"), ";
    outhistos.push_back(pair<string,wTH1 *>(newid,wh));
    glmap_id2histo.insert(pair<string,wTH1 *>(newid,wh));
  }
  if (gl_verbose) cout << endl;

}                                                    // rebinVariable1D

//======================================================================

bool                              // returns true if success
processHmathSection(FILE *fp,
		    string& theline,
		    bool& new_section)
{
  vector<string> v_tokens;
  vector<pair<string,wTH1 *> > v_histos;

  string *hid = NULL;
  TH1   *h1 = NULL;
  float skipbinatx=-9e99;
  float statsPerBin=-1.;
  string binrange;

  if (gl_verbose)
    cout << "Processing hmath section" << endl;

  new_section=false;

  while (getLine(fp,theline,"hmath")) {
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
      if (hid != NULL) {
	cerr << "no more than one id per hmath section allowed " << value << endl;
	break;
      }

      hid = new string(value);

    //------------------------------
    } else if (key == "unaryop") {  // unary operation /+-* with a constant
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string hmathexpr = value;

      Tokenize(hmathexpr,v_tokens,"-+*/");
      if (v_tokens.size() != 2) {
	cerr << "only simple math ops -+*/ supported between two operands, sorry!";
	cerr << theline << endl;
	continue;
      }

      TH1 *histop = 0;
      double dummynum=0.0;
      TF1 *f1 = 0;

      string& arg1=v_tokens[0];
      string& arg2=v_tokens[1];
      if (arg1[0] == '@') {
	assert(0);
	arg1= extractAlias(arg1.substr(1));
	if (!arg1.size()) continue;
      }
      if (arg2[0] == '@') {
	assert(0);
	arg2= extractAlias(arg2.substr(1));
	if (!arg2.size()) continue;
      }

      TH1 *hres = NULL;

      histop = findHisto(arg1,"Checking:is this a histo?");
      if (!histop) {	// trying scanning a double
	if(!sscanf(arg1.c_str(),"%lg",&dummynum)) {
	  cerr << arg1 << ": it's not a known histo, and it's not a number.";
	  cerr << "I'm outta here." << endl;
	  continue;
	} else {
	  // arg1 is a constant, so
	  // arg2 must be a histogram
	  histop = findHisto(arg2,"Checking:is this a histo?");
	  if (!histop) continue;
	  if (theline.find('-') != string::npos) {
	    // the histo operand is being negated
	    //
	    // WARNING: not handling negative constants
	    //
	    f1 = new TF1("minus1","-1",
			 histop->GetXaxis()->GetXmin(),
			 histop->GetXaxis()->GetXmax());
	    histop->Multiply(f1);
	    string newname= (*hid) + "_" + string(histop->GetName())+"_"+arg1+"-this";
	    hres = (TH1 *)histop->Clone(newname.c_str());
	    f1 = new TF1("someconst",arg1.c_str(),
			 histop->GetXaxis()->GetXmin(),
			 histop->GetXaxis()->GetXmax());
	    hres->Add(f1);
	  }
	}
      } else {
	// arg1 is a histo, so
	// arg2 must be a number
	if(!sscanf(arg2.c_str(),"%lg",&dummynum)) {
	  cerr << arg2 << " must be a number, since " << arg1 << " is a histogram. ";
	  cerr << "Use 'binaryop' otherwise." << endl;
	  continue;
	}
	f1 = new TF1("myfunc",arg2.c_str(),
		     histop->GetXaxis()->GetXmin(),
		     histop->GetXaxis()->GetXmax());
	if (theline.find('-') != string::npos) {
	  string newname= (*hid) + "_" + string(histop->GetName())+"-"+arg2;
	  hres = (TH1 *)histop->Clone(newname.c_str());
	  hres->Add(f1,-1.0);
	} else if (theline.find('*') != string::npos) {
	  string newname= (*hid) + "_" + string(histop->GetName())+"x"+arg2;
	  hres = (TH1 *)histop->Clone(newname.c_str());
	} else {
	  string newname= (*hid) + "_" + string(histop->GetName())+"_?"+arg2;
	  hres = (TH1 *)histop->Clone(newname.c_str());
	}
      }

      if      (theline.find('*') != string::npos) hres->Multiply(f1);
      //else if (theline.find('/') != string::npos) hres->Divide(h1,h2,1.0,1.0,"C");
      else if (theline.find('/') != string::npos) hres->Divide(f1);
      else if (theline.find('+') != string::npos) hres->Add(f1);

      h1 = hres;
      wTH1 *wh = new wTH1(h1);
      v_histos.push_back(pair<string,wTH1 *>(*hid,wh));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "binaryop") {  // binary operation +-*/
    //------------------------------

      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string hmathexpr = value;

      Tokenize(hmathexpr,v_tokens,"-+*/");
      if (v_tokens.size() != 2) {
	cerr << "only simple math ops -+*/ supported between two operands, sorry! ";
	cerr << theline << endl;
	continue;
      }

      TH1 *tmph1 = findHisto(v_tokens[0]); if (!tmph1) continue;
      TH1 *tmph2 = findHisto(v_tokens[1]); if (!tmph2) continue;
      TH1 *hres = (TH1 *)tmph1->Clone();
      //hres->Reset();

      if      (theline.find('-') != string::npos) hres->Add(tmph2,-1.0);
      else if (theline.find('+') != string::npos) hres->Add(tmph2);
      else if (theline.find('*') != string::npos) hres->Multiply(tmph2);
      //else if (theline.find('/') != string::npos) hres->Divide(tmph1,tmph2,1.0,1.0,"C");
      else if (theline.find('/') != string::npos) hres->Divide(tmph2);

      h1 = (TH1 *)hres;
      wTH1 *wh = new wTH1(h1);
      v_histos.push_back(pair<string,wTH1 *>(*hid,wh));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key.find("sum") != string::npos) {
    //------------------------------
      Tokenize(value,v_tokens,",");
      if (v_tokens.size() < 2) {
	cerr << "expect comma-separated list of at least two histo specs to sum! ";
	cerr << theline << endl;
	continue;
      }
      TH1 *firstone = (TH1 *)findHisto(v_tokens[0]);
      if (!firstone) exit(-1);
      h1 = (TH1 *)firstone->Clone();
      if (key == "weightsum")
	h1->SetBit(TH1::kIsAverage);  // <========== Addends also have to have this set.
      for (unsigned i=1; i<v_tokens.size(); i++) {
	TH1 *addend = (TH1 *)findHisto(v_tokens[i]);
	if (!addend) exit(-1);
	h1->Add(addend,1.0);
      }
      wTH1 *wh = new wTH1(h1);
      v_histos.push_back(pair<string,wTH1 *>(*hid,wh));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "integright") {  // sweep from low-high x-bins and integrate to the right
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      TH1 *tmph1 = findHisto(value,"histo operand must be defined before math ops");
      if (!tmph1) exit(-1);
      h1 = (TH1 *)IntegrateRight(tmph1,skipbinatx);
      wTH1 *wh = new wTH1(h1);
      v_histos.push_back(pair<string,wTH1 *>(*hid,wh));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    //------------------------------
    } else if (key == "integleft") {  // sweep from low-high x-bins and integrate to the left
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      TH1 *tmph1 = findHisto(value,"histo operand must be defined before math ops");
      if (!tmph1) exit(-1);
      h1 = (TH1 *)IntegrateLeft(tmph1);
      wTH1 *wh = new wTH1(h1);
      v_histos.push_back(pair<string,wTH1 *>(*hid,wh));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));

    } else if (key == "skipbinatx")
      skipbinatx = str2flt(value);

    //------------------------------
    else if (key == "projectx") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string binspec = value; // range of bins to project

      projectX(binspec,*hid,v_histos);

    //------------------------------
    } else if (key == "projecty") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string binspec = value; // range of bins to project

      projectY(binspec,*hid,v_histos);

    //------------------------------
    } else if (key == "projectyx") {
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined, " << hid << endl; continue;
      }

      string binspec = value; // range of bins to project

      projectYX(binspec,*hid,v_histos);
    }
    //------------------------------
    else if (key.find("slice") != string::npos) { // [xy]slice{mean|rms|chi2}
    //------------------------------
      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      string binspec = value; // range of bins to project

      sliceHistos(key,binspec,*hid,v_histos);

    //------------------------------
    } else if (key == "x2Dize") {  // means to take a collection of 1d plots and make a
    //------------------------------  2d plot by laying them sequentially along the x-axis

      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }
      Tokenize(value,v_tokens,",");
      if (v_tokens.size()<2) {
	cerr << "expect comma-separated list of at least two histo ids to 2D-ize!";
	cerr << theline << endl;
	continue;
      }

      x2DizeHistos(v_tokens,*hid,v_histos);

    //------------------------------
    } else if (key == "catmulti")    {  // concatenate multiple histograms
    //------------------------------

      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (v_histos.size()) {
	cerr << "histo already defined" << endl; continue;
      }

      wTH1 *wh = catMultiHist(*hid,value);
      v_histos.push_back(pair<string,wTH1 *>(*hid,wh));
      glmap_id2histo.insert(pair<string,wTH1 *>(*hid,wh));
      
    //------------------------------
    } else if (key == "statsPerBin") {  // for use with rebinVar1d, needs to be defined 1st
    //------------------------------

      statsPerBin = str2flt(value);

    //------------------------------
    } else if (key == "binrange")    {  // optionally use with rebinVar1d, needs to be defined 1st
    //------------------------------

      binrange = value;

    //------------------------------
    } else if (key == "rebinVar1d") {  // rebin the input histo with roughly even stats/bin
    //------------------------------

      if (!hid) {
	cerr << "id key must be defined first in the section" << endl; continue;
      }
      if (h1) {
	cerr << "histo already defined" << endl; continue;
      }

      Tokenize(value,v_tokens,",");
      if (!v_tokens.size()) {
	cerr << "ERROR: expect comma-separated list of histo ids";
	cerr << theline << endl;
	continue;
      }

      rebinVariable1D(v_tokens,binrange,statsPerBin,*hid,v_histos);

    } else if (!v_histos.size()) {
      cerr << "an operation key must be defined before key " << key << endl;
      break;
    }

    else {
      processCommonHistoParams(key,value,v_histos);
    }
  } // while getline loop

  return (v_histos.size());
}                                                 // processHmathSection

//======================================================================
