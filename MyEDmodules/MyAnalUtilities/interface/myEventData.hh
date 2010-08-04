#ifndef _MYEDMODULESMYANALUTILITIES
#define _MYEDMODULESMYANALUTILITIES

// -*- C++ -*-
//
// Package:    myEventData
// Class:      myEventData
// 
/**\class myEventData myEventData.cc MyEDmodules/myEventData/src/myEventData.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Phillip Russell DUDERO
//         Created:  Tue Sep  9 13:11:09 CEST 2008
// $Id: myEventData.hh,v 1.11 2010/06/14 13:11:21 dudero Exp $
//
//

// system include files
#include <vector>

// user include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Provenance/interface/EventRange.h"
#include "DataFormats/Provenance/interface/LuminosityBlockRange.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/HcalDigi/interface/HcalCalibrationEventTypes.h"
#include "DataFormats/HcalDigi/interface/HcalLaserDigi.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/CaloMETCollection.h"
#include "DataFormats/METReco/interface/METCollection.h"
#include "DataFormats/CaloTowers/interface/CaloTowerCollection.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "TBDataFormats/HcalTBObjects/interface/HcalTBTriggerData.h"

#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"

//
// class declaration
//

class myEventData {
public:
  myEventData(const edm::ParameterSet&);
  ~myEventData() {}

  void get(const edm::Event&, const edm::EventSetup&);

  static const std::vector<edm::EventRange>          getEvents2Process(void);
  static const std::vector<edm::LuminosityBlockRange> getLumis2Process(void);

  inline edm::Handle<HcalTBTriggerData>      hcaltbtrigdata(void)  const { return hcaltbtrigdata_; }
  inline edm::Handle<FEDRawDataCollection>   fedrawdata(void)      const { return fedrawdata_;  }
  inline edm::Handle<HcalLaserDigi>          laserdigi(void)       const { return laserdigi_;   }
  inline edm::Handle<HBHEDigiCollection>     hbhedigis(void)       const { return hbhedigis_;   }
  inline edm::Handle<HBHERecHitCollection>   hbherechits(void)     const { return hbherechits_; }
  inline edm::Handle<HFRecHitCollection>     hfrechits(void)       const { return hfrechits_;   }
  inline edm::Handle<HFDigiCollection>       hfdigis(void)         const { return hfdigis_;     }
  inline edm::Handle<HORecHitCollection>     horechits(void)       const { return horechits_;   }
  inline edm::Handle<HODigiCollection>       hodigis(void)         const { return hodigis_;     }
  inline edm::Handle<ZDCRecHitCollection>    zdcrechits(void)      const { return zdcrechits_;  }
  inline edm::Handle<ZDCDigiCollection>      zdcdigis(void)        const { return zdcdigis_;    }
  inline edm::Handle<CaloTowerCollection>    towers(void)          const { return towers_;      }
  inline edm::Handle<CaloMETCollection>      calomet(void)         const { return calomet_;     }
  inline edm::Handle<METCollection>          recomet(void)         const { return recomet_;     }
  inline edm::Handle<edm::PCaloHitContainer> simhits(void)         const { return hsimhits_;    }
  inline edm::Handle<edm::TriggerResults>    trgResults(void)      const { return trgResults_;  }
  inline edm::Handle<reco::VertexCollection> vertices(void)        const { return vertices_;    }
  inline edm::Handle<bool>                   hbheNoiseResult(void) const { return hbheNoiseResult_; }

  inline uint32_t                            runNumber(void)     const { return runn_; }
  inline uint32_t                            evtNumber(void)     const { return evtn_; }
  inline uint32_t                            bxNumber(void)      const { return bxn_;  }
  inline uint32_t                            lumiSection(void)   const { return lsn_;  }

  inline const edm::InputTag&                fedRawDataTag(void) const { return fedRawDataTag_; }
  inline const edm::InputTag&                tbTrigDataTag(void) const { return tbTrigDataTag_; }
  inline const edm::InputTag&                laserDigiTag (void) const { return laserDigiTag_;  }
  inline const edm::InputTag&                hbheRechitTag(void) const { return hbheRechitTag_; }
  inline const edm::InputTag&                hbheDigiTag  (void) const { return hbheDigiTag_;   }
  inline const edm::InputTag&                hfRechitTag  (void) const { return hfRechitTag_;   }
  inline const edm::InputTag&                hfDigiTag    (void) const { return hfDigiTag_;     }
  inline const edm::InputTag&                hoRechitTag  (void) const { return hoRechitTag_;   }
  inline const edm::InputTag&                hoDigiTag    (void) const { return hoDigiTag_;     }
  inline const edm::InputTag&                zdcRechitTag (void) const { return zdcRechitTag_;  }
  inline const edm::InputTag&                zdcDigiTag   (void) const { return zdcDigiTag_;    }
  inline const edm::InputTag&                simHitTag    (void) const { return simHitTag_;     }
  inline const edm::InputTag&                caloMETtag   (void) const { return caloMETtag_;    }
  inline const edm::InputTag&                recoMETtag   (void) const { return recoMETtag_;    }
  inline const edm::InputTag&                twrTag       (void) const { return twrTag_;        }
  inline const edm::InputTag&                vertexTag    (void) const { return vertexTag_;     }
  inline const edm::InputTag&                trgResultsTag(void) const { return trgResultsTag_; }

  inline const edm::InputTag&                hbheNoiseResultTag(void) const { return hbheNoiseResultTag_; }

private:

  // ----------member data ---------------------------
  bool localHCALrun_;

  // configurable parameter:
  edm::InputTag      fedRawDataTag_;
  edm::InputTag      tbTrigDataTag_;
  edm::InputTag      laserDigiTag_;
  edm::InputTag      hbheRechitTag_;
  edm::InputTag      hbheDigiTag_;
  edm::InputTag      hfRechitTag_;
  edm::InputTag      hfDigiTag_;
  edm::InputTag      hoRechitTag_;
  edm::InputTag      hoDigiTag_;
  edm::InputTag      zdcRechitTag_;
  edm::InputTag      zdcDigiTag_;
  edm::InputTag      simHitTag_;
  edm::InputTag      caloMETtag_;
  edm::InputTag      recoMETtag_;
  edm::InputTag      twrTag_;
  edm::InputTag      vertexTag_;
  edm::InputTag      trgResultsTag_;
  edm::InputTag      hbheNoiseResultTag_;
  bool               verbose_;

  edm::EventID eventId_;
  uint32_t runn_;
  uint32_t evtn_;
  uint32_t bxn_;
  uint32_t lsn_;

  edm::Handle<HcalTBTriggerData>       hcaltbtrigdata_;
  edm::Handle<FEDRawDataCollection>    fedrawdata_;
  edm::Handle<edm::PCaloHitContainer>  hsimhits_;
  edm::Handle<HcalLaserDigi>           laserdigi_;
  edm::Handle<HBHERecHitCollection>    hbherechits_;
  edm::Handle<HBHEDigiCollection>      hbhedigis_;
  edm::Handle<HFRecHitCollection>      hfrechits_;
  edm::Handle<HFDigiCollection>        hfdigis_;
  edm::Handle<HORecHitCollection>      horechits_;
  edm::Handle<HODigiCollection>        hodigis_;
  edm::Handle<ZDCRecHitCollection>     zdcrechits_;
  edm::Handle<ZDCDigiCollection>       zdcdigis_;
  edm::Handle<CaloTowerCollection>     towers_;
  edm::Handle<reco::CaloMETCollection> calomet_;
  edm::Handle<reco::METCollection>     recomet_;
  edm::Handle<reco::VertexCollection>  vertices_;
  edm::Handle<edm::TriggerResults>     trgResults_;
  edm::Handle<bool>                    hbheNoiseResult_;
};

#endif // _MYEDMODULESMYANALUTILITIES
