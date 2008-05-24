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
      
$Date: 2008/03/12 14:21:28 $
$Revision: 1.1.1.1 $
\author P. Dudero - Minnesota
*/
class GenEvtClass {
public:
  GenEvtClass(bool verbosity);
  ~GenEvtClass();

  typedef enum {
    eOTHERSAMPLE,
    eSUSY,
    eTTBAR,
    eWJETS
  }
  EnumSample_t;

  typedef enum {
    eNOLEPTONS,
    e1ELEC,
    e1ELECpLEPTONS,
    e1MUorTAU,
    e2orMOREMUorTAU
  }
  EnumSignature_t;

  std::string  classDescr(int classnum) const;
  std::string  signDescr(EnumSignature_t sig) const;

  int numClasses() const;
  int numSignatureTypes() const;

  myParticleRecord *newParticleRecord(void);

  inline int  getElectronCount()   const { return myprec_->count(electron); }
  inline int  getNonELeptonCount() const { return
					     (myprec_->count(muminus) +
					      myprec_->count(tauminus)); }
  inline int  getQuarkCount()      const { return myprec_->quarkcount(); }

  void classifyEvent(const HepMC::GenEvent& genEvt,
		     EnumSample_t&    sampleclass,
		     EnumSignature_t& signatureclass);

  int classifyEvent(const reco::CandidateCollection& genParticles);

private:
  // internal methods
  void loadPdgidStrings(void);
  std::string pdgidStr(int pdgid);

  EnumSignature_t detSignatureClass(void);

  int  recurseTree(HepMC::GenParticle *p, int depth, myParticleRecord *prec);
  void printTree  (HepMC::GenParticle *p, int depth);
  void printCounts(myParticleRecord& prec);

  // user-configurable parameters
  bool              verbosity_;

  // internal members
  myParticleRecord *myprec_;
  std::map<int,std::string> m_pdgidstr_;
  std::map<int,HepMC::GenParticle *> m_pMap_;
};

#endif // _GENEVTCLASS_H
