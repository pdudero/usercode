{
  gROOT->SetStyle("Plain");

  TH1 *metunfilt; _file0->GetObject("myanunfilt/Hcal/h_caloMet_Met", metunfilt);
  TH1 *metfilt6;  _file0->GetObject("myanfilt6/Hcal/h_caloMet_Met", metfilt6);
  TH1 *metfilt10; _file0->GetObject("myanfilt10/Hcal/h_caloMet_Met", metfilt10);

  TCanvas *c1 = new TCanvas("c1","MET",1200,600);

  c1->Divide(2);
  c1->cd(2);
  gPad->SetRightMargin(0.03);

  metfilt6->Draw();
  metfilt6->SetLineWidth(2);
  metfilt6->SetLineColor(kRed);
  metfilt6->GetXaxis()->SetTitle("Calo MET, no HF (GeV)");
  gPad->SetLogy(1);

  metfilt10->SetLineWidth(2);
  metfilt10->SetLineColor(kBlue);
  metfilt10->Draw("same");

  metunfilt->SetLineWidth(2);
  metunfilt->Draw("same");

  TLegend *leg1 = new TLegend(0.20,0.68,0.95,0.83);

  leg1->AddEntry(metunfilt,"Unfiltered CaloMet","l");
  leg1->AddEntry(metfilt6,"Time-filtered CaloMet, 6ns window","l");
  leg1->AddEntry(metfilt10,"Time-filtered CaloMet, 10ns window","l");
  leg1->Draw();

  TH1 *rhtimeunfilt; _file0->GetObject("myanunfilt/Hcal/RHTimesFlt", rhtimeunfilt);
  TH1 *rhtimefilt6;  _file0->GetObject("myanfilt6/Hcal/RHTimesFlt", rhtimefilt6);
  TH1 *rhtimefilt10; _file0->GetObject("myanfilt10/Hcal/RHTimesFlt", rhtimefilt10);

  c1->cd(1);
  gPad->SetRightMargin(0.03);
  gPad->SetTopMargin(0.11);

  rhtimeunfilt->Draw();
  rhtimeunfilt->SetLineWidth(2);
  rhtimeunfilt->GetXaxis()->SetTitle("Reco Hit Times (ns)");
  rhtimeunfilt->GetXaxis()->CenterTitle();

  rhtimefilt6->SetLineWidth(2);
  rhtimefilt6->SetLineColor(kRed);
  rhtimefilt6->Draw("same");

  rhtimefilt10->SetLineWidth(2);
  rhtimefilt10->SetLineColor(kBlue);
  rhtimefilt10->Draw("same");

  TLegend *leg2 = new TLegend(0.20,0.68,0.95,0.83);

  leg2->AddEntry(rhtimeunfilt,"Unfiltered RecHit Times","l");
  leg2->AddEntry(rhtimefilt6,"Time-filtered RecHit Times, 6ns window","l");
  leg2->AddEntry(rhtimefilt10,"Time-filtered RecHit Times, 10ns window","l");

  leg2->Draw();

  c1->SaveAs("timesAndMET.png");
}


