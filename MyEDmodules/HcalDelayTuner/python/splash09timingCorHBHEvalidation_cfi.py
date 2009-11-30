from MyEDmodules.HcalDelayTuner.splashtiminganal_cfi import *


hbtimeanal.CorrectionsList = cms.VPSet(
    # feed back timing changes to see what affect it would've had
    #
    cms.PSet(applyExternal     = cms.bool(True),
             applyToHits       = cms.bool(True),
             timecorrFilenames = cms.untracked.vstring("../data/timeCorrs_old2newSettings_2ndIt_hbhed1.csv",
                                                       "../data/timeCorrs_old2newSettings_2ndIt_hbhed2.csv"),
             timecorrScanFmt   = cms.untracked.string("%s %d %d %d %f")
             ),
    )

hetimeanal.CorrectionsList = cms.VPSet(
    # feed back timing changes to see what affect it would've had
    #
    cms.PSet(applyExternal     = cms.bool(True),
             applyToHits       = cms.bool(True),
             timecorrFilenames = cms.untracked.vstring("../data/timeCorrs_old2newSettings_2ndIt_hbhed1.csv",
                                                       "../data/timeCorrs_old2newSettings_2ndIt_hbhed2.csv",
                                                       "../data/timeCorrs_old2newSettings_2ndIt_hbhed3.csv"),
             timecorrScanFmt   = cms.untracked.string("%s %d %d %d %f")
             ),
    )

