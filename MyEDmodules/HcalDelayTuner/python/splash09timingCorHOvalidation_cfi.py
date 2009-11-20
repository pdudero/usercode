from MyEDmodules.HcalDelayTuner.splashtiminganal_cfi import *


hotimeanal.CorrectionsList = cms.VPSet(
    #
    # this is strictly for display, to compensate for HO map changes that
    # occurred since the determination of the old settings, to see if
    # the associated discrepancies go away. "ApplyToHits" means the correction
    # is applied to each rechit in the data rather than compiled into the
    # output corrections
    #
    cms.PSet(applyExternal     = cms.bool(True),
             applyToHits       = cms.bool(True),
             timecorrFilenames = cms.untracked.vstring("../data/timeCorrs_run120042_HO_fromFloatingRingsTune.csv"),
             timecorrScanFmt   = cms.untracked.string("%s %d %d %d %f")
             ),
    )
