// Current issues:
//
#include "MyEDmodules/LjmetAnalyzer/src/GenEvtClass.h"
#include "MyEDmodules/LjmetAnalyzer/src/pdgid.h"

#include <iostream>

//======================================================================

using namespace std;
//using namespace HepMC;
using namespace reco;

//======================================================================
void GenEvtClass::loadPdgidStrings(void)
{

// QUARKS
  m_pdgidstr_[1] = "dq";
  m_pdgidstr_[2] = "uq";
  m_pdgidstr_[3] = "sq";
  m_pdgidstr_[4] = "cq";
  m_pdgidstr_[5] = "bq";
  m_pdgidstr_[6] = "tq";

  m_pdgidstr_[-1] = "/dq";
  m_pdgidstr_[-2] = "/uq";
  m_pdgidstr_[-3] = "/sq";
  m_pdgidstr_[-4] = "/cq";
  m_pdgidstr_[-5] = "/bq";
  m_pdgidstr_[-6] = "/tq";

//LEPTONS
  m_pdgidstr_[ 11] = "e-";
  m_pdgidstr_[-11] = "e+";
  m_pdgidstr_[ 12] = "nue";
  m_pdgidstr_[-12] = "/nue";
  m_pdgidstr_[ 13] = "mu-";
  m_pdgidstr_[-13] = "mu+";
  m_pdgidstr_[ 14] = "numu";
  m_pdgidstr_[-14] = "/numu";
  m_pdgidstr_[ 15] = "tau-";
  m_pdgidstr_[-15] = "tau+";
  m_pdgidstr_[ 16] = "nutau";
  m_pdgidstr_[-16] = "/nutau";

// GAUGE AND HIGGS BOSONS
  m_pdgidstr_[ 21] = "g";
  m_pdgidstr_[ 22] = "ph";
  m_pdgidstr_[ 23] = "Z0";
  m_pdgidstr_[ 24] = "W+";
  m_pdgidstr_[-24] = "W-";
  m_pdgidstr_[ 25] = "H";

// DIQUARKS
  m_pdgidstr_[1103] = "dd1";
  m_pdgidstr_[2101] = "ud0";
  m_pdgidstr_[2103] = "ud1";
  m_pdgidstr_[2203] = "uu1";
  m_pdgidstr_[3101] = "sd0";
  m_pdgidstr_[3103] = "sd1";
  m_pdgidstr_[3201] = "su0";
  m_pdgidstr_[3203] = "su1";
  m_pdgidstr_[3303] = "ss1";
  m_pdgidstr_[4101] = "cd0";
  m_pdgidstr_[4103] = "cd1";
  m_pdgidstr_[4201] = "cu0";
  m_pdgidstr_[4203] = "cu1";
  m_pdgidstr_[4301] = "cs0";
  m_pdgidstr_[4303] = "cs1";
  m_pdgidstr_[4403] = "cc1";
  m_pdgidstr_[5101] = "bd0";
  m_pdgidstr_[5103] = "bd1";
  m_pdgidstr_[5201] = "bu0";
  m_pdgidstr_[5203] = "bu1";
  m_pdgidstr_[5301] = "bs0";
  m_pdgidstr_[5303] = "bs1";
  m_pdgidstr_[5401] = "bc0";
  m_pdgidstr_[5403] = "bc1";
  m_pdgidstr_[5503] = "bb0";

// SUSY PARTICLES
  m_pdgidstr_[ 1000001] = "sdqL";
  m_pdgidstr_[-1000001] = "/sdqL";
  m_pdgidstr_[ 1000002] = "suqL";
  m_pdgidstr_[-1000002] = "/suqL";
  m_pdgidstr_[ 1000003] = "ssqL";
  m_pdgidstr_[-1000003] = "/ssqL";
  m_pdgidstr_[ 1000004] = "scqL";
  m_pdgidstr_[-1000004] = "/scqL";
  m_pdgidstr_[ 1000005] = "sbq1";
  m_pdgidstr_[-1000005] = "/sbq1";
  m_pdgidstr_[ 1000006] = "stq1";
  m_pdgidstr_[-1000006] = "/stq1";
  m_pdgidstr_[ 1000011] = "seL";
  m_pdgidstr_[-1000011] = "/seL";
  m_pdgidstr_[ 1000012] = "snue";
  m_pdgidstr_[-1000012] = "/snue";
  m_pdgidstr_[ 1000013] = "smuL";
  m_pdgidstr_[-1000013] = "/smuL";
  m_pdgidstr_[ 1000014] = "snumu";
  m_pdgidstr_[-1000014] = "/snumu";
  m_pdgidstr_[ 1000015] = "stau1";
  m_pdgidstr_[-1000015] = "/stau1";
  m_pdgidstr_[ 1000016] = "snutau";
  m_pdgidstr_[-1000016] = "/snutau";
  m_pdgidstr_[ 2000001] = "sdqR";
  m_pdgidstr_[-2000001] = "/sdqR";
  m_pdgidstr_[ 2000002] = "suqR";
  m_pdgidstr_[-2000002] = "/suqR";
  m_pdgidstr_[ 2000003] = "ssqR";
  m_pdgidstr_[-2000003] = "/ssqR";
  m_pdgidstr_[ 2000004] = "scqR";
  m_pdgidstr_[-2000004] = "/scqR";
  m_pdgidstr_[ 2000005] = "sbq2";
  m_pdgidstr_[-2000005] = "/sbq2";
  m_pdgidstr_[ 2000006] = "stq2";
  m_pdgidstr_[-2000006] = "/stq2";
  m_pdgidstr_[ 2000011] = "seR";
  m_pdgidstr_[-2000011] = "/seR";
  m_pdgidstr_[ 2000013] = "smuR";
  m_pdgidstr_[-2000013] = "/smuR";
  m_pdgidstr_[ 2000015] = "stau2";
  m_pdgidstr_[-2000015] = "/stau2";
  m_pdgidstr_[ 1000021] = "gluino";
  m_pdgidstr_[ 1000022] = "X01";
  m_pdgidstr_[ 1000023] = "X02";
  m_pdgidstr_[ 1000024] = "X+1";
  m_pdgidstr_[ 1000025] = "X03";
  m_pdgidstr_[ 1000035] = "X04";
  m_pdgidstr_[ 1000037] = "X+2";
  m_pdgidstr_[ 1000039] = "~G";

// LIGHT I=1 MESONS
  m_pdgidstr_[ 111] = "pi0";
  m_pdgidstr_[ 211] = "pi+";
  m_pdgidstr_[-211] = "pi-";
  m_pdgidstr_[ 113] = "rho770";
  m_pdgidstr_[ 213] = "rho770+";
  m_pdgidstr_[-213] = "rho770-";
  m_pdgidstr_[ 115] = "a2";
  m_pdgidstr_[ 215] = "a2+";
  m_pdgidstr_[ 117] = "rho3";
  m_pdgidstr_[ 217] = "rho3+";
  m_pdgidstr_[-217] = "rho3-";
  m_pdgidstr_[ 119] = "a4";
  m_pdgidstr_[ 219] = "a4+";
  m_pdgidstr_[-219] = "a4-";

// LIGHT I=0 MESONS
  m_pdgidstr_[ 221] = "eta";
  m_pdgidstr_[ 331] = "eta'";
  m_pdgidstr_[ 223] = "w782";
  m_pdgidstr_[ 333] = "phi1020";

// STRANGE MESONS
  m_pdgidstr_[ 130] = "K0";
  m_pdgidstr_[ 310] = "K0";
  m_pdgidstr_[ 311] = "K0";
  m_pdgidstr_[-311] = "/K0";
  m_pdgidstr_[ 313] = "K*0";
  m_pdgidstr_[-313] = "/K*0";
  m_pdgidstr_[ 321] = "K+";
  m_pdgidstr_[-321] = "K-";
  m_pdgidstr_[ 323] = "K*+";
  m_pdgidstr_[-323] = "/K*+";

// CHARMED MESONS
  m_pdgidstr_[ 411] = "D+";
  m_pdgidstr_[-411] = "D-";
  m_pdgidstr_[ 413] = "D*+";
  m_pdgidstr_[-413] = "/D*+";
  m_pdgidstr_[ 421] = "D0";
  m_pdgidstr_[-421] = "/D0";
  m_pdgidstr_[ 423] = "D*0";
  m_pdgidstr_[-423] = "/D*0";

  m_pdgidstr_[ 511] = "B0";
  m_pdgidstr_[-511] = "/B0";
  m_pdgidstr_[ 521] = "B+";
  m_pdgidstr_[-521] = "B-";

// LIGHT BARYONS
  m_pdgidstr_[ 2212] = "P";
  m_pdgidstr_[-2212] = "/P";
  m_pdgidstr_[ 2112] = "N";
  m_pdgidstr_[-2112] = "/N";
  m_pdgidstr_[ 2224] = "Delta++";
  m_pdgidstr_[-2224] = "/Delta++";
  m_pdgidstr_[ 2214] = "Delta+";
  m_pdgidstr_[-2214] = "/Delta+";
  m_pdgidstr_[ 2114] = "Delta0";
  m_pdgidstr_[-2114] = "/Delta0";
  m_pdgidstr_[ 1114] = "Delta-";
  m_pdgidstr_[-1114] = "/Delta-";
}                                                    // loadPdgidStrings

//======================================================================

std::string GenEvtClass::pdgidStr(int pdgid) {
  std::map<int,std::string>::const_iterator it = m_pdgidstr_.find(pdgid);
  
  if (it == m_pdgidstr_.end()) {
    ostringstream ss;
    ss << pdgid;
    return ss.str();
  }

  return it->second;
}                                                            // pdgidStr

//======================================================================

GenEvtClass::GenEvtClass(bool verbosity) :
  verbosity_(verbosity)
{
  vector<int> pdgidvec;

  /****************************************
   * Initialize Generated Particle Record
   ****************************************/

  pdgidvec.push_back(downq);
  pdgidvec.push_back(upq);
  pdgidvec.push_back(strangeq);
  pdgidvec.push_back(charmq);
  pdgidvec.push_back(bottomq);
  pdgidvec.push_back(topq);

  pdgidvec.push_back(electron);
  pdgidvec.push_back(nu_e);
  pdgidvec.push_back(muminus);
  pdgidvec.push_back(nu_mu);
  pdgidvec.push_back(tauminus);
  pdgidvec.push_back(nu_tau);

  myprec_ = new myParticleRecord(pdgidvec);

  loadPdgidStrings();
}

//======================================================================

GenEvtClass::~GenEvtClass()
{
  delete myprec_;
}

//======================================================================

int GenEvtClass::numClasses() const
{
  return 6;
}

//======================================================================

std::string GenEvtClass::classDescr(int classnum) const
{
  switch(classnum) {
  case 0: return "unclassed";
  case 1: return "susy";
  case 2: return "ttbar";
  case 3: return "wtaunujets";
  case 4: return "wmunujets";
  case 5: return "wenujets";
  }

  return "other";
}

//======================================================================
//
void GenEvtClass::printTree(HepMC::GenParticle *p, int depth)
{
  int pdgid = abs(p->pdg_id());

  cout << setw(7) << pdgidStr(pdgid);

  if (!p->end_vertex() ||  (pdgid == 92)) {  cout << endl; return; }

  HepMC::GenVertex::particle_iterator child;
  HepMC::GenVertex::particle_iterator begin;
  begin = p->end_vertex ()->particles_begin (HepMC::children);
  for (child  = begin;
       child != p->end_vertex ()->particles_end   (HepMC::children); 
       ++child) {
    if (child != begin) {
      cout << "       ";
      for (int i=0; i<depth; i++) cout << "          ";
    }
    cout << " => ";
    printTree(*child, depth+1);
  }
}                                               // GenEvtClass:printTree

//======================================================================
// m_pMap_ keeps track of visited particles as we traverse the tree.
// Look for end vertices and count up final state particles.
// Classify the event according to the current particle.
// W's are special because we have to look for 3 leptonic decay channels.
// So we collect classification codes from child recursion calls and see
// if a particular lepton showed up amongst the children. Return abs(pdgid)
// until the classification is known, then return -1*class back up the
// recursion.
//
int GenEvtClass::recurseTree(HepMC::GenParticle *p, int depth)
{
  int evtclass = 0;

  std::map<int,HepMC::GenParticle *>::const_iterator it;
  it = m_pMap_.find(p->barcode());

  int pdgid   = abs(p->pdg_id());
  int barcode = p->barcode();

  if (it == m_pMap_.end())
    m_pMap_ [barcode] = p;            // register that we visited this particle
  else {
    if (verbosity_) cout << "double-linked barcode " << barcode << " pdgid " << pdgid <<  endl;
    return pdgid;                     // no need to traverse the same subtree twice!
  }

  if (pdgid == 92) return pdgid; // she's gonna hadronize, abort!

  if (!p->end_vertex()) {    // is this a final state particle?
    myprec_->add(pdgid);     // yes, count up final state particles.
    return pdgid;
  }

  // Recurse the children in all other cases to count final state particles
  //
  std::map<int, HepMC::GenParticle *> m_childretvals;

  HepMC::GenVertex::particle_iterator child;
  HepMC::GenVertex::particle_iterator begin;
  begin = p->end_vertex ()->particles_begin (HepMC::children);
  for (child  = begin;
       child != p->end_vertex ()->particles_end   (HepMC::children); 
       ++child) {

    int retval = recurseTree(*child, depth+1);
#if 0
    if (verbosity_)
      cout << "Depth " << depth+1 << ": retval = " << retval << endl;
#endif
    m_childretvals[retval] = (*child); // don't care if it overwrites
  }

  if (pdgid > 1000000) evtclass = -1;    // susy;
  if (pdgid == topq)   evtclass = -2;    // top

  if (evtclass) return evtclass;

  // No class determined yet, inspect the kid pdgids...
  // return highest classification from among the children...
  //
  for (int i=-1; i>=-5; i--)
    if (m_childretvals.find(i)  != m_childretvals.end()) {
      evtclass =  i;
      break;
    }
  
  if (evtclass) return evtclass;

  // W->lepton scenarios...
  //
  bool foundW = (pdgid == Wplus);

  if      (m_childretvals.find(tauminus) != m_childretvals.end()) pdgid = tauminus;
  else if (m_childretvals.find(muminus)  != m_childretvals.end()) pdgid = muminus;
  else if (m_childretvals.find(electron) != m_childretvals.end()) pdgid = electron;

  if (foundW) {
    if (pdgid == tauminus) return -3;   // W->tau+nu
    if (pdgid == muminus)  return -4;   // W->mu+nu
    if (pdgid == electron) return -5;   // W->e+nu
  }

  return depth ? pdgid : -1*numClasses();

}                                             // GenEvtClass:recurseTree

//======================================================================

int GenEvtClass::classifyEvent(const HepMC::GenEvent& genEvt)
{
  int eventclass = numClasses();

  int nParticles = genEvt.particles_size();
    
  if (verbosity_) {
    cout << "          Number of Raw Particles collected:         ";
    cout << nParticles << endl;
  }

  myprec_->clearcounts();
  m_pMap_.clear();

  HepMC::GenEvent::particle_const_iterator p;
  for (p  = genEvt.particles_begin ();
       p != genEvt.particles_end ();
       ++p) {
    if (!(*p)->production_vertex()) { // top of the tree
      if (verbosity_) printTree(*p,0);
      int evtclass = recurseTree(*p,0);
      evtclass *= -1;
      if ((evtclass) &&
	  (evtclass < eventclass))
	eventclass = evtclass;
    }
  }

  if (verbosity_)
    cout << "event class = " << eventclass << endl;

  if (verbosity_) {
    cout << "#e = "        << myprec_->count(electron);
    cout << ", #nu_e = "   << myprec_->count(nu_e);
    cout << ", #mu- = "    << myprec_->count(muminus);
    cout << ", #nu_mu = "  << myprec_->count(nu_mu);
    cout << ", #tau- = "   << myprec_->count(tauminus);
    cout << ", #nu_tau = " << myprec_->count(nu_tau);
    cout << ", #quark = "  << myprec_->quarkcount() << endl;
  }

  return eventclass;
}                                          // GenEvtClass::classifyEvent

//======================================================================

int GenEvtClass::classifyEvent(const reco::CandidateCollection& genParticles)
{
  bool foundW  = false;
  int evtclass = 0;
  int nParticles = genParticles.size();
    
  if (verbosity_) {
    cout << "          Number of Raw Particles collected:         ";
    cout << nParticles << endl;
  }

  myprec_->clearcounts();

  for( size_t i = 0; i < genParticles.size(); ++ i ) {
    const Candidate & p = genParticles[ i ];
    int       pdgid = abs(p.pdgId());
    unsigned int n  = p.numberOfDaughters();

    if (!evtclass) {
      if (pdgid > 1000000) evtclass = 1;    // susy;
      if (pdgid == topq)   evtclass = 2;    // top
      if (pdgid == Wplus)  foundW = true;
    }

    if (verbosity_)
      cout << "Parent is: " << pdgid << ", children are: ";
      
    for( size_t j = 0; j < n; ++ j ) {
      const Candidate * d = p.daughter( j );
      int dauId = d->pdgId();
      if (verbosity_) cout << setw(3) << dauId << ' ';

      if (foundW && !evtclass) {
	if (dauId == tauminus) evtclass = 3;
	else if (dauId == muminus)  evtclass = 4;
	else if (dauId == electron) evtclass = 5;
      }
    }
    if (!n) myprec_->add(pdgid);
    if (verbosity_) cout << endl;
  }

  if (verbosity_)
    cout << "event class = " << evtclass << endl;

  if (verbosity_ || !evtclass) {
    cout << "#e        = " << myprec_->count(electron);
    cout << ", #nu_e   = " << myprec_->count(nu_e);
    cout << ", #mu-    = " << myprec_->count(muminus);
    cout << ", #nu_mu  = " << myprec_->count(nu_mu);
    cout << ", #tau-   = " << myprec_->count(tauminus);
    cout << ", #nu_tau = " << myprec_->count(nu_tau);
    cout << ", #quark  = " << myprec_->quarkcount() << endl;
  }

  if (!evtclass) evtclass = numClasses();

  return evtclass;
}                                    // GenEvtClass::classifyEvent

//======================================================================
