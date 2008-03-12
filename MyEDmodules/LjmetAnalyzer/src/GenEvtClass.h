#ifndef _GENEVTCLASS_H
#define _GENEVTCLASS_H

#include <vector>
#include <map>

#include "MyEDmodules/LjmetAnalyzer/src/myParticleRecord.h"
#include "MyEDmodules/LjmetAnalyzer/src/pdgid.h"

#include "CLHEP/HepMC/GenEvent.h"
#include "CLHEP/HepMC/GenVertex.h"
#include "SimDataFormats/HepMCProduct/interface/HepMCProduct.h"
#include "DataFormats/Candidate/interface/Candidate.h"

//======================================================================
/** \class GenEvtClass specification
      
$Date: 2008/03/10 16:07:18 $
$Revision: 1.1 $
\author P. Dudero - Minnesota
*/
class GenEvtClass {
public:
  GenEvtClass(bool verbosity);
  ~GenEvtClass();

  int          numClasses() const;
  std::string  classDescr(int classnum) const;

  int          getElectronCount() const { return myprec_->count(electron); }
  int          getQuarkCount()    const { return myprec_->quarkcount(); }

  int          classifyEvent(const HepMC::GenEvent& MCEvt);
  int          classifyEvent(const reco::CandidateCollection& genParticles);

private:
  // internal methods
  void loadPdgidStrings(void);
  std::string pdgidStr(int pdgid);
  int recurseTree(HepMC::GenParticle *p, int depth);
  void printTree(HepMC::GenParticle *p, int depth);

  // user-configurable parameters
  bool              verbosity_;

  // internal members
  myParticleRecord *myprec_;
  std::map<int,std::string> m_pdgidstr_;
  std::map<int,HepMC::GenParticle *> m_pMap_;
};

#endif // _GENEVTCLASS_H
