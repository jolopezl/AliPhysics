Bool_t SetTPCCorrection = kFALSE;
Bool_t SetITSCorrection = kFALSE;
Bool_t SetTOFCorrection = kFALSE;

Bool_t SetGeneratedSmearingHistos = kTRUE;

Bool_t DoPairing    = kFALSE;
Bool_t DoULSLS      = kFALSE;
Bool_t DeactivateLS = kTRUE;

// Leave blank to not use resolution files
std::string resoFilename = "resolution_17f2a_deltaVsP_0090cent.root";
std::string resoFilenameFromAlien = "/alice/cern.ch/user/a/acapon/ResolutionFiles/resolution_17f2a_deltaVsP_0090cent.root";

Bool_t DoCocktailWeighting  = kFALSE;
Bool_t GetCocktailFromAlien = kFALSE;
std::string CocktailFilename = "";
std::string CocktailFilenameFromAlien = "/alice/cern.ch/user/a/acapon/.root";
// std::string CocktailFilenameFromAlien = "/alice/cern.ch/user/c/cklein/data/cocktail_PbPb0080_5TeV.root";

Bool_t GetCentralityFromAlien = kFALSE;
std::string centralityFilename = "";
std::string centralityFilenameFromAlien = "/alice/cern.ch/user/a/acapon/.root";

const Int_t triggerNames = AliVEvent::kINT7;

const Int_t nMCSignal   = 0;
const Int_t nCutsetting = 0;

const Double_t minGenPt  = 0.05;
const Double_t maxGenPt  = 10;
const Double_t minGenEta = -1.5;
const Double_t maxGenEta = 1.5;

const Double_t minPtCut  = 0.05;
const Double_t maxPtCut  = 8.0;
const Double_t minEtaCut = -0.8;
const Double_t maxEtaCut = 0.8;
// const Double_t minPtCut = 0.2;
// const Double_t maxPtCut = 8.0;
// const Double_t minEtaCut = -0.8;
// const Double_t maxEtaCut = 0.8;


// binning of single leg histograms
Bool_t usePtVector = kTRUE;
Double_t ptBins[] = {0.000,0.050,0.100,0.150,0.200,0.250,0.300,0.350,0.400,0.450,0.500,0.550,0.600,0.650,0.700,0.750,0.800,0.850,0.900,0.950,
  1.000,1.10,1.20,1.30,1.40,1.50,1.60,1.70,1.80,1.90,2.00,2.10,2.30,2.50,3.00,3.50,
  4.00,5.0,6.0,7.0,8.0
  };
const Int_t nBinsPt =  ( sizeof(ptBins) / sizeof(ptBins[0]) )-1;

const Double_t minPtBin   = 0;
const Double_t maxPtBin   = 8;
const Int_t    stepsPtBin = 400;

const Double_t minEtaBin   = -1.0;
const Double_t maxEtaBin   = 1.0;
const Int_t    stepsEtaBin = 20;

const Double_t minPhiBin   = 0;
const Double_t maxPhiBin   = 6.3;
const Int_t    stepsPhiBin = 20;

const Double_t minThetaBin   = 0;
const Double_t maxThetaBin   = TMath::TwoPi();
const Int_t    stepsThetaBin = 60;

const Double_t minMassBin     = 0;
const Double_t maxMassBin     = 5;
const Int_t    stepsMassBin   = 250;
const Double_t minPairPtBin   = 0;
const Double_t maxPairPtBin   = 8;
const Int_t    stepsPairPtBin = 160;

// Binning of resolution histograms
const Int_t    NbinsDeltaMom   = 2000;
const Double_t DeltaMomMin     = -10.0;
const Double_t DeltaMomMax     = 10.0;
const Int_t    NbinsRelMom     = 400;
const Double_t RelMomMin       = 0.0;
const Double_t RelMomMax       = 2.0;
const Int_t    NbinsDeltaEta   = 200;
const Double_t DeltaEtaMin     = -0.4;
const Double_t DeltaEtaMax     = 0.4;
const Int_t    NbinsDeltaTheta = 200;
const Double_t DeltaThetaMin   = -0.4;
const Double_t DeltaThetaMax   = 0.4;
const Int_t    NbinsDeltaPhi   = 200;
const Double_t DeltaPhiMin     = -0.4;
const Double_t DeltaPhiMax     = 0.4;

void GetCentrality(const Int_t centrality, Double_t& CentMin, Double_t& CentMax){
  std::cout << "GetCentrality with centrality " << centrality << std::endl;
  if     (centrality == 0){CentMin = 0;  CentMax = 100;}
  else if(centrality == 1){CentMin = 0;  CentMax = 20;}
  else if(centrality == 2){CentMin = 20; CentMax = 40;}
  else if(centrality == 3){CentMin = 40; CentMax = 60;}
  else if(centrality == 4){CentMin = 60; CentMax = 100;}
  else if(centrality == 5){CentMin = 0;  CentMax = 5;}
  else                      {std::cout << "WARNING::Centrality range not found....." std::endl;}
  return;
}

void ApplyPIDpostCalibration(AliAnalysisTaskElectronEfficiencyV2* task, Int_t whichDet){
  std::cout << task << std::endl;

  std::cout << "starting ApplyPIDpostCalibration()\n";
  if(whichDet == 0){// ITS
    std::cout << "Loading ITS correction" << std::endl;
    std::string file_name = "outputITS_17f2a.root";
    TFile* _file = TFile::Open(file_name.c_str());

    if(!_file){
      gSystem->Exec(("alien_cp alien:///alice/cern.ch/user/a/acapon/PIDcalibration/" + file_name + " .").c_str());
      std::cout << "Copy ITS correction from Alien" << std::endl;
      _file = TFile::Open(file_name.c_str());
    }
    else {
      std::cout << "Correction loaded" << std::endl;
    }

    TH3D* mean = dynamic_cast<TH3D*>(_file->Get("sum_mean_correction"));
    TH3D* width= dynamic_cast<TH3D*>(_file->Get("sum_width_correction"));

    task->SetCentroidCorrFunction(AliAnalysisTaskElectronEfficiencyV2::kITS, mean,  AliDielectronVarManager::kP, AliDielectronVarManager::kEta, AliDielectronVarManager::kRefMultTPConly);
    task->SetWidthCorrFunction   (AliAnalysisTaskElectronEfficiencyV2::kITS, width, AliDielectronVarManager::kP, AliDielectronVarManager::kEta, AliDielectronVarManager::kRefMultTPConly);
  }
  if(whichDet == 1){// TOF
    std::cout << "Loading TOF correction" << std::endl;
    std::string file_name = "outputTOF_17f2a.root";
    TFile* _file = TFile::Open(file_name.c_str());

    if(!_file){
      gSystem->Exec(("alien_cp alien:///alice/cern.ch/user/a/acapon/PIDcalibration/" + file_name + " .").c_str());
      std::cout << "Copy TOF correction from Alien" << std::endl;
      _file = TFile::Open(file_name.c_str());
    }
    else {
      std::cout << "Correction loaded" << std::endl;
    }

    TH3D* mean = dynamic_cast<TH3D*>(_file->Get("sum_mean_correction"));
    TH3D* width= dynamic_cast<TH3D*>(_file->Get("sum_width_correction"));

    task->SetCentroidCorrFunction(AliAnalysisTaskElectronEfficiencyV2::kTOF, mean,  AliDielectronVarManager::kP, AliDielectronVarManager::kEta, AliDielectronVarManager::kRefMultTPConly);
    task->SetWidthCorrFunction   (AliAnalysisTaskElectronEfficiencyV2::kTOF, width, AliDielectronVarManager::kP, AliDielectronVarManager::kEta, AliDielectronVarManager::kRefMultTPConly);
  }
}

// #########################################################
// #########################################################

AliAnalysisFilter* SetupTrackCutsAndSettings(TString cutDefinition)
{
  std::cout << "SetupTrackCutsAndSettings( cutInstance = " << cutDefinition << " )" <<std::endl;
  AliAnalysisFilter *anaFilter = new AliAnalysisFilter("anaFilter","anaFilter"); // named constructor seems mandatory!

	Bool_t SDDstatus = kTRUE;

  LMEECutLib* LMcutlib = new LMEECutLib(SDDstatus);
  if(cutDefinition == "kTTreeCuts"){
		std::cout << "TTree Cuts being set" << std::endl;
		anaFilter->AddCuts(LMcutlib->GetTrackCuts(LMEECutLib::kTTreeCuts, LMEECutLib::kTTreeCuts));
		anaFilter->SetName(cutDefinition);
		anaFilter->Print();
  }
	else if(cutDefinition == "kCutSet1"){ //TMVA
		std::cout << "Setting up cut set 1" << std::endl;
		anaFilter->AddCuts(LMcutlib->GetTrackCuts(LMEECutLib::kCutSet1, LMEECutLib::kCutSet1));
		anaFilter->SetName(cutDefinition);
		anaFilter->Print();
	}
	else{
		std::cout << "Undefined cut definition...." << std::endl;
		return 0x0;
	}

  return anaFilter;
}


// #########################################################
// #########################################################
/* AliAnalysisCuts* SetupEventCuts(Bool_t isAOD) */
/* { */
/*   std::cout << "Setup Event Cuts" << std::endl; */
/*   // event cuts are identical for all analysis 'cutInstance's that run together! */
/*   AliDielectronEventCuts *eventCuts=new AliDielectronEventCuts("eventCuts","Vertex Track && |vtxZ|<10 && ncontrib>0"); */
/*   eventCuts->SetRequireVertex(); */
/*   eventCuts->SetMinVtxContributors(1); */
/*   eventCuts->SetVertexZ(-10.,10.); */
/*   if(isAOD) eventCuts->SetVertexType(AliDielectronEventCuts::kVtxSPD); // AOD */
/*   eventCuts->Print(); */
/*   return eventCuts; */
/* } */



// #########################################################
// #########################################################
std::vector<Bool_t> AddSingleLegMCSignal(AliAnalysisTaskElectronEfficiencyV2* task){
  AliDielectronSignalMC partFinalState("partFinalState","partFinalState");
  partFinalState.SetLegPDGs(0,1);//dummy second leg (never MCkTRUE)\n"
  // partFinalState.SetCheckBothChargesLegs(kTRUE,kTRUE);
  partFinalState.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);

  AliDielectronSignalMC eleFinalState("eleFinalState","eleFinalState");
  eleFinalState.SetLegPDGs(11,1);//dummy second leg (never MCkTRUE)\n"
  eleFinalState.SetCheckBothChargesLegs(kTRUE,kTRUE);
  //eleFinalState.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);

  AliDielectronSignalMC eleFinalStateFromSameMotherMeson("eleFinalStateFromSameMotherMeson","eleFinalStateFromSameMotherMeson");
  eleFinalStateFromSameMotherMeson.SetLegPDGs(11,1);//dummy second leg (never MCkTRUE)\n"
  eleFinalStateFromSameMotherMeson.SetCheckBothChargesLegs(kTRUE,kTRUE);
  eleFinalStateFromSameMotherMeson.SetMotherPDGs(600, 600); // open charm mesons and baryons together
  eleFinalStateFromSameMotherMeson.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
  //
  AliDielectronSignalMC eleFinalStateFromD("eleFinalStateFromD","eleFinalStateFromD");
  eleFinalStateFromD.SetLegPDGs(11,1);//dummy second leg (never MCkTRUE)\n"
  eleFinalStateFromD.SetCheckBothChargesLegs(kTRUE,kTRUE);
  eleFinalStateFromD.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
  eleFinalStateFromD.SetMotherPDGs(402, 402); // open charm mesons and baryons together
  eleFinalStateFromD.SetCheckBothChargesMothers(kTRUE,kTRUE);
  //
  AliDielectronSignalMC eleFinalStateFromB("eleFinalStateFromB","eleFinalStateFromB");
  eleFinalStateFromB.SetLegPDGs(11,1);//dummy second leg (never MCkTRUE)\n"
  eleFinalStateFromB.SetCheckBothChargesLegs(kTRUE,kTRUE);
  eleFinalStateFromB.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
  eleFinalStateFromB.SetMotherPDGs(502, 502); // open charm mesons and baryons together
  eleFinalStateFromB.SetCheckBothChargesMothers(kTRUE,kTRUE);

  // AliDielectronSignalMC eleSecondary("eleSecondary","eleSecondary");
  // eleSecondary.SetLegPDGs(11,1);//dummy second leg (never MCkTRUE)\n"
  // eleSecondary.SetCheckBothChargesLegs(kTRUE,kTRUE);
  // eleSecondary.SetLegSources(AliDielectronSignalMC::kSecondary, AliDielectronSignalMC::kSecondary);
  // task->AddSingleLegMCSignal(eleSecondary);
  //
  // AliDielectronSignalMC eleDontCare("eleDontCare","eleDontCare");
  // eleDontCare.SetLegPDGs(11,1);//dummy second leg (never MCkTRUE)\n"
  // eleDontCare.SetCheckBothChargesLegs(kTRUE,kTRUE);
  // eleDontCare.SetLegSources(AliDielectronSignalMC::kDontCare, AliDielectronSignalMC::kDontCare);
  // task->AddSingleLegMCSignal(eleDontCare);

  // task->AddSingleLegMCSignal(partFinalState);
  task->AddSingleLegMCSignal(eleFinalState);
  // task->AddSingleLegMCSignal(eleFinalStateFromPion);
  task->AddSingleLegMCSignal(eleFinalStateFromD);
  task->AddSingleLegMCSignal(eleFinalStateFromB);
  // task->AddSingleLegMCSignal(eleFinalStateFromSameMotherMeson);

  // this is used to get electrons from charmed mesons in a environment where GEANT is doing the decay of D mesons, like in LHC18b5a
  // ordering is according to MCSignals of single legs
 std::vector<Bool_t> DielectronsPairNotFromSameMother;
 DielectronsPairNotFromSameMother.push_back(kFALSE);
 DielectronsPairNotFromSameMother.push_back(kTRUE);
 DielectronsPairNotFromSameMother.push_back(kTRUE);
 // DielectronsPairNotFromSameMother.push_back(kFALSE);
 return DielectronsPairNotFromSameMother;
}


// #########################################################
// #########################################################
void AddPairMCSignal(AliAnalysisTaskElectronEfficiencyV2* task){

    AliDielectronSignalMC pair_sameMother("sameMother","sameMother");
    pair_sameMother.SetLegPDGs(11,-11);
    pair_sameMother.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother.SetMotherPDGs(22,22,kTRUE,kTRUE); // exclude conversion electrons. should have no effect on final state ele.

    AliDielectronSignalMC pair_sameMother_pion("sameMother_pion","sameMother_pion");
    pair_sameMother_pion.SetLegPDGs(11,-11);
    pair_sameMother_pion.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_pion.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_pion.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_pion.SetMotherPDGs(111,111); //

    AliDielectronSignalMC pair_sameMother_eta("sameMother_eta","sameMother_eta");
    pair_sameMother_eta.SetLegPDGs(11,-11);
    pair_sameMother_eta.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_eta.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_eta.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_eta.SetMotherPDGs(221,221); //

    AliDielectronSignalMC pair_sameMother_etaP("sameMother_etaP","sameMother_etaP");
    pair_sameMother_etaP.SetLegPDGs(11,-11);
    pair_sameMother_etaP.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_etaP.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_etaP.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_etaP.SetMotherPDGs(331,331); //

    AliDielectronSignalMC pair_sameMother_rho("sameMother_rho","sameMother_rho");
    pair_sameMother_rho.SetLegPDGs(11,-11);
    pair_sameMother_rho.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_rho.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_rho.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_rho.SetMotherPDGs(113, 113); //

    AliDielectronSignalMC pair_sameMother_omega("sameMother_omega","sameMother_omega");
    pair_sameMother_omega.SetLegPDGs(11,-11);
    pair_sameMother_omega.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_omega.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_omega.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_omega.SetMotherPDGs(223, 223); //

    AliDielectronSignalMC pair_sameMother_phi("sameMother_phi","sameMother_phi");
    pair_sameMother_phi.SetLegPDGs(11,-11);
    pair_sameMother_phi.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_phi.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_phi.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_phi.SetMotherPDGs(333, 333); //

    AliDielectronSignalMC pair_sameMother_jpsi("sameMother_jpsi","sameMother_jpsi");
    pair_sameMother_jpsi.SetLegPDGs(11,-11);
    pair_sameMother_jpsi.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_jpsi.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_jpsi.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_jpsi.SetMotherPDGs(443, 443); //

    AliDielectronSignalMC pair_sameMother_CharmedMesonsWithSameMother("CharmedMesonsWithSameMother","CharmedMesonsWithSameMother");
    pair_sameMother_CharmedMesonsWithSameMother.SetLegPDGs(11,-11);
    pair_sameMother_CharmedMesonsWithSameMother.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_sameMother_CharmedMesonsWithSameMother.SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
    //mother
    pair_sameMother_CharmedMesonsWithSameMother.SetMothersRelation(AliDielectronSignalMC::kSame);
    pair_sameMother_CharmedMesonsWithSameMother.SetMotherPDGs(402, 402); //

    AliDielectronSignalMC pair_conversion("pair_conversion","pair_conversion");
    pair_conversion.SetLegPDGs(11,-11);
    pair_conversion.SetCheckBothChargesLegs(kTRUE,kTRUE);
    pair_conversion.SetLegSources(AliDielectronSignalMC::kSecondary, AliDielectronSignalMC::kSecondary);
    //mother
    pair_conversion.SetMothersRelation(AliDielectronSignalMC::kSame);



    task->AddPairMCSignal(pair_sameMother);
    // task->AddPairMCSignal(pair_sameMother_pion);
    // task->AddPairMCSignal(pair_sameMother_eta);
    // task->AddPairMCSignal(pair_sameMother_etaP);
    // task->AddPairMCSignal(pair_sameMother_rho);
    // task->AddPairMCSignal(pair_sameMother_omega);
    // task->AddPairMCSignal(pair_sameMother_phi);
    // task->AddPairMCSignal(pair_sameMother_jpsi);
    // task->AddPairMCSignal(pair_sameMother_CharmedMesonsWithSameMother);
    // task->AddPairMCSignal(pair_conversion);
}
