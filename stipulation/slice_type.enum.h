#if !defined(STIPULATION_SLICE_TYPE_ENUM_H)
#define STIPULATION_SLICE_TYPE_ENUM_H

typedef enum
{
 STProxy, STTemporaryHackFork, STQuodlibetStipulationModifier, STGoalIsEndStipulationModifier, STWhiteToPlayStipulationModifier, STPostKeyPlayStipulationModifier, STStipulationStarterDetector, STCountersWriter, STStipulationCopier, STProofSolverBuilder, STAToBSolverBuilder, STEndOfStipulationSpecific, STSolvingMachineryIntroBuilder, STOutputPlainTextOptionNoboard, STOutputPlaintextTwinIntroWriterBuilder, STOutputLaTeXTwinningWriterBuilder, STOutputPlainTextInstrumentSolvers, STOutputPlainTextInstrumentSolversBuilder, STOutputLaTeXInstrumentSolvers, STOutputLaTeXInstrumentSolversBuilder, STTwinIdAdjuster, STConditionsResetter, STOptionsResetter, STProblemSolvingIncomplete, STPhaseSolvingIncomplete, STCommandLineOptionsParser, STInputPlainTextOpener, STPlatformInitialiser, STHashTableDimensioner, STTimerStarter, STZeroPositionInitialiser, STZeropositionSolvingStopper, STCheckDirInitialiser, STOutputLaTeXCloser, STOutputPlainTextWriter, STInputPlainTextUserLanguageDetector, STInputPlainTextProblemsIterator, STInputPlainTextProblemHandler, STInputPlainTextInitialTwinReader, STInputPlainTextTwinsHandler, STStartOfCurrentTwin, STDuplexSolver, STHalfDuplexSolver, STCreateBuilderSetupPly, STPieceWalkCacheInitialiser, STPiecesCounter, STPiecesFlagsInitialiser, STRoyalsLocator, STProofgameVerifyUniqueGoal, STProofgameStartPositionInitialiser, STProofSolver, STInputVerification, STMoveEffectsJournalReset, STSolversBuilder1, STSolversBuilder2, STProxyResolver, STSlackLengthAdjuster, STAttackAdapter, STDefenseAdapter, STReadyForAttack, STReadyForDefense, STNotEndOfBranchGoal, STNotEndOfBranch, STMinLengthOptimiser, STHelpAdapter, STReadyForHelpMove, STSetplayFork, STRetroStartRetractionPly, STRetroRetractLastCapture, STRetroUndoLastPawnMultistep, STRetroStartRetroMovePly, STRetroPlayNullMove, STRetroRedoLastCapture, STRetroRedoLastPawnMultistep, STEndOfBranch, STEndOfBranchForced, STEndOfBranchGoal, STEndOfBranchTester, STEndOfBranchGoalTester, STAvoidUnsolvable, STResetUnsolvable, STLearnUnsolvable, STConstraintSolver, STConstraintTester, STGoalConstraintTester, STEndOfRoot, STEndOfIntro, STDeadEnd, STMove, STForEachAttack, STForEachDefense, STFindAttack, STFindDefense, STPostMoveIterationInitialiser, STMoveGenerationPostMoveIterator, STSquareObservationPostMoveIterator, STMoveEffectJournalUndoer, STMovePlayer, STCastlingPlayer, STEnPassantAdjuster, STPawnPromoter, STFootballChessSubsitutor, STBeforePawnPromotion, STLandingAfterPawnPromotion, STNullMovePlayer, STRoleExchangeMovePlayer, STLandingAfterMovingPieceMovement, STLandingAfterMovePlay, STAttackPlayed, STDefensePlayed, STHelpMovePlayed, STDummyMove, STShortSolutionsStart, STIfThenElse, STCheckZigzagLanding, STBlackChecks, STSingleBoxType1LegalityTester, STSingleBoxType2LegalityTester, STSingleBoxType3PawnPromoter, STSingleBoxType3TMovesForPieceGenerator, STSingleBoxType3LegalityTester, STSingleBoxType3EnforceObserverWalk, STIsardamLegalityTester, STLeffieLegalityTester, STCirceAssassinAssassinate, STCirceAssassinAllPieceObservationTester, STCirceParachuteRemember, STCirceVolcanicRemember, STCirceVolcanicSwapper, STCirceParachuteUncoverer, STKingSquareUpdater, STOwnKingCaptureAvoider, STOpponentKingCaptureAvoider, STPatienceChessLegalityTester, STGoalReachedTester, STGoalMateReachedTester, STGoalStalemateReachedTester, STGoalDoubleStalemateReachedTester, STGoalTargetReachedTester, STGoalCheckReachedTester, STGoalCaptureReachedTester, STGoalSteingewinnReachedTester, STGoalEnpassantReachedTester, STGoalDoubleMateReachedTester, STGoalCounterMateReachedTester, STGoalCastlingReachedTester, STGoalAutoStalemateReachedTester, STGoalAutoMateReachedTester, STGoalCircuitReachedTester, STGoalExchangeReachedTester, STGoalCircuitByRebirthReachedTester, STGoalExchangeByRebirthReachedTester, STGoalAnyReachedTester, STGoalProofgameReachedTester, STGoalAToBReachedTester, STGoalMateOrStalemateReachedTester, STGoalChess81ReachedTester, STGoalKissReachedTester, STGoalImmobileReachedTester, STGoalNotCheckReachedTester, STGoalKingCaptureReachedTester, STUnsaveableTester, STTrue, STFalse, STAnd, STOr, STNot, STSelfCheckGuard, STOhneschachStopIfCheck, STOhneschachStopIfCheckAndNotMate, STOhneschachDetectUndecidableGoal, STRecursionStopper, STMoveInverter, STMoveInverterSetPlay, STMinLengthGuard, STForkOnRemaining, STFindShortest, STFindByIncreasingLength, STGeneratingMoves, STExecutingKingCapture, STDoneGeneratingMoves, STDoneRemovingIllegalMoves, STDoneRemovingFutileMoves, STDonePriorisingMoves, STMoveGenerator, STKingMoveGenerator, STNonKingMoveGenerator, STSkipMoveGeneration, STCastlingIntermediateMoveLegalityTester, STCastlingRightsAdjuster, STMoveGeneratorFork, STGeneratingMovesForPiece, STGeneratedMovesForPiece, STMovesForPieceBasedOnWalkGenerator, STCastlingGenerator, STRefutationsAllocator, STRefutationsSolver, STRefutationsFilter, STEndOfRefutationSolvingBranch, STTrivialTryAvoider, STPlaySuppressor, STContinuationSolver, STSolvingContinuation, STThreatSolver, STThreatEnforcer, STThreatStart, STThreatEnd, STThreatCollector, STThreatDefeatedTester, STRefutationsCollector, STRefutationsAvoider, STLegalAttackCounter, STLegalDefenseCounter, STAnyMoveCounter, STCaptureCounter, STTestingPrerequisites, STDoubleMateFilter, STCounterMateFilter, STPrerequisiteOptimiser, STNoShortVariations, STRestartGuard, STRestartGuardNested, STRestartGuardIntelligent, STIntelligentTargetCounter, STMaxTimeProblemInstrumenter, STMaxTimeSetter, STMaxTimeGuard, STMaxSolutionsProblemInstrumenter, STMaxSolutionsSolvingInstrumenter, STMaxSolutionsInitialiser, STMaxSolutionsGuard, STMaxSolutionsCounter, STEndOfBranchGoalImmobile, STDeadEndGoal, STOrthodoxMatingMoveGenerator, STOrthodoxMatingKingContactGenerator, STKillerAttackCollector, STKillerDefenseCollector, STKillerMovePrioriser, STKillerMoveFinalDefenseMove, STEnPassantFilter, STEnPassantRemoveNonReachers, STCastlingFilter, STCastlingRemoveNonReachers, STChess81RemoveNonReachers, STCaptureRemoveNonReachers, STTargetRemoveNonReachers, STDetectMoveRetracted, STRetractionPrioriser, STUncapturableRemoveCaptures, STNocaptureRemoveCaptures, STWoozlesRemoveIllegalCaptures, STBiWoozlesRemoveIllegalCaptures, STHeffalumpsRemoveIllegalCaptures, STBiHeffalumpsRemoveIllegalCaptures, STProvocateursRemoveUnobservedCaptures, STLortapRemoveSupportedCaptures, STPatrolRemoveUnsupportedCaptures, STUltraPatrolMovesForPieceGenerator, STBackhomeExistanceTester, STBackhomeRemoveIllegalMoves, STNoPromotionsRemovePromotionMoving, STGridRemoveIllegalMoves, STMonochromeRemoveNonMonochromeMoves, STBichromeRemoveNonBichromeMoves, STEdgeMoverRemoveIllegalMoves, STShieldedKingsRemoveIllegalCaptures, STSuperguardsRemoveIllegalCaptures, STKoekoLegalityTester, STGridContactLegalityTester, STAntiKoekoLegalityTester, STNewKoekoRememberContact, STNewKoekoLegalityTester, STJiggerLegalityTester, STTakeAndMakeGenerateMake, STTakeAndMakeAvoidPawnMakeToBaseLine, STWormholeRemoveIllegalCaptures, STWormholeTransferer, STHashOpener, STAttackHashed, STAttackHashedTester, STHelpHashed, STHelpHashedTester, STIntelligentMovesLeftInitialiser, STIntelligentFilter, STIntelligentFlightsGuarder, STIntelligentFlightsBlocker, STIntelligentMateTargetPositionTester, STIntelligentStalemateTargetPositionTester, STIntelligentTargetPositionFound, STIntelligentProof, STGoalReachableGuardFilterMate, STGoalReachableGuardFilterStalemate, STGoalReachableGuardFilterProof, STGoalReachableGuardFilterProofFairy, STIntelligentSolutionsPerTargetPosProblemInstrumenter, STIntelligentSolutionsPerTargetPosSolvingInstrumenter, STIntelligentSolutionsPerTargetPosInitialiser, STIntelligentSolutionsPerTargetPosCounter, STIntelligentLimitNrSolutionsPerTargetPosLimiter, STIntelligentDuplicateAvoider, STIntelligentSolutionRememberer, STIntelligentImmobilisationCounter, STKeepMatingFilter, STMaxFlightsquares, STFlightsquaresCounter, STDegenerateTree, STMaxNrNonTrivial, STMaxNrNonTrivialCounter, STMaxThreatLength, STMaxThreatLengthStart, STStopOnShortSolutionsProblemInstrumenter, STStopOnShortSolutionsSolvingInstrumenter, STStopOnShortSolutionsWasShortSolutionFound, STStopOnShortSolutionsFilter, STAmuMateFilter, STUltraschachzwangLegalityTester, STCirceSteingewinnFilter, STCirceCircuitSpecial, STCirceExchangeSpecial, STPiecesParalysingMateFilter, STPiecesParalysingMateFilterTester, STPiecesParalysingStalemateSpecial, STPiecesParalysingRemoveCaptures, STPiecesParalysingSuffocationFinderFork, STPiecesParalysingSuffocationFinder, STPiecesKamikazeTargetSquareFilter, STPiecesHalfNeutralRecolorer, STMadrasiMovesForPieceGenerator, STPartialParalysisMovesForPieceGenerator, STEiffelMovesForPieceGenerator, STDisparateMovesForPieceGenerator, STParalysingMovesForPieceGenerator, STParalysingObserverValidator, STParalysingObservationGeometryValidator, STPepoMovesForPieceGenerator, STPepoCheckTestInitialiser, STPepoCheckTestHack, STCentralMovesForPieceGenerator, STCentralObservationValidator, STBeamtenMovesForPieceGenerator, STImmobilityTester, STOpponentMovesCounterFork, STOpponentMovesCounter, STOpponentMovesFewMovesPrioriser, STStrictSATInitialiser, STStrictSATUpdater, STDynastyKingSquareUpdater, STHurdleColourChanger, STHopperAttributeSpecificPromotionInitialiser, STHopperAttributeSpecificPromotion, STBul, STBulPlyCatchup, STBulPlyRewinder, STOscillatingKingsTypeA, STOscillatingKingsTypeB, STOscillatingKingsTypeC, STPawnToImitatorPromoter, STExclusiveChessExclusivityDetector, STExclusiveChessNestedExclusivityDetector, STExclusiveChessLegalityTester, STExclusiveChessMatingMoveCounterFork, STExclusiveChessGoalReachingMoveCounter, STExclusiveChessUndecidableWriterTree, STExclusiveChessUndecidableWriterLine, STExclusiveChessGoalReachingMoveCounterSelfCheckGuard, STMaffImmobilityTesterKing, STOWUImmobilityTesterKing, STBrunnerDefenderFinder, STKingCaptureLegalityTester, STMoveLegalityTester, STCageCirceNonCapturingMoveFinder, STCageCirceFutileCapturesRemover, STSinglePieceMoveGenerator, STMummerOrchestrator, STMummerBookkeeper, STMummerDeadend, STUltraMummerMeasurerFork, STUltraMummerMeasurerDeadend, STBackHomeFinderFork, STBackHomeMovesOnly, STCheckTesterFork, STTestingCheck, STTestedCheck, STNoCheckConceptCheckTester, STVogtlaenderCheckTester, STExtinctionExtinctedTester, STExtinctionAllPieceObservationTester, STCastlingGeneratorTestDeparture, STNoKingCheckTester, STSATCheckTester, STSATxyCheckTester, STStrictSATCheckTester, STKingSquareObservationTesterPlyInitialiser, STAntikingsCheckTester, STKingCapturedObservationGuard, STKingSquareObservationTester, STCastlingSuspender, STObservingMovesGenerator, STAttackTarget, STBGLEnforcer, STBGLAdjuster, STMasandRecolorer, STMasandEnforceObserver, STMasandGeneralisedRecolorer, STMasandGeneralisedEnforceObserver, STInfluencerWalkChanger, STMessignyMovePlayer, STActuatedRevolvingCentre, STActuatedRevolvingBoard, STRepublicanKingPlacer, STRepublicanType1DeadEnd, STCirceConsideringRebirth, STGenevaConsideringRebirth, STMarsCirceConsideringRebirth, STMarsCirceConsideringObserverRebirth, STAntimarsCirceConsideringRebirth, STGenevaStopCaptureFromRebirthSquare, STCirceCaptureFork, STCirceParrainThreatFork, STAprilCaptureFork, STCircePreventKingRebirth, STCirceInitialiseRelevantFromReborn, STCirceDeterminingRebirth, STCirceDeterminedRebirth, STCirceInitialiseFromCurrentMove, STCirceInitialiseFromLastMove, STCirceInitialiseRebornFromCapturee, STAntiCloneCirceDetermineRebornWalk, STCirceCloneDetermineRebornWalk, STCirceDoubleAgentsAdaptRebornSide, STChameleonCirceAdaptRebornWalk, STCirceEinsteinAdjustRebornWalk, STCirceReverseEinsteinAdjustRebornWalk, STCirceCouscousMakeCapturerRelevant, STAnticirceCouscousMakeCaptureeRelevant, STMirrorCirceOverrideRelevantSide, STCirceDetermineRebirthSquare, STCirceFrischaufAdjustRebirthSquare, STCirceGlasgowAdjustRebirthSquare, STFileCirceDetermineRebirthSquare, STCirceDiametralAdjustRebirthSquare, STCirceVerticalMirrorAdjustRebirthSquare, STRankCirceOverrideRelevantSide, STRankCirceProjectRebirthSquare, STTakeMakeCirceDetermineRebirthSquares, STTakeMakeCirceCollectRebirthSquaresFork, STTakeMakeCirceCollectRebirthSquares, STAntipodesCirceDetermineRebirthSquare, STSymmetryCirceDetermineRebirthSquare, STVerticalSymmetryCirceDetermineRebirthSquare, STHorizontalSymmetryCirceDetermineRebirthSquare, STPWCDetermineRebirthSquare, STDiagramCirceDetermineRebirthSquare, STContactGridAvoidCirceRebirth, STCirceTestRebirthSquareEmpty, STCircePlacingReborn, STCircePlaceReborn, STSuperCirceNoRebirthFork, STSuperCirceDetermineRebirthSquare, STCirceCageNoCageFork, STCirceCageCageTester, STCirceParrainDetermineRebirth, STCirceContraparrainDetermineRebirth, STCirceRebirthAvoided, STCirceRebirthOnNonEmptySquare, STSupercircePreventRebirthOnNonEmptySquare, STCirceDoneWithRebirth, STCirceVolageRecolorer, STAnticirceConsideringRebirth, STCirceKamikazeCaptureFork, STAnticirceInitialiseRebornFromCapturer, STMarscirceInitialiseRebornFromGenerated, STGenevaInitialiseRebornFromCapturer, STAnticirceCheylanFilter, STAnticirceRemoveCapturer, STMarscirceRemoveCapturer, STSeriesCapturePlyRewinder, STSeriesCaptureRecursionLanding, STBeforeSeriesCapture, STSeriesCaptureFork, STLandingAfterSeriesCapture, STSeriesCaptureJournalFixer, STSentinellesInserter, STMagicViewsInitialiser, STMagicPiecesRecolorer, STMagicPiecesObserverEnforcer, STSingleboxType2LatentPawnSelector, STSingleboxType2LatentPawnPromoter, STDuellistsRememberDuellist, STHauntedChessGhostSummoner, STHauntedChessGhostRememberer, STGhostChessGhostRememberer, STKobulKingSubstitutor, STSnekSubstitutor, STSnekCircleSubstitutor, STAndernachSideChanger, STAntiAndernachSideChanger, STDarksideSideChanger, STBretonApplier, STBretonPopeyeApplier, STChameleonPursuitSideChanger, STLostPiecesTester, STLostPiecesRemover, STNorskRemoveIllegalCaptures, STNorskArrivingAdjuster, STProteanPawnAdjuster, STEinsteinArrivingAdjuster, STReverseEinsteinArrivingAdjuster, STAntiEinsteinArrivingAdjuster, STEinsteinEnPassantAdjuster, STTraitorSideChanger, STVolageSideChanger, STMagicSquareSideChanger, STMagicSquareType2SideChanger, STMagicSquareType2AnticirceRelevantSideAdapter, STTibetSideChanger, STDoubleTibetSideChanger, STDegradierungDegrader, STChameleonChangePromoteeInto, STChameleonArrivingAdjuster, STChameleonChessArrivingAdjuster, STLineChameleonArrivingAdjuster, STFrischaufPromoteeMarker, STCASTMovesForPieceGenerator, STCASTValidateObservation, STCASTInverseMovesForPieceGenerator, STCASTInverseValidateObservation, STBicapturesRecolorPieces, STBicapturesUnrecolorPieces, STMakeTakeResetMoveIdsCastlingAsMakeInMoveGeneration, STMakeTakeResetMoveIdsCastlingAsMakeInMoveGenerationInCheckTest, STMakeTakeGenerateCapturesWalkByWalk, STMakeTakeLimitMoveGenerationMakeWalk, STMakeTakeMoveCastlingPartner, STBoleroGenerateMovesWalkByWalk, STBoleroInverseEnforceObserverWalk, STMarsCirceMoveToRebirthSquare, STMarsCirceMoveGeneratorEnforceRexInclusive, STPhantomAvoidDuplicateMoves, STPhantomKingSquareObservationTesterPlyInitialiser, STPlusAdditionalCapturesForPieceGenerator, STMoveForPieceGeneratorAlternativePath, STMoveForPieceGeneratorStandardPath, STMoveForPieceGeneratorPathsJoint, STMoveForPieceGeneratorTwoPaths, STMarsCirceRememberNoRebirth, STMarsCirceRememberRebirth, STMarsCirceFixDeparture, STMarsCirceGenerateFromRebirthSquare, STMoveGeneratorRejectCaptures, STMoveGeneratorRejectNoncaptures, STKamikazeCapturingPieceRemover, STHaanChessHoleInserter, STCastlingChessMovePlayer, STExchangeCastlingMovePlayer, STSuperTransmutingKingTransmuter, STSuperTransmutingKingMoveGenerationFilter, STReflectiveKingsMovesForPieceGenerator, STVaultingKingsMovesForPieceGenerator, STTransmutingKingsMovesForPieceGenerator, STSuperTransmutingKingsMovesForPieceGenerator, STRokagogoMovesForPieceGeneratorFilter, STCastlingChessMovesForPieceGenerator, STPlatzwechselRochadeMovesForPieceGenerator, STMessignyMovesForPieceGenerator, STAnnanMovesForPieceGenerator, STNannaMovesForPieceGenerator, STPointReflectionMovesForPieceGenerator, STFaceToFaceMovesForPieceGenerator, STBackToBackMovesForPieceGenerator, STCheekToCheekMovesForPieceGenerator, STAMUAttackCounter, STAMUObservationCounter, STMutualCastlingRightsAdjuster, STImitatorMover, STImitatorRemoveIllegalMoves, STImitatorDetectIllegalMoves, STNullMoveGenerator, STRoleExchangeMoveGenerator, STTotalInvisibleMoveSequenceTester, STTotalInvisibleMoveSequenceMoveRepeater, STTotalInvisibleInstrumenter, STTotalInvisibleInvisiblesAllocator, STTotalInvisibleUninterceptableSelfCheckGuard, STTotalInvisibleGoalGuard, STTotalInvisibleSpecialMoveGenerator, STTotalInvisibleMovesByInvisibleGenerator, STTotalInvisibleSpecialMovesPlayer, STTotalInvisibleReserveKingMovement, STTotalInvisibleRevealAfterFinalMove, STOutputModeSelector, STOutputPlainTextMetaWriter, STOutputPlainTextBoardWriter, STOutputPlainTextPieceCountsWriter, STOutputPlainTextAToBIntraWriter, STOutputPlainTextStartOfTargetWriter, STOutputPlainTextStipulationWriter, STOutputPlainTextRoyalPiecePositionsWriter, STOutputPlainTextNonRoyalAttributesWriter, STOutputPlainTextConditionsWriter, STOutputPlainTextMutuallyExclusiveCastlingsWriter, STOutputPlainTextDuplexWriter, STOutputPlainTextHalfDuplexWriter, STOutputPlainTextQuodlibetWriter, STOutputPlainTextGridWriter, STOutputPlainTextEndOfPositionWriters, STOutputPlaintextZeropositionIntroWriter, STOutputPlaintextTwinIntroWriter, STIllegalSelfcheckWriter, STOutputPlainTextEndOfPhaseWriter, STOutputPlainTextTreeEndOfSolutionWriter, STOutputLaTeXTreeEndOfSolutionWriter, STOutputPlainTextThreatWriter, STOutputLaTeXThreatWriter, STOutputPlainTextMoveWriter, STOutputLaTeXMoveWriter, STOutputPlainTextKeyWriter, STOutputLaTeXKeyWriter, STOutputPlainTextTryWriter, STOutputLaTeXTryWriter, STOutputPlainTextZugzwangByDummyMoveCheckWriter, STOutputPlainTextZugzwangWriter, STOutputLaTeXZugzwangByDummyMoveCheckWriter, STOutputLaTeXZugzwangWriter, STTrivialEndFilter, STOutputPlaintextTreeRefutingVariationWriter, STOutputPlaintextLineRefutingVariationWriter, STOutputLaTeXTreeRefutingVariationWriter, STOutputPlainTextRefutationsIntroWriter, STOutputPlainTextRefutationWriter, STOutputLaTeXRefutationWriter, STOutputPlaintextTreeCheckWriter, STOutputLaTeXTreeCheckWriter, STOutputPlaintextLineLineWriter, STOutputPlaintextGoalWriter, STOutputPlaintextConstraintWriter, STOutputPlaintextMoveInversionCounter, STOutputPlaintextMoveInversionCounterSetPlay, STOutputPlaintextLineEndOfIntroSeriesMarker, STOutputPlaintextProblemWriter, STOutputLaTeXProblemWriter, STOutputLaTeXProblemIntroWriter, STOutputLaTeXTwinningWriter, STOutputLaTeXLineLineWriter, STOutputLaTeXGoalWriter, STOutputLaTeXConstraintWriter, STIsSquareObservedFork, STTestingIfSquareIsObserved, STTestingIfSquareIsObservedWithSpecificWalk, STDetermineObserverWalk, STBicoloresTryBothSides, STOptimisingObserverWalk, STDontTryObservingWithNonExistingWalk, STDontTryObservingWithNonExistingWalkBothSides, STOptimiseObservationsByQueenInitialiser, STOptimiseObservationsByQueen, STUndoOptimiseObservationsByQueen, STTrackBackFromTargetAccordingToObserverWalk, STVaultingKingsEnforceObserverWalk, STVaultingKingIsSquareObserved, STTransmutingKingsEnforceObserverWalk, STTransmutingKingIsSquareObserved, STTransmutingKingDetectNonTransmutation, STReflectiveKingsEnforceObserverWalk, STAnnanEnforceObserverWalk, STNannaEnforceObserverWalk, STPointReflectionEnforceObserverWalk, STFaceToFaceEnforceObserverWalk, STBackToBackEnforceObserverWalk, STCheekToCheekEnforceObserverWalk, STIsSquareObservedTwoPaths, STIsSquareObservedStandardPath, STIsSquareObservedAlternativePath, STIsSquareObservedPathsJoint, STMarsIterateObservers, STMarsCirceIsSquareObservedEnforceRexInclusive, STPlusIsSquareObserved, STMarsIsSquareObserved, STIsSquareObservedOrtho, STValidatingCheckFork, STValidatingCheck, STValidatingObservationFork, STValidatingObservation, STEnforceObserverWalk, STEnforceHunterDirection, STBrunnerValidateCheck, STValidatingObservationUltraMummer, STValidatingObserverFork, STValidatingObserver, STValidatingObservationGeometryFork, STValidatingObservationGeometry, STValidatingObservationGeometryByPlayingMove, STValidateCheckMoveByPlayingCapture, STMoveTracer, STMoveCounter, nr_slice_types, no_slice_type = nr_slice_types
} slice_type;
extern char const *slice_type_names[];
/* include slice_type.enum to make sure that all the dependencies are generated correctly: */
#include "slice_type.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS

#endif
