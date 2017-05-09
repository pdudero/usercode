#include "TGaxis.h"
#include "THStack.h"
#include "TArrayD.h"
#include "drawStandardTexts.C"

//======================================================================

void drawInPad(wPad_t *wp, wTH1& myHisto,bool firstInPad,
	       const string& altdrawopt="")
{
  wp->vp->cd();

  wp->vp->SetFillColor(wp->fillcolor);

  gStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);

  if (!gl_verbose) myHisto.ShutUpAlready();

  if (firstInPad) {
    if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
    if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
    if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
    if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);

    altdrawopt.size() ? myHisto.Draw(altdrawopt) : myHisto.Draw();

    // Now we can set the axis attributes and range:
    myHisto.SetAxes();
  }
  else {
    if (!myHisto.statsAreOn()) altdrawopt.size() ?
				 myHisto.Draw(altdrawopt+ " same") :
				 myHisto.DrawSame();
    else                       altdrawopt.size() ?
				 myHisto.Draw(altdrawopt+ " sames") :
				 myHisto.DrawSames();
  }

  myHisto.DrawFits("same");

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
void drawInPad(wPad_t *wp, T *obj,const string& indrawopt, bool firstInPad=false)
{
  wp->vp->cd();

  wp->vp->SetFillColor(wp->fillcolor);

  gStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);

  if (firstInPad) {
    if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
    if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
    if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
    if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);
  }

  TString drawopt = indrawopt;
  if (!firstInPad) 
    drawopt += " SAME";

  if (gl_verbose)
    cout<<"Drawing "<<obj->GetName()<<" with option(s) "<<drawopt<<endl;

  obj->Draw(drawopt);


  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void drawInPad(wPad_t *wp, wStack_t *ws, bool firstInPad,
	       const string& drawopt="")
{
  wp->vp->cd();

  wp->vp->SetFillColor(wp->fillcolor);

  gStyle->cd();

  wp->vp->SetLogx(wp->logx);
  wp->vp->SetLogy(wp->logy);
  wp->vp->SetLogz(wp->logz);

  if (firstInPad) {
    if (wp->topmargin)    wp->vp->SetTopMargin   (wp->topmargin);
    if (wp->bottommargin) wp->vp->SetBottomMargin(wp->bottommargin);
    if (wp->rightmargin)  wp->vp->SetRightMargin (wp->rightmargin);
    if (wp->leftmargin)   wp->vp->SetLeftMargin  (wp->leftmargin);

    // The sum histogram is used to set up the plot itself, far easier
    // this way than messing with the histogram internal to the stack,
    // and allows for fits on the sum.
    //
    ws->sum->Draw(drawopt);
    wp->vp->Update();
  } else {
    if (!strstr(drawopt.c_str(),"nostack")) {
      if (!ws->sum->statsAreOn()) drawopt.size() ?
				  ws->sum->Draw(drawopt+ " same") :
				  ws->sum->DrawSame();
      else {                      drawopt.size() ?
				  ws->sum->Draw(drawopt+ " sames") :
				  ws->sum->DrawSames();
      }
    }
  }

  if (gl_verbose)
    cout << "Drawing stack with option AH" << endl;

  ws->stack->Draw("SAME HIST");

  assert(gPad->GetFrame());

  ws->sum->DrawFits("same");  wp->vp->Update();
  ws->sum->DrawStats();       wp->vp->Update();

  //ws->sum->Draw("AXIG SAME");
  wp->vp->RedrawAxis();
  wp->vp->RedrawAxis("g same");
  wp->vp->Update();
}                                                           // drawInPad

//======================================================================

void saveCanvas2File(wCanvas_t *wc, const string& namefmt)
{
  // use first file read in for rootfilepath

  string datafile;
  map<string,TFile*>::const_iterator it = glmap_id2rootfile.begin();
  if (it != glmap_id2rootfile.end())
    datafile = it->first;
  
  string picfilename = buildStringFromSpecifiers(namefmt,
						 wc->title,
						 datafile);

  cout << "saving to..." << picfilename << endl;
  wc->c1->SaveAs(picfilename.c_str());
}                                                     // saveCanvas2File

//======================================================================

unsigned assignObjects2Multipad(canvasSet_t& cs) // returns total number of occupied pads
{
  wCanvas_t *wc0       = cs.canvases[0];
  unsigned npadspercan = wc0->npadsx*wc0->npadsy;
  unsigned npadsall    = cs.ncanvases*npadspercan;

  // Note: wci can't be wCanvas_t& because apparently filling the
  //       vector messes up the reference to the first element!
  //
  wCanvas_t wci(*wc0); // doesn't copy member "pads"
  wCanvas_t *wc = wc0;  // Divvy up the pads among multiple canvases if so specified

  unsigned ipad=0,ipad2start=0; // ipad=global pad index, not the Apple product!
  unsigned m=0;                 // m=multipad index

  // A multipad potentially references multiple sets of graphical objects
  // (histograms, graphs, etc.) to overlay. The objects within a set
  // are plotted in sequential pads, but another set of objects
  // assigned to the same multipad are overlaid sequentially on the
  // previous set. Ideally each set assigned to the same multipad
  // contains the same number of objects, but not necessarily.
  // Multiple multipads can exist; their collection of object sets are
  // assigned sequentially to ranges of available pads defined in the
  // layout section.
  //
  for (m=0; m<wc0->multipads.size(); m++) {
    wPad_t  *mp = wc0->multipads[m];

    if (gl_verbose) {
      cout << "multipad "<<m<<" has "<<mp->histo_ids.size()<<" histo set(s), ";
      cout << mp->altyh_ids.size()<<" alt y-axis histo set(s), ";
      cout << mp->graph_ids.size()<<" graph set(s) and ";
      cout << mp->altyg_ids.size()<<" alt y-axis graph set(s)"<<endl;
    }

    // figure out how many pads this multipad spans, taking max of the referenced object sets
    unsigned npads4mp=0;
    for (unsigned i=0; ; i++) {
      unsigned npads4i=0;
      if (i<mp->histo_ids.size()) {
	std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(mp->histo_ids[i]);
	if (it==glmap_mobj2size.end()) {
	  if (findHisto(mp->histo_ids[i]),"")
	    npads4i = 1;
	} else {
	  npads4i = it->second;
	}
      }
      if (i<mp->altyh_ids.size()) {
	std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(mp->altyh_ids[i]);
	if (it==glmap_mobj2size.end()) {
	  if (findHisto(mp->altyh_ids[i]),"")
	    npads4i = 1;
	} else {
	  npads4i = it->second;
	}
      }
      if (i<mp->graph_ids.size()) {
	std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(mp->graph_ids[i]);
	if (it==glmap_mobj2size.end()) {
	  if (findGraph(mp->graph_ids[i]),"")
	    npads4i = 1;
	} else {
	  npads4mp = it->second;
	}
      }
      if (i<mp->altyg_ids.size()) {
	std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(mp->altyg_ids[i]);
	if (it==glmap_mobj2size.end()) {
	  if (findGraph(mp->altyg_ids[i]),"")
	    npads4i = 1;
	} else {
	  npads4mp = it->second;
	}
      }
      if (i<mp->tf1_ids.size()) {
	std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(mp->tf1_ids[i]);
	if (it==glmap_mobj2size.end()) {
	  if (findTF1(mp->tf1_ids[i]))
	    npads4i = 1;
	} else {
	  npads4mp = it->second;
	}
      }
      if (!npads4i) break;
      npads4mp = std::max(npads4mp,npads4i);
    }

    npads4mp = std::min(npads4mp,npadsall-ipad2start);

    if (!npads4mp) continue;

    // Plot all objects for this multipad that can be plotted within
    // the span of assigned pads

    // Create the pads first
    for (ipad=ipad2start; ipad<ipad2start+npads4mp; ipad++) {

      unsigned   i  =  ipad % npadspercan;      // index to current pad in canvas
      unsigned cnum = (ipad / npadspercan) + 1; // current canvas number

      if (!i) { // first pad in new canvas
	if (cnum > cs.canvases.size()) {
	  if (gl_verbose)
	    cout << "making new canvas" << endl;
	  wc = new wCanvas_t(wci);
	  cs.canvases.push_back(wc);
	  wc->title = cs.title + "_" + int2str(cnum);
	  wc->pads.clear();
	  wc->latex_ids = wc0->latex_ids;
	}
      }

      wPad_t *wp = new wPad_t(*(mp));
      wp->histo_ids.clear(); 
      wp->altyh_ids.clear(); 
      wp->graph_ids.clear(); 
      wp->altyg_ids.clear(); 
      wp->tf1_ids.clear(); 
      if (i) wp->legid.clear(); // don't automatically propagate legend to all pads

      wc->pads.push_back(wp);
    }

    if (gl_verbose)
      cout << npads4mp << " pads created for multipad " << m << endl;

    unsigned f,g,h,j,k,l,n,t,y,z; /* f=tf1 index in current tf1 set
				     g=graph index in current graph set
				     h=histo index in current histo set
				     j=graph id (set) index
				     k=histo_id (set) index
				     l=altyhisto index
				     n=altygraph index
				     t=tf1_id (set) index
				     y=histoindex in current altyhisto set
				     z=graphindex in current altygraph set */
    
    f=g=h=j=k=l=n=t=y=z=0;

    // Now assign
    for (ipad=ipad2start; ipad<ipad2start+npads4mp; ipad++) {

      unsigned   i  =  ipad % npadspercan;      // index to current pad in canvas
      unsigned cnum = (ipad / npadspercan) + 1; // current canvas number

      wc = cs.canvases[cnum-1];
      wPad_t *wp = wc->pads[i];

      bool foundhisto=false;
      if (k < mp->histo_ids.size()) {
	string hid=mp->histo_ids[k];

	if (!h && gl_verbose) {
	  cout<<"Assigning histo/histo set "<<hid<<" to pads ";
	  cout<<ipad2start<<"-"<<ipad2start+npads4mp-1<<endl;
	}

	if (!h && findHisto(hid, "switching to histo set"))
	  foundhisto=true;
	else {
	  hid = hid +"_"+int2str(h++);
	  if (findHisto(hid, "hit the end of histo set"))
	    foundhisto=true;
	}
	if (foundhisto) {
	  // now we associate histogram sets with the pad set
	  wp->histo_ids.push_back(hid);
	  if (gl_verbose)
	    cout << "histo " << hid << " assigned to pad " << ipad << endl;
	}
      }

      // altyhistos:
      if (l < mp->altyh_ids.size()) {
	string ahid=mp->altyh_ids[l];
	bool foundaltyh=false;
	if (!y && findHisto(ahid, "switching to set"))
	  foundaltyh=true;
	else {
	  ahid = ahid +"_"+int2str(y++);
	  if (findHisto(ahid, "hit the end of histo set"))
	    foundaltyh=true;
	}
	if (foundaltyh) {
	  // now we associate histogram sets with the pad set
	  wp->altyh_ids.push_back(ahid);
	  if (gl_verbose)
	    cout << "alty histo " << ahid << " assigned to pad " << ipad << endl;
	}
      }

      // graphs:
      bool foundgraph=false;

      if (j < mp->graph_ids.size()) {
	string gid=mp->graph_ids[j];

	if (!g && findGraph(gid, "switching to set"))
	  foundgraph=true;
	else {
	  gid = gid +"_"+int2str(g++);
	  if (findGraph(gid, "hit the end of graph set"))
	    foundgraph=true;
	}
	if (foundgraph) {
	  // now we associate graph sets with the pad set
	  wp->graph_ids.push_back(gid);
	  if (gl_verbose)
	    cout << "graph " << gid << " assigned to pad " << ipad << endl;
	}
      }

      if (n < mp->altyg_ids.size()) {
	string gid=mp->altyg_ids[j];
	bool foundaltyg=false;

	if (!z && findGraph(gid, "switching to set"))
	  foundaltyg=true;
	else {
	  gid = gid +"_"+int2str(z++);
	  if (findGraph(gid, "hit the end of graph set"))
	    foundaltyg=true;
	}
	if (foundaltyg) {
	  // now we associate graph sets with the pad set
	  wp->altyg_ids.push_back(gid);
	  if (gl_verbose)
	    cout << "alty graph " << gid << " assigned to pad " << ipad << endl;
	}
      }

      // tf1s:
      bool foundtf1=false;

      if (t < mp->tf1_ids.size()) {
	string fid=mp->tf1_ids[t];

	if (!f && findTF1(fid))
	  foundtf1=true;
	else {
	  fid = fid +"_"+int2str(f++);
	  if (findTF1(fid))
	    foundtf1=true;
	}
	if (foundtf1) {
	  // now we associate tf1 sets with the pad set
	  wp->tf1_ids.push_back(fid);
	  if (gl_verbose)
	    cout << "TF1 " << fid << " assigned to pad " << ipad << endl;
	}
      }

      if (!foundgraph && !foundhisto && !foundtf1) { // reset to next graph/histo/tf1 ids
	ipad=ipad2start;
	f=g=h=y=z=0;
	++j;  ++k; ++l; ++n, ++t;
	if (j == mp->graph_ids.size() &&
	    k == mp->histo_ids.size() &&
	    t == mp->tf1_ids.size()) break;
      }
    } // pad loop

    ipad2start += npads4mp;

  } // multipads loop

  return std::min(npadsall,ipad);
}                                              // assignObjects2Multipad

//======================================================================

unsigned assignPads2Canvases(canvasSet_t& cs)
{
  wCanvas_t *wc0    = cs.canvases[0];
  unsigned npads    = wc0->npadsx*wc0->npadsy;
  unsigned npadsall = cs.ncanvases*npads;

  // Note: wci can't be wCanvas_t& because apparently filling the
  //       vector messes up the reference to the first element!
  //
  wCanvas_t wci(*wc0); // doesn't copy member "pads"
  wCanvas_t *wc = wc0;

  // Divvy up the pads among multiple canvases if so specified

  unsigned ipad=0;                      // ipad=global pad index 
  for (;ipad<wc0->pads.size(); ipad++) {
    unsigned   i  =  ipad % npads;      // index to current pad in canvas
    unsigned cnum = (ipad / npads) + 1; // current canvas number

    if (cnum > cs.ncanvases) break;

    if ((i==0) && (cnum > cs.canvases.size())) {
      if (gl_verbose)
	cout << "making new canvas" << endl;
      wc = new wCanvas_t(wci);
      cs.canvases.push_back(wc);
      wc->title = cs.title + "_" + int2str(cnum);
      wc->pads.clear();
      wc->latex_ids = wc0->latex_ids;
    }

    wc->pads.push_back(wc0->pads[ipad]);
  }

  return std::min(npadsall,ipad+1);
}                                                 // assignPads2Canvases

//======================================================================

void  drawPlots(canvasSet_t& cs,bool savePlots2file)
{
  wCanvas_t *wc0 = cs.canvases[0];
  unsigned npads = wc0->npadsx*wc0->npadsy;
  unsigned npadsall = cs.ncanvases*npads;

  if (!npadsall) {
    if (gl_verbose) cout << "Nothing to draw, guess I'm done." << endl;
    return; // no pads to draw on.

  } else if (!wc0->pads.size()) {

    /********************************************************
     * CHECK MULTIPAD OPTION, ASSIGN HISTOS TO PADS/CANVASES
     ********************************************************/

    if (wc0->multipads.size()) {
      npadsall = assignObjects2Multipad(cs);
    } else {
      cerr << "npads>0, but no pad specs supplied, exiting." << endl;
      return; // no pads to draw on.
    }
  } else if (cs.ncanvases>1) {
    npadsall = assignPads2Canvases(cs);
  } else {
    npadsall = std::min(npadsall,(unsigned)wc0->pads.size());
  }

  wc0->c1->cd();

  if (gl_verbose)
    cout << "Drawing on " << npadsall << " pad(s)" << endl;

  wLegend_t *wleg = NULL;

  /***************************************************
   * LOOP OVER PADS...
   ***************************************************/

  //vector<vector<string> >::const_iterator it;
  for (unsigned ipad = 0; ipad< npadsall; ipad++) {

    if (gl_verbose) cout << "Drawing pad# " << ipad+1 << endl;

    unsigned ipadc =  ipad % npads;
    unsigned cnum  = (ipad / npads) + 1;

    wCanvas_t *wc = cs.canvases[cnum-1];

    if (!ipadc) {
      if (cnum-1) { // first canvas already created
	wc->c1 = new TCanvas(wc->title.c_str(),wc->title.c_str(),
			     wc->padxdim*wc->npadsx,
			     wc->padydim*wc->npadsy);
	float left = wc->leftmargin;
	float bot  = wc->bottommargin;
	float rght = 1-wc->rightmargin;
	float top  = 1-wc->topmargin;
	wc->motherpad = new TPad("mother","",left,bot,rght,top);
	wc->c1->SetFillColor(wc->fillcolor);
	wc->motherpad->SetFillColor(wc->fillcolor);
	wc->motherpad->Draw();
	wc->motherpad->cd();
	wc->motherpad->Divide(wc->npadsx,wc->npadsy); // , wc->padxmargin,wc->padymargin);
      }

      /***************************************************
       * CHECK FOR LATEX OBJECTS ON THE CANVAS
       ***************************************************/
      
      wc->c1->cd();
      for (unsigned j=0; j<wc->latex_ids.size(); j++) {
	string& lid = wc->latex_ids[j];
	map<string,TLatex *>::const_iterator it = glmap_id2latex.find(lid);
	if (it == glmap_id2latex.end()) {
	  cerr << "ERROR: latex id " << lid << " never defined in layout" << endl;
	  exit (-1);
	}
	TLatex *ltx = it->second;
	ltx->Draw();
	wc->c1->Update();
      }
    }

    wPad_t *& wp = wc->pads[ipadc];
    wp->vp = wc->motherpad->cd(ipadc+1);

    if ((wp->xupndc > wp->xlowndc) &&
	(wp->yupndc > wp->ylowndc)   )
      wp->vp->SetPad(wp->xlowndc,wp->ylowndc,wp->xupndc,wp->yupndc);

    if (!wp->histo_ids.size() &&
	!wp->stack_ids.size() &&
	!wp->graph_ids.size() &&
	!wp->tf1_ids.size() &&
	!wp->macro_ids.size()) {
      cerr << "ERROR: pad #" << ipadc+1 << " has no ids defined for it";
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
     * Check for existence of a legend, create it
     ***************************************************/
    bool drawlegend = false;

    if (wp->legid.size()) {
      map<string,wLegend_t *>::const_iterator it=glmap_id2legend.find(wp->legid);
      if (it != glmap_id2legend.end()) {
	drawlegend = true;
	wleg = it->second;
      } else {
	cerr << "ERROR: legend id " << wp->legid;
	cerr << " never defined in layout" << endl;
      }
    }
#if 0
    else {
      // Maybe gPad already *has* a legend from macros...
      TPave *testing = (TPave *)gPad->GetPrimitive("TPave");
      if (testing &&
	  !strcmp(testing->IsA()->GetName(),"TLegend")) {
	TLegend *pullTheOtherOne = (TLegend *)testing;
	if (gl_verbose) cout << "Found legend from macro" << endl;
	wleg = new wLegend_t();
	wleg->leg = pullTheOtherOne;
	drawlegend = true;
      }
    }
#endif

    /***************************************************
     * LOOP OVER STACKS DEFINED FOR PAD...
     ***************************************************/

    if (wp->stack_ids.size()) {
      wStack_t *ws=NULL;
      for (unsigned j = 0; j < wp->stack_ids.size(); j++) {
	string& sid = wp->stack_ids[j];
	map<string,wStack_t *>::const_iterator it = glmap_id2stack.find(sid);
	if (it == glmap_id2stack.end()) {
	  cerr<<"ERROR: stack id "<<sid<<" never defined in layout"<<endl;
	  exit (-1);
	}

	bool firstInPad = !j;

	ws = it->second;
	if (!ws) {
	  cerr<<"find returned NULL stack pointer for "<<sid<<endl; continue;
	}

	// Add the histos in the stack to any legend that exists

	//
	if (drawlegend) {
	  for (size_t i=0; i<ws->v_histos.size(); i++) {
	    wTH1 *wh = ws->v_histos[i];
	    wh->ApplySavedStyle();
	    if(wh->GetLegendEntry().size())
	      wh->Add2Legend(wleg->leg);
	  }
	}

	string drawopt("");
	if (ws->sum->GetDrawOption().size()) {
	  drawopt = ws->sum->GetDrawOption();
	  if (gl_verbose)
	    cout<<"drawopt stored with histo = "<<drawopt<<endl;
	}

	drawInPad(wp, ws, firstInPad, drawopt);

	wp->vp->Update();
      }
    } // stack loop

    /***************************************************
     * LOOP OVER HISTOS DEFINED FOR PAD...
     ***************************************************/

    int nhist = 0;
    for (unsigned j = 0; j < wp->histo_ids.size(); j++) {
      //cout << j << endl;
      string hid = wp->histo_ids[j];
      map<string,wTH1 *>::const_iterator it = glmap_id2histo.find(hid);
      if (it == glmap_id2histo.end()) {
	// look for a multihist all of which will be assigned to the existing pad..
	std::map<string,unsigned>::const_iterator mit=glmap_mobj2size.find(hid);
	if (mit==glmap_mobj2size.end()) {
	  cerr<<"ERROR: histo id "<<hid<<" never defined in layout"<<endl;
	  exit (-1);
	} else {
	  nhist = mit->second;
	  //cout << nhist << endl;
	  for (int h=0; h<nhist; h++) {
	    string hidi = hid +"_"+int2str(h);
	    //cout << hidi << endl;
	    wp->histo_ids.push_back(hidi);
	  }
	  continue;
	}
      }

      wTH1 *myHisto = it->second;
      
      if (myHisto) {
	bool firstInPad = (!j && !wp->stack_ids.size()) || (j==1 && nhist);
	if (gl_verbose) {
	  cout << "Drawing " << hid << " => ";
	  cout << myHisto->histo()->GetName() << endl;
	  cout << "firstInPad = " << firstInPad << endl;
	}
	drawInPad(wp,*myHisto,firstInPad);

	if (drawlegend && myHisto->GetLegendEntry().size()) {
	  if (wleg->drawoption.size()) myHisto->SetDrawOption(wleg->drawoption);
	  myHisto->Add2Legend(wleg->leg);
	}
	if (myHisto->statsAreOn()) {
	  myHisto->DrawStats();
	  wp->vp->Update();
	}

	myHisto->ApplySavedStyle();
	wp->vp->Update();
      }
    } // histos loop

    /***************************************************
     * LOOP OVER GRAPHS DEFINED FOR PAD...
     ***************************************************/
#if 0
    TMultiGraph *mg;
    if (graph_ids.size())
      mg = new TMultiGraph();
#endif
    size_t size=wp->graph_ids.size();
    for( unsigned j = 0; j < size; j++ ) {
      string gid = wp->graph_ids[j];
      
      wGraph_t *wg = findGraph(gid, "switching to set");
      if (!wg) {
	// handle case where a multi-object is assigned to a single pad
	std::map<string,unsigned>::const_iterator it=glmap_mobj2size.find(gid);
	if (it!=glmap_mobj2size.end()) {
	  wp->graph_ids.erase(wp->graph_ids.begin()+j);
	  for (size_t k=0; k<it->second; k++) {
	    string gidi = gid +"_"+int2str(k);
	    wp->graph_ids.push_back(gidi);
	  }
	  size = wp->graph_ids.size();
	  j=-1; // reset counter and start over.
	  continue;
	} else {
	  cerr << "Can't find multigraph with id " << gid << endl;
	  exit(-1);
	}
      }

      bool firstInPad = !j && !wp->histo_ids.size();

      if( wg && wg->gr ) {

	string drawopt = wg->drawopt;

	if( firstInPad && wg->gr->InheritsFrom("TGraph") )
	  drawopt += string("A"); // no histos drawn, need to draw the frame ourselves.

	// "pre-draw" in order to define the plot elements
	wg->gr->Draw(drawopt.c_str());
	
	if (firstInPad) {
	  // Now we can set the axis attributes and range:
	  wg->gr->GetXaxis()->ImportAttributes(wg->xax);
	  wg->gr->GetYaxis()->ImportAttributes(wg->yax);

	  //cout << wg->xax->GetXmin() << " " << wg->xax->GetXmax() << endl;
	  if( wg->xax->GetXmax()>wg->xax->GetXmin() )
	    wg->gr->GetXaxis()->SetLimits(wg->xax->GetXmin(),wg->xax->GetXmax());
	  if( wg->yax->GetXmax()>wg->yax->GetXmin() )
	    wg->gr->GetYaxis()->SetRangeUser(wg->yax->GetXmin(),wg->yax->GetXmax());
	}
	// draw for good
	drawInPad<TGraph>(wp,wg->gr,drawopt.c_str(),firstInPad);

	wp->vp->Update();
	if( wg->fitfn ) 
	  wg->gr->Fit(wg->fitfn);
	if( drawlegend && wg->leglabel.size() ) {
	  wleg->leg->AddEntry(wg->gr,wg->leglabel.c_str(),wg->legdrawopt.c_str());
	  cout << "adding entry " << wg->leglabel << " with option " << endl ;
	}
      }
      if( wg && wg->gr2d ) {
	string drawopt = wg->drawopt;

	if (wg->contours) {
	  //cout << "setting contours for graph " << gid << endl; wg->contours->Print();
	  wg->gr2d->GetHistogram()->SetContour(wg->contours->GetNoElements(),
					       wg->contours->GetMatrixArray());
	  // In order to control the contour lines, we have to draw the 2D first to generate
	  // the contours and then pick them out of the "contours" object as separate graph
	  // elements
	  if (ci_find(drawopt,"LIST") != string::npos) {
	    new TCanvas("dummy","dummy",100,100);
	    wg->gr2d->Draw(drawopt.c_str());
	  } else
	    drawInPad<TGraph2D>(wp,wg->gr2d,drawopt.c_str(),firstInPad);
	}
	else
	  drawInPad<TGraph2D>(wp,wg->gr2d,drawopt.c_str(),firstInPad);

	gPad->Update();

	if (firstInPad) {
	  // Now we can set the axis attributes and range:
	  wg->gr2d->GetXaxis()->ImportAttributes(wg->xax);
	  wg->gr2d->GetYaxis()->ImportAttributes(wg->yax);
	  wg->gr2d->GetZaxis()->ImportAttributes(wg->zax);

	  //cout << wg->xax->GetXmin() << " " << wg->xax->GetXmax() << endl;
	  if( wg->xax->GetXmax()>wg->xax->GetXmin() )
	    wg->gr2d->GetXaxis()->SetLimits(wg->xax->GetXmin(),wg->xax->GetXmax());
	  if( wg->yax->GetXmax()>wg->yax->GetXmin() )
	    wg->gr2d->GetYaxis()->SetRangeUser(wg->yax->GetXmin(),wg->yax->GetXmax());
	  if( wg->zax->GetXmax()>wg->zax->GetXmin() )
	    wg->gr2d->GetZaxis()->SetRangeUser(wg->zax->GetXmin(),wg->zax->GetXmax());
	}
	
	if (wg->contours && ci_find(drawopt,"LIST") != string::npos) {
	  cout << wg->contours->GetNoElements() << " contour(s)" << endl;
	  for (int i=0; i<wg->contours->GetNoElements(); i++) {
	    TList *contLevel = (TList*)wg->gr2d->GetContourList((*(wg->contours))(i));
	    if (!contLevel) {
	      cerr << "Could not find any contours at level ";
	      cerr << (*(wg->contours))(i);
	      cerr << " for graph " << gid << endl;
	      wp->vp->cd();
	      continue;
	    }
	    TGraph *curv = (TGraph*)contLevel->First();
	    for (int k=0; k<contLevel->GetSize(); k++) {
	      cout << "contour #" << k << endl;
	      curv->SetLineStyle   (wg->lstyle);
	      curv->SetLineColor   (wg->lcolor);
	      cout << "setting line color " << wg->lcolor << endl;
	      curv->SetLineWidth   (wg->lwidth);
	      if (firstInPad && k==0) {
		drawInPad<TGraph>(wp,curv,"AL",true);
		curv->GetXaxis()->ImportAttributes(wg->xax);
		curv->GetYaxis()->ImportAttributes(wg->yax);
		cout << "Setting x axis limits " << wg->xax->GetXmin() << " " << wg->xax->GetXmax() << endl;
		if( wg->xax->GetXmax()>wg->xax->GetXmin() ) {
		  curv->GetXaxis()->SetLimits(wg->xax->GetXmin(),wg->xax->GetXmax());
		}
		if( wg->yax->GetXmax()>wg->yax->GetXmin() )
		  curv->GetYaxis()->SetRangeUser(wg->yax->GetXmin(),wg->yax->GetXmax());
	      } else
		drawInPad<TGraph>(wp,curv,"L",false);
	      if( drawlegend && wg->leglabel.size() && !k )
		wleg->leg->AddEntry(curv,wg->leglabel.c_str(),wg->legdrawopt.c_str());
	      gPad->Modified();
	      gPad->Update();
	      firstInPad = false;
	      curv = (TGraph*)contLevel->After(curv);
	    }
	  }
	}

	wp->vp->Modified();
	wp->vp->Update();
	if( drawlegend && wg->leglabel.size() && !wg->contours )
	  wleg->leg->AddEntry(wg->gr2d,wg->leglabel.c_str(),wg->legdrawopt.c_str());
      }
    } // graph loop

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
      TH1 *h = myHisto->histo();

      if (!j) {
	//scale second set of histos to the pad coordinates
	rightmin = h->GetMinimum();
	rightmax = 1.1*h->GetMaximum();
	scale    = gPad->GetUymax()/rightmax;
      }
      TH1 *scaled=(TH1 *)h->Clone(Form("%s_%d",h->GetName(),ipad));

      scaled->Scale(scale);
      scaled->Draw("same");
   
      //draw an axis on the right side
      TGaxis *axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(),
				gPad->GetUxmax(), gPad->GetUymax(),
				rightmin,rightmax,505,"+L");
      axis->Draw();
      gPad->Update();
      if (drawlegend && myHisto->GetLegendEntry().size()) {
	if (wleg->drawoption.size()) myHisto->SetDrawOption(wleg->drawoption);
	myHisto->Add2Legend(wleg->leg);
      }
    }

    /***************************************************
     * LOOP OVER GRAPHS DEFINED FOR ALTERNATE Y-AXIS
     ***************************************************/

    scale=0.0;

    for( unsigned j = 0; j < wp->altyg_ids.size(); j++ ) {
      string& gid = wp->altyg_ids[j];

      wGraph_t *wg   = findGraph(gid);

      string drawopt = wg->drawopt;

      if( wg && wg->gr ) {
	// scale the graph y axis points:
	TVectorD yaxpts = TVectorD(wg->gr->GetN(),wg->gr->GetY());

	if (!j) {
	  //scale second set of histos to the pad coordinates
	  rightmin = yaxpts.Min();
	  rightmax = 1.1*yaxpts.Max();
	  scale    = gPad->GetUymax()/rightmax;
	  //cout << "rightmin,rightmax,scale = "<< rightmin<<" "<<rightmax<<" "<<scale<<endl;
	}

	yaxpts *= scale;

	TGraph *scaled=new TGraph(wg->gr->GetN(),
				  wg->gr->GetX(),
				  yaxpts.GetMatrixArray());
	//scaled->Print();
	scaled->Draw("same");
   
	//draw an axis on the right side
	TGaxis *axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(),
				  gPad->GetUxmax(), gPad->GetUymax(),
				  rightmin,rightmax,505,"+L");
	axis->Draw();
	if (drawlegend && wg->leglabel.size()) {
	  wleg->leg->AddEntry(wg->gr,
			      wg->leglabel.c_str(),
			      wg->legdrawopt.c_str());
	}
	gPad->Update();
      }
    } // altyg loop

    /***************************************************
     * LOOP OVER TF1S DEFINED FOR PAD...
     ***************************************************/

    for( unsigned j = 0; j < wp->tf1_ids.size(); j++ ) {
      string& fid = wp->tf1_ids[j];
      
      wTF1_t *wtf1   = findwTF1(fid);

      bool firstInPad = !j && !wp->histo_ids.size() && !wp->graph_ids.size();

      drawInPad<TF1>(wp,wtf1->tf1,"",firstInPad);

      if (drawlegend && wtf1->leglabel.size()) {
	cout << "adding tf1 legend entry with legdrawopt " << wtf1->legdrawopt << endl;
	wleg->leg->AddEntry(wtf1->tf1,
			    wtf1->leglabel.c_str(),
			    wtf1->legdrawopt.c_str());
      }

      wp->vp->Update();

    } // tf1 loop

    /***************************************************
     * Check for external macros to run on the pad
     ***************************************************/
    for (size_t i=0; i<wp->macro_ids.size(); i++) {
      map<string,string>::const_iterator it = glmap_objpath2id.find(wp->macro_ids[i]);
      if (it != glmap_objpath2id.end()) {
	string path = it->second;
	int error;
	cout << "Executing macro " << it->first << " --> " << path << endl;

	if (path.find_first_of(';')) {// execute a function out of .so file
	  vector<string> v_tok;
	  Tokenize(path, v_tok, ";");
	  if (v_tok.size() != 2) {
	    cerr << "ERROR: invalid format loadlib=funcname_C.so;funcname" << endl;
	    exit(-1);
	  }
	  gSystem->Load(v_tok[0].c_str());
	  Func_t f = gSystem->DynFindSymbol(v_tok[0].c_str(), v_tok[1].c_str());
	  if (!f) {
	    cerr << "ERROR: couldn't find << " << v_tok[1] << " inside " << v_tok[0] << endl;
	    exit(-1);
	  }
	  (*f)();
	} else {
	  gROOT->Macro(path.c_str(), &error, kTRUE); // update current pad
	  if (error) {
	    static const char *errorstr[] = {
	      "kNoError","kRecoverable","kDangerous","kFatal","kProcessing" };
	    cerr << "ERROR: error returned from macro: " << errorstr[error] << endl;
	  }
	}
      } else {
	cerr << "ERROR: macro id " << wp->macro_ids[i];
	cerr << " never defined in layout" << endl;
      }
    }

    /***************************************************
     * LOOP OVER LINES DEFINED FOR PAD...
     ***************************************************/

    for( unsigned j = 0; j < wp->line_ids.size(); j++ ) {
      string drawopt("L");
      string& lid = wp->line_ids[j];
      map<string,TLine *>::const_iterator it2 = glmap_id2line.find(lid);
      if (it2 == glmap_id2line.end()) {
	cerr << "ERROR: line id " << lid << " never defined in layout" << endl;
	exit (-1);
      }

      TLine *line = it2->second;

      if (!j && !wp->histo_ids.size() && !wp->macro_ids.size())
	drawopt += string("A"); // no histos drawn, need to draw the frame ourselves.

      if (line) {
	drawInPad<TLine>(wp,line,drawopt.c_str());
	//if (drawlegend)
	//wleg->leg->AddEntry(line,lid.c_str(),"L");
      }
    }

    /***************************************************
     * LOOP OVER BOXES DEFINED FOR PAD...
     ***************************************************/

    for (unsigned j = 0; j < wp->box_ids.size(); j++) {
      string drawopt("L");
      string& bid = wp->box_ids[j];
      map<string,TBox *>::const_iterator it2 = glmap_id2box.find(bid);
      if (it2 == glmap_id2box.end()) {
	cerr << "ERROR: box id " << bid << " never defined in layout" << endl;
	exit (-1);
      }

      TBox *box = it2->second;

      if (box) {
	drawInPad<TBox>(wp,box,drawopt.c_str());
      }
    }

    wp->vp->SetGridx(wp->gridx);
    wp->vp->SetGridy(wp->gridy);

    /***************************************************
     * Draw the legend
     ***************************************************/

    if (drawlegend) {
      wleg->leg->Draw("same");
      wp->vp->Update();
    }

    /***************************************************
     * Draw each latex/label object
     ***************************************************/
    
    for (unsigned j=0; j<wp->latex_ids.size(); j++) {
      string& lid = wp->latex_ids[j];
      map<string,TLatex *>::const_iterator it2 = glmap_id2latex.find(lid);
      if (it2 == glmap_id2latex.end()) {
	cerr << "ERROR: latex id " << lid << " never defined in layout" << endl;
	exit (-1);
      }
      if (gl_verbose) cout << "Drawing latex object " << lid << endl;
      TLatex *ltx = it2->second;
      ltx->Draw();
      wp->vp->Update();
    }

    for (unsigned j = 0; j < wp->label_ids.size(); j++) {
      string& lid = wp->label_ids[j];
      map<string,wLabel_t *>::const_iterator it2 = glmap_id2label.find(lid);
      if (it2 == glmap_id2label.end()) {
	cerr << "ERROR: label id " << lid << " never defined in layout" << endl;
	exit (-1);
      }
      if (gl_verbose) cout << "Drawing label object " << lid << endl;
      wLabel_t *wlab = it2->second;
      drawStandardText(wlab->text, wlab->x1ndc, wlab->y1ndc,-1,-1,wlab->textsize);

      wp->vp->Update();
    }

    wc->c1->Update();

  } // pad loop

  //prdFixOverlay();

  if (savePlots2file) {
    wc0 = cs.canvases[0];
    if (!wc0->savenamefmts.size())  // define a default
      wc0->savenamefmts.push_back("%F_%C.png");
    for (size_t i=0; i<cs.canvases.size(); i++) {
      wCanvas_t *wc = cs.canvases[i];
      wc->c1->cd();
      for (size_t j=0; j<wc0->savenamefmts.size(); j++)
	saveCanvas2File(wc,wc0->savenamefmts[j]);
    }
  }
}                                                           // drawPlots

//======================================================================
