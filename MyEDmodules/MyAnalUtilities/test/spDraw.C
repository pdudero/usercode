#include "TGaxis.h"
#include "THStack.h"
#include "TArrayD.h"
#include "drawStandardTexts.C"

//======================================================================

void drawInPad(wPad_t *wp, wTH1& myHisto,bool firstInPad,
	       const string& altdrawopt="")
{
  wp->vp->SetFillColor(wp->fillcolor);

  gStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);
  wp->vp->SetGridx(wp->gridx);
  wp->vp->SetGridy(wp->gridy);

  if (firstInPad) {
    if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
    if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
    if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
    if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);

    altdrawopt.size() ? myHisto.Draw(altdrawopt) : myHisto.Draw();
  }
  else {
    if (!myHisto.statsAreOn()) altdrawopt.size() ?
				 myHisto.Draw(altdrawopt+ " same") :
				 myHisto.DrawSame();
    else                       altdrawopt.size() ?
				 myHisto.Draw(altdrawopt+ " sames") :
				 myHisto.DrawSames();
  }

#if 0
  myHisto.histo()->Sumw2();
  TArrayD *sumw2 = myHisto.histo()->GetSumw2();
  cout << sumw2->GetSize() << endl;
  for (int i=0; i<sumw2->GetSize(); i++)
    printf ("%g ", (*sumw2)[i]);
  cout << endl;
#endif

  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

template<class T>
void drawInPad(wPad_t *wp, T *obj, const string& drawopt)
{
  wp->vp->SetFillColor(wp->fillcolor);

  gStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);

  cout<<"Drawing "<<obj->GetName()<<" with option(s) "<<drawopt<<endl;

  obj->Draw(drawopt.c_str());

  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void drawInPad(wPad_t *wp, THStack *stack)
{
  wp->vp->SetFillColor(wp->fillcolor);

  gStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);
  wp->vp->SetGridx(wp->gridx);
  wp->vp->SetGridy(wp->gridy);

  if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
  if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
  if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
  if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);

  // what a hassle. Root redraws the axes automatically for stacks,
  // and if the text sizes are different..., so have to find another
  // way to get the job done.
  //
  stack->Draw("AH");

  TH1 *grid = (TH1 *)stack->GetHistogram();
  if (grid) grid->Draw("AXIG SAME");
  else cout << "Problem getting stack histogram" << endl;

  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void saveCanvas2File(wCanvas_t *wc, const string& namefmt)
{
  string picfilename;
  size_t len = namefmt.length();
  size_t pos,pos0 = 0;

  do {
    // Format of output filename specified in "savenamefmt"
    // %F = first file read in
    // %C = configuration file name
    //
    pos = namefmt.find('%',pos0);
    if (pos == string::npos) {
      picfilename += namefmt.substr(pos0); // no more format codes, finish up
      break;
    } else if (pos>pos0) {
      picfilename+=namefmt.substr(pos0,pos-pos0);
    }
    // expand format codes
    if (pos != len-1) {  // make sure '%' wasn't the last character
      pos0=pos+1;
      switch (namefmt[pos0]) { 
      case 'C': picfilename += wc->title; break;
      case 'F': {
	string datafile;
	map<string,TFile*>::const_iterator it = glmap_id2rootfile.begin();
	if (it != glmap_id2rootfile.end())
	  datafile = it->first.substr(0,it->first.find_last_of('.'));
	picfilename += datafile;
      }	break;
      default:
	cout<<"Unrecognized format code %"<<namefmt[pos0]<<endl;
	break;
      }
      pos0++;
    }
  } while (pos0<len);

  cout << "saving to..." << picfilename << endl;
  wc->c1->SaveAs(picfilename.c_str());
}                                                     // saveCanvas2File

//======================================================================

void  drawPlots(canvasSet_t& cs,bool savePlots2file)
{
  wCanvas_t *wc0 = cs.canvases[0];
  unsigned npads = wc0->npadsx*wc0->npadsy;
  unsigned npadsall = cs.ncanvases*npads;

  if (!npads) {
    cout << "Nothing to draw, guess I'm done." << endl;
    return; // no pads to draw on.

  } else if (!wc0->pads.size()) {

    /********************************************************
     * CHECK MULTIPAD OPTION, ASSIGN HISTOS TO PADS/CANVASES
     ********************************************************/

    if (wc0->multipad) {
      // Note: wci can't be wCanvas_t& because apparently filling the
      //       vector messes up the reference to the first element!
      //
      wCanvas_t wci(*wc0); // doesn't copy member "pads"
      wPad_t    *mp = wc0->multipad;
      wCanvas_t *wc = wc0;

      // Divvy up the pads among multiple canvases if so specified

      unsigned j=0;                      // j=global pad index 
      for (unsigned h=0,k=0; ; j++) {    /* h=histo index in current histo multiset
					    k=multiset index */
	unsigned   i  =  j % npads;      // index to current pad in canvas
	unsigned cnum = (j / npads) + 1; // current canvas number

	if (cnum > cs.ncanvases) break;

	if ((i==0) && (cnum > cs.canvases.size())) {
	  cout << "making new canvas" << endl;
	  wc = new wCanvas_t(wci);
	  cs.canvases.push_back(wc);
	  wc->title = cs.title + "_" + int2str(cnum);
	  wc->pads.clear();
	}

	if (!h) cout<<"Assigning multiset "<<mp->histo_ids[k]<<" to pads."<<endl;

	string multihist1 = mp->histo_ids[k]+"_"+int2str(h++);
	if (findHisto(multihist1, "hit the end of histo multiset")) {
	  // now we associate histogram sets with the pad set
	  wPad_t *wp = new wPad_t(*(mp));
	  wp->histo_ids.clear();
	  wp->histo_ids.push_back(multihist1);
	  wc->pads.push_back(wp);
	} else {
	  j--; // have to back up one...
	  h=0;
	  if (++k == mp->histo_ids.size()) break;
	}
      }
      npadsall = min(npadsall,j);
    } else {
      cout << "npads>0, but no pad specs supplied, exiting." << endl;
      return; // no pads to draw on.
    }
  } else {
    npadsall = min(npadsall,wc0->pads.size());
  }

  wc0->c1->cd();

  /***************************************************
   * CHECK FOR LATEX OBJECTS ON THE CANVAS
   ***************************************************/

  for (unsigned j=0; j<wc0->latex_ids.size(); j++) {
    string& lid = wc0->latex_ids[j];
    map<string,TLatex *>::const_iterator it = glmap_id2latex.find(lid);
    if (it == glmap_id2latex.end()) {
      cerr << "ERROR: latex id " << lid << " never defined in layout" << endl;
      exit (-1);
    }
    TLatex *ltx = it->second;
    ltx->Draw();
    wc0->c1->Update();
  }

  cout << "Drawing on " << npadsall << " pad(s)" << endl;

  wLegend_t *wl = NULL;

  /***************************************************
   * LOOP OVER PADS...
   ***************************************************/

  vector<vector<string> >::const_iterator it;
  for (unsigned j = 0; j< npadsall; j++) {

    cout << "Drawing pad# " << j << endl;

    unsigned   i  =  j % npads;
    unsigned cnum = (j / npads) + 1;

    wCanvas_t *wc = cs.canvases[cnum-1];

    if (!i && (cnum-1)) { // first canvas already created
      wc->c1 = new TCanvas(wc->title.c_str(),wc->title.c_str(),
			  wc->padxdim*wc->npadsx,
			  wc->padydim*wc->npadsy);
      float left = wc->leftmargin;
      float bot  = wc->bottommargin;
      float rhgt = 1-wc->rightmargin;
      float top  = 1-wc->topmargin;
      wc->motherpad = new TPad("mother","",left,bot,rhgt,top);
      wc->c1->SetFillColor(wc->fillcolor);
      wc->motherpad->SetFillColor(wc->fillcolor);
      wc->motherpad->Draw();
      wc->motherpad->cd();
      wc->motherpad->Divide(wc->npadsx,wc->npadsy);
			   // , wc->padxmargin,wc->padymargin);
    }

    wPad_t *& wp = wc->pads[i];
    wp->vp = wc->motherpad->cd(i+1);

    if (!wp->histo_ids.size() &&
	!wp->stack_ids.size() &&
	!wp->graph_ids.size() &&
	!wp->macro_ids.size()) {
      cerr << "ERROR: pad #" << i+1 << " has no ids defined for it";
      cerr << ", continuing to the next" << endl;
      continue;
    }


#if 0
    /***************************************************
     * Draw the frame first:
     * (Fix up frame since it can't be auto-scaled:)
     ***************************************************/
    string& hid0 = wp->histo_ids[0];
    map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid0);
    if (it == glmap_id2histo.end()) {
      cerr << "ERROR: id0 " << hid0 << " never defined in layout" << endl;
      return;
    }
    wTH1 *myHisto = it->second;
    TH1  *h = myHisto->histo();

    if (wp->hframe->histo()->GetXaxis()->GetXmin() <=
	wp->hframe->histo()->GetXaxis()->GetXmax())
      wp->hframe->histo()->GetXaxis()->SetRangeUser(h->GetXaxis()->GetXmin(),
						    h->GetXaxis()->GetXmax());
    if (wp->hframe->histo()->GetYaxis()->GetXmin() <= 
	wp->hframe->histo()->GetYaxis()->GetXmax())
      wp->hframe->histo()->GetYaxis()->SetRangeUser(h->GetYaxis()->GetXmin(),
						    h->GetYaxis()->GetXmax());

    wp->hframe->SetStats(0);
    //wp->hframe->Draw("AXIS");
#endif

    /***************************************************
     * Check for external macros to run on the pad
     ***************************************************/
    for (size_t i=0; i<wp->macro_ids.size(); i++) {
      map<string,string>::const_iterator it = glmap_objpaths2id.find(wp->macro_ids[i]);
      if (it != glmap_objpaths2id.end()) {
	string path = it->second;
	int error;
	gROOT->Macro(path.c_str(), &error, kTRUE); // update current pad
	if (error) {
	  static const char *errorstr[] = {
	    "kNoError","kRecoverable","kDangerous","kFatal","kProcessing" };
	  cerr << "ERROR: error returned from macro: " << errorstr[error] << endl;
	}
      } else {
	cerr << "ERROR: macro id " << wp->macro_ids[i];
	cerr << " never defined in layout" << endl;
      }
    }
    /***************************************************
     * Check for existence of a legend, create it
     ***************************************************/
    bool drawlegend = false;

    if (wp->legid.size()) {
      map<string,wLegend_t *>::const_iterator it=glmap_id2legend.find(wp->legid);
      if (it != glmap_id2legend.end()) {
	drawlegend = true;
	wl = it->second;
      } else {
	cerr << "ERROR: legend id " << wp->legid;
	cerr << " never defined in layout" << endl;
      }
    } else {
      // Maybe gPad already *has* a legend from macros...
      TPave *testing = (TPave *)gPad->GetPrimitive("TPave");
      if (testing &&
	  !strcmp(testing->IsA()->GetName(),"TLegend")) {
	TLegend *pullTheOtherOne = (TLegend *)testing;
	cout << "Found legend from macro" << endl;
	wl = new wLegend_t();
	wl->leg = pullTheOtherOne;
	drawlegend = true;
      }
    }

    /***************************************************
     * Check for stacked histos:
     ***************************************************/

    if (wp->stack_ids.size()) {
      THStack *stack=NULL;
      for (unsigned j = 0; j < wp->stack_ids.size(); j++) {
	string& hid = wp->stack_ids[j];
	map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
	if (it == glmap_id2histo.end()) {
	  cerr << "ERROR: histo id " << hid << " never defined in layout" << endl;
	  exit (-1);
	}

	wTH1 *myHisto = it->second;
	if (!myHisto) cerr<< "find returned NULL pointer for " << hid << endl;

	if (!j) {
	  // use first histogram to set quantities
	  stack=new THStack(myHisto->histo()->GetName(),
			    myHisto->histo()->GetTitle());
	  stack->SetMaximum(myHisto->histo()->GetYaxis()->GetXmax());
	  stack->SetMinimum(myHisto->histo()->GetYaxis()->GetXmin());
	}

	stack->Add(myHisto->histo());

	if (drawlegend && myHisto->GetLegendEntry().size()) {
	  if (wl->drawoption.size()) myHisto->SetDrawOption(wl->drawoption);
	  myHisto->Add2Legend(wl->leg);
	}
	myHisto->ApplySavedStyle();
	wp->vp->Update();
      }
      cout << "Drawing stack" << endl;
      drawInPad(wp,stack);
    }

    /***************************************************
     * LOOP OVER HISTOS DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->histo_ids.size(); j++) {
      string& hid = wp->histo_ids[j];
      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
      if (it == glmap_id2histo.end()) {
	cerr << "ERROR: histo id " << hid << " never defined in layout" << endl;
	exit (-1);
      }

      wTH1 *myHisto = it->second;
      
      if (myHisto) {
	cout << "Drawing " << hid << " => ";
	cout << myHisto->histo()->GetName() << endl;
	bool firstInPad = !(j||wp->stack_ids.size());
	cout << "firstInPad = " << firstInPad << endl;
	drawInPad(wp,*myHisto,firstInPad);
	if (myHisto->statsAreOn()) {
	  cout << "OptStat = " <<  gStyle->GetOptStat() << endl;
	  myHisto->DrawStats();
	  wp->vp->Update();
	}
	myHisto->DrawFits();
	if (drawlegend && myHisto->GetLegendEntry().size()) {
	  if (wl->drawoption.size()) myHisto->SetDrawOption(wl->drawoption);
	  myHisto->Add2Legend(wl->leg);
	}
	myHisto->ApplySavedStyle();
	wp->vp->Update();
      }
    } // histos loop

    /***************************************************
     * LOOP OVER HISTOS DEFINED FOR ALTERNATE Y-AXIS
     ***************************************************/

    Float_t rightmax=0.0,rightmin=0.0;
    Float_t scale=0.0;
    for (unsigned j = 0; j < wp->altyh_ids.size(); j++) {
      string& hid = wp->altyh_ids[j];
      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
      if (it == glmap_id2histo.end()) {
	cerr << "ERROR: histo id " << hid << " never defined in layout" << endl;
	exit (-1);
      }

      wTH1 *myHisto = it->second;

      if (!j) {
	//scale second set of histos to the pad coordinates
	rightmin = myHisto->histo()->GetMinimum();
	rightmax = 1.1*myHisto->histo()->GetMaximum();
	scale    = gPad->GetUymax()/rightmax;
      }
      myHisto->histo()->Scale(scale);
      myHisto->histo()->Draw("same");
   
      //draw an axis on the right side
      TGaxis *axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(),
				gPad->GetUxmax(), gPad->GetUymax(),
				rightmin,rightmax,510,"+L");
      axis->Draw();
      gPad->Update();
      if (drawlegend && myHisto->GetLegendEntry().size()) {
	if (wl->drawoption.size()) myHisto->SetDrawOption(wl->drawoption);
	myHisto->Add2Legend(wl->leg);
      }
    }

    /***************************************************
     * LOOP OVER GRAPHS DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->graph_ids.size(); j++) {
      //string drawopt("CP");
      string drawopt("L");
      string& gid = wp->graph_ids[j];
      map<string,wGraph_t *>::const_iterator it   = glmap_id2graph.find(gid);
      map<string,TGraph2D *>::const_iterator it2d = glmap_id2graph2d.find(gid);
      wGraph_t *wg   = NULL;
      TGraph2D *gr2d = NULL;

      if (it == glmap_id2graph.end()) {
	if (it2d == glmap_id2graph2d.end()) {
	  cerr << "ERROR: graph id " << gid << " never defined in layout" << endl;
	  exit (-1);
	} else {
	  gr2d = it2d->second;
	}
      } else {
	wg = it->second;
      }


      //if (!j && !wp->histo_ids.size() && !wp->macro_ids.size())
      //drawopt += string("A"); // no histos drawn, need to draw the frame ourselves.

      if (wg) {
	drawInPad<TGraph>(wp,wg->gr,drawopt.c_str());
	wp->vp->Update();
	if (drawlegend)
	  wl->leg->AddEntry(wg->gr,gid.c_str(),"L");
      }
      if (gr2d) {
	drawInPad<TGraph2D>(wp,gr2d,drawopt.c_str());
	wp->vp->Update();
	if (drawlegend)
	  wl->leg->AddEntry(wg->gr,wg->leglabel.c_str(),"L");
      }
    }

    /***************************************************
     * LOOP OVER LINES DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->line_ids.size(); j++) {
      string drawopt("L");
      string& lid = wp->line_ids[j];
      map<string,TLine *>::const_iterator it = glmap_id2line.find(lid);
      if (it == glmap_id2line.end()) {
	cerr << "ERROR: line id " << lid << " never defined in layout" << endl;
	exit (-1);
      }

      TLine *line = it->second;

      if (!j && !wp->histo_ids.size() && !wp->macro_ids.size())
	drawopt += string("A"); // no histos drawn, need to draw the frame ourselves.

      if (line) {
	drawInPad<TLine>(wp,line,drawopt.c_str());
	//if (drawlegend)
	//wl->leg->AddEntry(line,lid.c_str(),"L");
      }
    }

    /***************************************************
     * LOOP OVER BOXES DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->box_ids.size(); j++) {
      string drawopt("L");
      string& bid = wp->box_ids[j];
      map<string,TBox *>::const_iterator it = glmap_id2box.find(bid);
      if (it == glmap_id2box.end()) {
	cerr << "ERROR: box id " << bid << " never defined in layout" << endl;
	exit (-1);
      }

      TBox *box = it->second;

      if (box) {
	drawInPad<TBox>(wp,box,drawopt.c_str());
      }
    }

    /***************************************************
     * Draw the legend
     ***************************************************/

    if (drawlegend) {
      wl->leg->Draw("same");
      wp->vp->Update();
    }

    /***************************************************
     * Draw each label
     ***************************************************/

    for (unsigned j = 0; j < wp->label_ids.size(); j++) {
      string& lid = wp->label_ids[j];
      map<string,wLabel_t *>::const_iterator it = glmap_id2label.find(lid);
      if (it == glmap_id2label.end()) {
	cerr << "ERROR: label id " << lid << " never defined in layout" << endl;
	exit (-1);
      }
      wLabel_t *wl = it->second;
      drawStandardText(wl->text, wl->x1ndc, wl->y1ndc,-1,-1,wl->textsize);

      wp->vp->Update();
    }
    wc->c1->Update();

  } // pad loop

  //prdFixOverlay();

  if (savePlots2file) {
    wCanvas_t *wc0 = cs.canvases[0];
    if (!wc0->savenamefmts.size())  // define a default
      wc0->savenamefmts.push_back("%F_%C.png");
    for (size_t i=0; i<cs.canvases.size(); i++) {
      wCanvas_t *wc = cs.canvases[i];
      wc->c1->cd();
      for (size_t i=0; i<wc0->savenamefmts.size(); i++)
	saveCanvas2File(wc,wc0->savenamefmts[i]);
    }
  }
}                                                           // drawPlots

//======================================================================
