from MyEDmodules.HcalDelayTuner.splashtiminganal_cfi import *

hetimeanal.CorrectionsList = cms.VPSet(
    cms.PSet(applyExternal     = cms.bool(False),
             correctByPhi      = cms.bool(False),
             correctByEta      = cms.bool(False),
             selfSynchronize   = cms.bool(False),
             devthresh         = cms.double(1.5),
             ietamin           = cms.int32(-29),
             ietamax           = cms.int32( 29),
             iphimin           = cms.int32(  1),
             iphimax           = cms.int32( 72),
             mirrorCorrection  = cms.bool(False)
             )
    )

hbtimeanal.CorrectionsList = cms.VPSet(
    cms.PSet(applyExternal     = cms.bool(False),
             correctByPhi      = cms.bool(False),
             correctByEta      = cms.bool(True),
             selfSynchronize   = cms.bool(False),
             devthresh         = cms.double(0.8),
             ietamin           = cms.int32(13),
             ietamax           = cms.int32(16),
             iphimin           = cms.int32( 1),
             iphimax           = cms.int32(72),
             mirrorCorrection  = cms.bool(True)
             )
    )

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
             timecorrFilenames = cms.untracked.vstring("../data/HOmapcorrectionsNoHO2P02.txt"),
             timecorrScanFmt   = cms.untracked.string("%s %d %d %d %f")
             ),
    #
    # this is to compensate for non-application of eta-corrections (which are
    # in the old settings) to the SiPM boxes, which were not under RBXManager
    # control at the time of Splash09
    #
    cms.PSet(applyExternal     = cms.bool(True),
             applyToHits       = cms.bool(True),
             timecorrFilenames = cms.untracked.vstring("../data/SiPMboxEtaCorrections.txt"),
             timecorrScanFmt   = cms.untracked.string("%s %d %d %d %*s %*d %*d %*d %f")
             ),
    #
    # Do a phi tune since eta already done last year
    #
    cms.PSet(applyExternal     = cms.bool(False),
             correctByPhi      = cms.bool(True),
             correctByEta      = cms.bool(False),
             selfSynchronize   = cms.bool(False),
             devthresh         = cms.double(0.0),
             ietamin           = cms.int32(-15),
#            ietamax           = cms.int32(  4),
             ietamax           = cms.int32( 15),
             iphimin           = cms.int32(  1),
             iphimax           = cms.int32( 72),
             mirrorCorrection  = cms.bool(False)
             ),
    #
    # Individually tune outliers to the HO mean
    #
    cms.PSet(applyExternal     = cms.bool(False),
             correctByPhi      = cms.bool(False),
             correctByEta      = cms.bool(False),
             selfSynchronize   = cms.bool(True),
             devthresh         = cms.double(2.0),
             ietamin           = cms.int32(-15),
             ietamax           = cms.int32( 15),
             iphimin           = cms.int32(  1),
             iphimax           = cms.int32( 72),
             mirrorCorrection  = cms.bool(False)
             )
    #
    # The following list is set up to avoid the SiPM boxes
    # but tune everything else.
    #
    #   cms.PSet(applyExternal     = cms.bool(False),
#            correctByPhi      = cms.bool(True),
#            correctByEta      = cms.bool(False),
#            devthresh         = cms.double(0.0),
#            ietamin           = cms.int32( 5),
#            ietamax           = cms.int32(10),
#            iphimin           = cms.int32(  1),
#            iphimax           = cms.int32( 46),
#            mirrorCorrection  = cms.bool(False)
#            ),
#   cms.PSet(applyExternal     = cms.bool(False),
#            correctByPhi      = cms.bool(True),
#            correctByEta      = cms.bool(False),
#            devthresh         = cms.double(0.0),
#            ietamin           = cms.int32( 5),
#            ietamax           = cms.int32(10),
#            iphimin           = cms.int32(59),
#            iphimax           = cms.int32(72),
#            mirrorCorrection  = cms.bool(False)
#            ),
#   cms.PSet(applyExternal     = cms.bool(False),
#            correctByPhi      = cms.bool(True),
#            correctByEta      = cms.bool(False),
#            devthresh         = cms.double(0.0),
#            ietamin           = cms.int32(11),
#            ietamax           = cms.int32(15),
#            iphimin           = cms.int32( 1),
#            iphimax           = cms.int32(58),
#            mirrorCorrection  = cms.bool(False)
#            ),
#   cms.PSet(applyExternal     = cms.bool(False),
#            correctByPhi      = cms.bool(True),
#            correctByEta      = cms.bool(False),
#            devthresh         = cms.double(0.0),
#            ietamin           = cms.int32(11),
#            ietamax           = cms.int32(15),
#            iphimin           = cms.int32(71),
#            iphimax           = cms.int32(72),
#            mirrorCorrection  = cms.bool(False)
#            )
    )
