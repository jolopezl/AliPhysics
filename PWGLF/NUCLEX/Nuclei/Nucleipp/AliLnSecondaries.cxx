/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

// removal of secondaries using DCA distributions
// author: Eulogio Serradilla <eulogio.serradilla@cern.ch>

#include <Riostream.h>
#include <TFile.h>
#include <TString.h>
#include <TF1.h>
#include <TMath.h>
#include <TFractionFitter.h>
#include <TObjArray.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TBackCompFitter.h>

#include "AliLnSecondaries.h"
#include "B2.h"

ClassImp(AliLnSecondaries)

AliLnSecondaries::AliLnSecondaries(const TString& particle, const TString& dataFilename, const TString& simuFilename, const TString& outputFilename, const TString& otag)
: TObject()
, fParticle(particle)
, fDataFilename(dataFilename)
, fSimuFilename(simuFilename)
, fOutputFilename(outputFilename)
, fOutputTag(otag)
, fPtMin(0.5)
, fPtMax(3.0)
, fNbin(1)
, fMinDCAxy(-1.5)
, fMaxDCAxy(1.5)
, fFracProc(0)
, fMatDCAxyMod(AliLnSecondaries::kFlatDCAxy)
, fANucTemplate(0)
, fScMat(1)
, fScFd(1)
, fAddFakeTracks(1)
{
//
// constructor
//
	TH1::SetDefaultSumw2(); // switch on histogram errors
}

AliLnSecondaries::~AliLnSecondaries()
{
//
// destructor
//
}

Int_t AliLnSecondaries::Exec()
{
//
// Fractions of primaries and secondaries using templates from simulations
// Secondaries are from feed-down and/or from materials
//
	using namespace std;
	
	TFile* fdata = new TFile(fDataFilename.Data(),"read");
	if(fdata->IsZombie()) exit(1);
	
	TFile* fsimu = new TFile(fSimuFilename.Data(),"read");
	if(fsimu->IsZombie()) exit(1);
	
	TFile* fdebug =  new TFile(Form("debug-%s",fOutputFilename.Data()),"recreate");
	
	if(fOutputTag != "") fdebug->mkdir(fOutputTag.Data());
	if(fOutputTag != "") fdebug->cd(fOutputTag.Data());
	
	// --------- ideal values: primaries + no secondaries --------
	
	TH1D* hPidPt = FindObj<TH1D>(fdata, fParticle + "_PID_Pt");
	hPidPt->Write();
	
	const char* contrib[] = { "prim", "mat", "fdwn" };
	
	TH1D* hFracPt[3];
	for(Int_t i=0; i<3; ++i)
	{
		hFracPt[i] = this->ZeroClone(hPidPt, Form("%s_P%s_Pt",fParticle.Data(),contrib[i]));
		hFracPt[i]->SetYTitle(Form("P_{%s}",contrib[i]));
	}
	
	// fill
	
	if(fFracProc == kMonteCarlo)
	{
		// compute the fractions from the montecarlo simulation
		TH1D* hAll  = FindObj<TH1D>(fsimu, fParticle + "_Sim_Pt");
		TH1D* hPrim = FindObj<TH1D>(fsimu, fParticle + "_Sim_Prim_Pt");
		TH1D* hMat  = FindObj<TH1D>(fsimu, fParticle + "_Sim_Mat_Pt");
		TH1D* hFdwn = FindObj<TH1D>(fsimu, fParticle + "_Sim_Fdwn_Pt");
		
		delete hFracPt[0];
		delete hFracPt[1];
		delete hFracPt[2];
		
		hFracPt[0] = Divide(hPrim, hAll, fParticle + "_Pprim_Pt");
		hFracPt[1] = Divide(hMat,  hAll, fParticle + "_Pmat_Pt");
		hFracPt[2] = Divide(hFdwn, hAll, fParticle + "_Pfdwn_Pt");
		
		hFracPt[1]->Scale(fScMat);
		hFracPt[2]->Scale(fScFd);
	}
	else if(fParticle == "AntiProton")
	{
		// assume only secondaries from feed-down
		TH2D* hDataDCAxyPt     = FindObj<TH2D>(fdata, fParticle + "_PID_DCAxy_Pt");
		TH2D* hDataMCDCAxyPt   = FindObj<TH2D>(fsimu, fParticle + "_PID_DCAxy_Pt");
		TH2D* hPrimDCAxyPt     = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Prim_DCAxy_Pt");
		TH2D* hFdwnDCAxyPt     = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fdwn_DCAxy_Pt");
		TH2D* hFakePrimDCAxyPt = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fake_Prim_DCAxy_Pt");
		TH2D* hFakeFdwnDCAxyPt = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fake_Fdwn_DCAxy_Pt");
		
		if(fAddFakeTracks)
		{
			hPrimDCAxyPt->Add(hFakePrimDCAxyPt);
			hFdwnDCAxyPt->Add(hFakeFdwnDCAxyPt);
		}
		
		hDataDCAxyPt->Rebin2D(1,fNbin);
		hDataMCDCAxyPt->Rebin2D(1,fNbin);
		hPrimDCAxyPt->Rebin2D(1,fNbin);
		hFdwnDCAxyPt->Rebin2D(1,fNbin);
		
		this->GetFraction(hFracPt[0], hFracPt[2], hDataDCAxyPt, hDataMCDCAxyPt, hPrimDCAxyPt, hFdwnDCAxyPt, "Fdwn");
	}
	else if(fParticle.Contains("Anti"))
	{
		// assume there are no secondaries for antinuclei
		this->GetFraction(hFracPt[0]);
	}
	else if(fParticle == "Proton")
	{
		// secondaries from materials and feed-down
		TH2D* hDataDCAxyPt     = FindObj<TH2D>(fdata, fParticle + "_PID_DCAxy_Pt");
		TH2D* hDataMCDCAxyPt   = FindObj<TH2D>(fsimu, fParticle + "_PID_DCAxy_Pt");
		TH2D* hPrimDCAxyPt     = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Prim_DCAxy_Pt");
		TH2D* hMatDCAxyPt      = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Mat_DCAxy_Pt");
		TH2D* hFdwnDCAxyPt     = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fdwn_DCAxy_Pt");
		TH2D* hFakePrimDCAxyPt = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fake_Prim_DCAxy_Pt");
		TH2D* hFakeMatDCAxyPt  = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fake_Mat_DCAxy_Pt");
		TH2D* hFakeFdwnDCAxyPt = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fake_Fdwn_DCAxy_Pt");
		
		if(fAddFakeTracks)
		{
			hPrimDCAxyPt->Add(hFakePrimDCAxyPt);
			hMatDCAxyPt->Add(hFakeMatDCAxyPt);
			hFdwnDCAxyPt->Add(hFakeFdwnDCAxyPt);
		}
		
		if(fMatDCAxyMod == kFlatDCAxy)
		{
			hMatDCAxyPt = this->GetFlatDCAxyPt(10, hPrimDCAxyPt, fParticle + "_Sim_PID_Mat_DCAxy_Pt");
		}
		
		hDataDCAxyPt->Rebin2D(1,fNbin);
		hDataMCDCAxyPt->Rebin2D(1,fNbin);
		hPrimDCAxyPt->Rebin2D(1,fNbin);
		hMatDCAxyPt->Rebin2D(1,fNbin);
		hFdwnDCAxyPt->Rebin2D(1,fNbin);
		
		this->GetFraction(hFracPt, hDataDCAxyPt, hDataMCDCAxyPt, hPrimDCAxyPt, hMatDCAxyPt, hFdwnDCAxyPt);
	}
	else
	{
		// only secondaries from materials for nuclei
		TH2D* hDataDCAxyPt     = FindObj<TH2D>(fdata, fParticle + "_PID_DCAxy_Pt");
		TH2D* hDataMCDCAxyPt   = FindObj<TH2D>(fsimu, fParticle + "_PID_DCAxy_Pt");
		TH2D* hPrimDCAxyPt     = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Prim_DCAxy_Pt");
		TH2D* hFakePrimDCAxyPt = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Fake_Prim_DCAxy_Pt");
		
		if(fAddFakeTracks) hPrimDCAxyPt->Add(hFakePrimDCAxyPt);
		
		if(fANucTemplate)
		{
			hPrimDCAxyPt   = FindObj<TH2D>(fdata, Form("Anti%s_PID_DCAxy_Pt",fParticle.Data()));
		}
		
		TH2D* hMatDCAxyPt = FindObj<TH2D>(fsimu, fParticle + "_Sim_PID_Mat_DCAxy_Pt");
		if(fMatDCAxyMod == kFlatDCAxy)
		{
			hMatDCAxyPt = this->GetFlatDCAxyPt(10, hPrimDCAxyPt, fParticle + "_Sim_PID_Mat_DCAxy_Pt");
		}
		
		hDataDCAxyPt->Rebin2D(1,fNbin);
		hDataMCDCAxyPt->Rebin2D(1,fNbin);
		hPrimDCAxyPt->Rebin2D(1,fNbin);
		hMatDCAxyPt->Rebin2D(1,fNbin);
		
		this->GetFraction(hFracPt[0], hFracPt[1], hDataDCAxyPt, hDataMCDCAxyPt, hPrimDCAxyPt, hMatDCAxyPt, "Mat");
	}
	
	// --------- save --------------
	
	TFile* foutput = new TFile(fOutputFilename.Data(),"recreate");
	if(fOutputTag != "")
	{
		foutput->mkdir(fOutputTag.Data());
		foutput->cd(fOutputTag.Data());
	}
	
	// ----- fractions w.r.t. primaries ------------
	
	TH1D* hMatFracPt = (TH1D*)hFracPt[1]->Clone(Form("%s_Frac_Mat_Pt", fParticle.Data()));
	hMatFracPt->SetYTitle("P_{1} / P_{0}");
	hMatFracPt->Divide(hFracPt[0]);
	hMatFracPt->Write();
	
	TH1D* hFdwnFracPt = (TH1D*)hFracPt[2]->Clone( Form("%s_Frac_Fdwn_Pt",fParticle.Data()));
	hFdwnFracPt->SetYTitle("P_{2} / P_{0}");
	hFdwnFracPt->Divide(hFracPt[0]);
	hFdwnFracPt->Write();
	
	// --------- optionally fit fractions to extrapolate at high pt -----------
	
	TF1* fncMat = this->GetMatFraction(fParticle + "_Frac_Mat_Fit_Pt");
	if(fParticle.Contains("Anti"))
	{
		fncMat->SetParameters(0,0,0);
	}
	else if(fParticle=="Proton")
	{
		hMatFracPt->Fit(fncMat, "NQ", "", 0.,2);
	}
	else
	{
		hMatFracPt->Fit(fncMat, "NQ", "", 0.,1.6);
	}
	
	TF1* fncFdwn = this->GetFdwnFraction(fParticle + "_Frac_Fdwn_Fit_Pt");
	if(!fParticle.Contains("Proton"))
	{
		fncFdwn->SetParameters(0,0,0);
	}
	else
	{
		hFdwnFracPt->Fit(fncFdwn, "NQ", "", 0.5,3.);
	}
	
	fncFdwn->Write();
	fncMat->Write();
	
	// ---------- clean ---------
	
	delete fncFdwn;
	delete fncMat;
	
	delete hMatFracPt;
	delete hFdwnFracPt;
	
	for(Int_t i=0; i<3; ++i)
	{
		hFracPt[i]->Write();
		delete hFracPt[i];
	}
	
	delete foutput;
	delete fdebug;
	delete fsimu;
	delete fdata;
	
	delete dynamic_cast<TBackCompFitter*>(TVirtualFitter::GetFitter());
	
	return 0;
}

void AliLnSecondaries::GetFraction(TH1D* hPrimPt) const
{
//
// No secondaries only primaries
//
	TF1* one = new TF1("one", "1", hPrimPt->GetXaxis()->GetXmin(), hPrimPt->GetXaxis()->GetXmax());
	hPrimPt->Reset();
	hPrimPt->Sumw2();
	hPrimPt->Add(one);
	delete one;
}

void AliLnSecondaries::GetFraction(TH1D* hPrimPt, TH1D* hSecPt, const TH2D* hDCAxyPt, const TH2D* hMCDCAxyPt, const TH2D* hPrimDCAxyPt, const TH2D* hSecDCAxyPt, const TString& sec) const
{
//
// slice the DCA distributions and compute the fractions
// 2 contributions (primaries and secondaries)
//
	TString nosec = (sec == "Fdwn") ? "Mat" : "Fdwn";
	
	Int_t lowbin = hDCAxyPt->GetXaxis()->FindFixBin(fPtMin);
	Int_t hibin  = hDCAxyPt->GetXaxis()->FindFixBin(fPtMax);
	
	for(Int_t i=lowbin; i<hibin; ++i)
	{
		TH1D* hDCAxy      = hDCAxyPt->ProjectionY(Form("%s_Data_DCAxy_%02d",fParticle.Data(),i),i,i);
		TH1D* hMCDCAxy    = hMCDCAxyPt->ProjectionY(Form("%s_SimData_DCAxy_%02d",fParticle.Data(),i),i,i);
		TH1D* hPrimDCAxy  = hPrimDCAxyPt->ProjectionY(Form("%s_Prim_DCAxy_%02d",fParticle.Data(),i),i,i);
		TH1D* hSecDCAxy   = hSecDCAxyPt->ProjectionY(Form("%s_%s_DCAxy_%02d",fParticle.Data(),sec.Data(),i),i,i);
		
		// fractions
		Double_t frac[2] = {0};
		Double_t err[2]  = {0};
		
		this->GetTFFfractions(frac, err, hDCAxy, hPrimDCAxy, hSecDCAxy, i, sec);
		
		// fill
		TH1D* hFracPt[] = { hPrimPt, hSecPt};
		for(Int_t j=0; j<2; ++j)
		{
			hFracPt[j]->SetBinContent(i, frac[j]);
			hFracPt[j]->SetBinError(i, err[j]);
		}
		
		// ------------- debug histograms ------------
		
		hDCAxy->Write();
		hMCDCAxy->Write();
		hPrimDCAxy->Write();
		hSecDCAxy->Write();
		
		TH1D* hNoSecDCAxy = this->ZeroClone(hMCDCAxy, Form("%s_%s_DCAxy_%02d",fParticle.Data(),nosec.Data(),i));
		TH1D* hNoSecFit   = this->ZeroClone(hMCDCAxy, Form("%s_Fit_%s_DCAxy_%02d",fParticle.Data(),nosec.Data(),i));
		
		hNoSecDCAxy->Write();
		hNoSecFit->Write();
		
		delete hNoSecDCAxy;
		delete hNoSecFit;
		
		// ----------------------- end debug ----------------------
		
		delete hDCAxy;
		delete hMCDCAxy;
		delete hPrimDCAxy;
		delete hSecDCAxy;
	}
}

void AliLnSecondaries::GetFraction(TH1D* hFracPt[3], const TH2D* hDCAxyPt, const TH2D* hMCDCAxyPt, const TH2D* hPrimDCAxyPt, const TH2D* hMatDCAxyPt, const TH2D* hFdwnDCAxyPt) const
{
//
// slice the DCA distribution and get the fractions for each pt bin
// (3 contributions)
//
	Int_t lowbin = hDCAxyPt->GetXaxis()->FindFixBin(fPtMin);
	Int_t hibin  = hDCAxyPt->GetXaxis()->FindFixBin(fPtMax);
	
	for(Int_t i=lowbin; i<hibin; ++i)
	{
		// slices
		TH1D* hDCAxy     = hDCAxyPt->ProjectionY(Form("%s_Data_DCAxy_%02d",fParticle.Data(),i),i,i);
		TH1D* hMCDCAxy   = hMCDCAxyPt->ProjectionY(Form("%s_SimData_DCAxy_%02d",fParticle.Data(),i),i,i);
		TH1D* hPrimDCAxy = hPrimDCAxyPt->ProjectionY(Form("%s_Prim_DCAxy_%02d",fParticle.Data(),i),i,i);
		TH1D* hMatDCAxy  = hMatDCAxyPt->ProjectionY(Form("%s_Mat_DCAxy_%02d",fParticle.Data(),i),i,i);
		TH1D* hFdwnDCAxy = hFdwnDCAxyPt->ProjectionY(Form("%s_Fdwn_DCAxy_%02d",fParticle.Data(),i),i,i);
		
		// fractions
		Double_t frac[3] = {0};
		Double_t err[3]  = {0};
		
		this->GetTFFfractions(frac, err, hDCAxy, hPrimDCAxy, hMatDCAxy, hFdwnDCAxy, i);
		
		// fill
		for(Int_t k=0; k<3; ++k)
		{
			hFracPt[k]->SetBinContent(i, frac[k]);
			hFracPt[k]->SetBinError(i, err[k]);
		}
		
		// -------------- debug ---------------------------
		
		hDCAxy->Write();
		hMCDCAxy->Write();
		hPrimDCAxy->Write();
		hMatDCAxy->Write();
		hFdwnDCAxy->Write();
		
		// --------------------- end debug ----------------
		
		delete hDCAxy;
		delete hMCDCAxy;
		delete hPrimDCAxy;
		delete hMatDCAxy;
		delete hFdwnDCAxy;
	}
}

Int_t AliLnSecondaries::GetTFFfractions(Double_t* frac, Double_t* err, TH1D* hData, TH1D* hPrim, TH1D* hSec, Int_t ibin, const TString& sec) const
{
//
// find the fractions of 2 contributions
//
	TObjArray* mc = new TObjArray(2);
	
	mc->Add(hPrim);
	mc->Add(hSec);
	
	TFractionFitter* fit = new TFractionFitter(hData, mc, "Q"); // initialise
	fit->Constrain(1,0.0001,1.);
	fit->Constrain(2,0.0001,1.);
	
	Int_t status = fit->Fit();
	status = fit->Fit();
	status = fit->Fit();
	
	if (status == 0) // get fractions
	{
		fit->GetResult(0, frac[0], err[0]);
		fit->GetResult(1, frac[1], err[1]);
	}
	
	const char* contrib[] = {"Prim", sec.Data() };
	this->WriteTFFdebug(hData, fit, status, ibin, contrib, frac, 2);
	
	delete mc;
	delete fit;
	
	return status;
}

Int_t AliLnSecondaries::GetTFFfractions(Double_t* frac, Double_t* err, TH1D* hData, TH1D* hPrim, TH1D* hMat, TH1D* hFdwn, Int_t ibin) const
{
//
// find the fractions of 3 contributions
//
	TObjArray* mc = new TObjArray(3);
	
	mc->Add(hPrim);
	mc->Add(hMat);
	mc->Add(hFdwn);
	
	TFractionFitter* fit = new TFractionFitter(hData, mc, "Q");
	fit->Constrain(1,0.0001,1.);
	fit->Constrain(2,0.0001,1.);
	fit->Constrain(3,0.0001,1.);
	//fit->SetRangeX(1,50);
	
	Int_t status = fit->Fit();
	status = fit->Fit();
	status = fit->Fit();
	
	if (status == 0) // get fractions
	{
		fit->GetResult(0, frac[0], err[0]);
		fit->GetResult(1, frac[1], err[1]);
		fit->GetResult(2, frac[2], err[2]);
	}
	
	const char* contrib[] = {"Prim", "Mat", "Fdwn"};
	this->WriteTFFdebug(hData, fit, status, ibin, contrib, frac, 3);
	
	delete mc;
	delete fit;
	
	return status;
}

void AliLnSecondaries::WriteTFFdebug(const TH1D* hData, TFractionFitter* fit, Int_t status, Int_t ibin, const char* contrib[], Double_t* frac, Int_t kmax) const
{
//
// Write TFractionFitter debug histograms
//
	if (status == 0)
	{
		TH1D* hResult = (TH1D*) fit->GetPlot();
		
		hResult->SetName(Form("%s_Fit_Data_DCAxy_%02d",fParticle.Data(),ibin));
		hResult->Write();
		
		for(Int_t k=0; k<kmax; ++k)
		{
			TH1D* hMCpred = (TH1D*) fit->GetMCPrediction(k);
			hMCpred->SetName(Form("%s_Fit_%s_DCAxy_%02d",fParticle.Data(),contrib[k],ibin));
			
			hMCpred->Sumw2();
			hMCpred->Scale(frac[k]*hResult->Integral()/hMCpred->Integral());
			
			hMCpred->Write();
		}
	}
	else // write void histograms
	{
		TH1D* hZero = this->ZeroClone(hData,Form("%s_Fit_Data_DCAxy_%02d",fParticle.Data(),ibin));
		hZero->Write();
		
		for(Int_t k=0; k<kmax; ++k)
		{
			hZero->SetName(Form("%s_Fit_%s_DCAxy_%02d",fParticle.Data(),contrib[k],ibin));
			hZero->Write();
		}
		delete hZero;
	}
}

TH1D* AliLnSecondaries::ZeroClone(const TH1D* h, const TString& name) const
{
//
// clone histogram and reset to zero
//
	TH1D* clone = dynamic_cast<TH1D*>(h->Clone(name.Data()));
	if(clone == 0)
	{
		this->Error("ZeroClone", "could not clone %s", h->GetName());
		exit(1);
	}
	clone->Reset();
	clone->Sumw2();
	
	return clone;
}

TH2D* AliLnSecondaries::GetFlatDCAxyPt(Double_t max, const TH2D* hDCAxyPt, const TString& name) const
{
//
// generate a flat dcaxy-pt distribution
//
	TF1* fnc = new TF1("gen","1", fMinDCAxy, fMaxDCAxy);
	TH2D* h = (TH2D*)hDCAxyPt->Clone(name.Data());
	h->Reset();
	h->Sumw2();
	
	TAxis* xAxis = h->GetXaxis();
	TAxis* yAxis = h->GetYaxis();
	
	for(Int_t i=1; i<xAxis->GetNbins()+1; ++i)
	{
		Double_t pt = xAxis->GetBinCenter(i);
		
		for(Int_t j=1; j<yAxis->GetNbins()+1; ++j)
		{
			for(Int_t k=0; k<max; ++k) h->Fill(pt,fnc->GetRandom());
		}
	}
	
	delete fnc;
	return h;
}

TF1* AliLnSecondaries::GetMatFraction(const TString& name) const
{
//
// model the material fraction as an exponential + a constant
//
	TF1* fnc = new TF1(name.Data(), "[0]*exp(-[1]*x)+[2]",0,10);
	fnc->SetParameters(1.7,5.5,0.01);
	
	fnc->SetParLimits(2,0,0.02); // asymptotic value
	
	return fnc;
}

TF1* AliLnSecondaries::GetFdwnFraction(const TString& name) const
{
//
// model the feed-down fraction as an exponential + constant
//
	TF1* fnc = new TF1(name, "[0]*exp(-[1]*x)+[2]",0,10);
	fnc->SetParameters(0.5,1,0.02);
	
	fnc->SetParLimits(2,0,0.3); // asymptotic value
	
	return fnc;
}
