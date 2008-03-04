#ifndef ALIDAJETFINDER_H
#define ALIDAJETFINDER_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//---------------------------------------------------------------------
//Jet finder based on Deterministic Annealing
//Author: Davide Perrino (davide.perrino@ba.infn.it)
//---------------------------------------------------------------------

#include <AliJetFinder.h>
#include <TMatrixD.h>
#include <TVectorD.h>
class AliDAJetHeader;

class AliDAJetFinder : public AliJetFinder
{
public:
    AliDAJetFinder();
    virtual  ~AliDAJetFinder();
    
    void FindJets      ();
 private:
    void InitDetAnn    (Double_t &dEtSum);
    void Annealing     (Int_t nk);
    void NumCl         (Int_t &nc,Int_t &nk);
    void ReduceClusters(Int_t **iSame,Int_t nc,Int_t &ncout,Int_t **cont,Int_t *nSameOut);
    void DoubleClusters(Int_t nc,Int_t &nk);
    void EndDetAnn     (Int_t &nk,Int_t *xx,Double_t etx);
    void StoreJets     (Int_t nk,Int_t *xx);

protected:
    AliDAJetFinder(const AliDAJetFinder &jf);
    AliDAJetFinder& operator=(const AliDAJetFinder &jf);
    Double_t   fAlpha;					// beta increment
    Double_t   fDelta;					// perturbation proportional to Delta
    Double_t   fAvDist;					// minimum distance to distinguish two clusters
    Double_t   fEps;					// convergence criterium below max number of loops
    Double_t   fEpsMax;					// convergence criterium above max number of loops
    Int_t      fNloopMax;				// maximum number of loops at a fixed beta
    Double_t   fBeta;					// increasing multiplier of entropy
    Int_t      fNclustMax;				// maximum number of clusters to find
    TMatrixD  *fPyx;					// conditional probability matrix
    TMatrixD  *fY;                                      // clusters matrix
    TVectorD  *fPx;                                     // input data weights
    TVectorD  *fPy;                                     // clusters' probability
    Double_t  *fXEta;					// input data eta coordinate
    Double_t  *fXPhi;					// input data phi coordinate
    Int_t      fNin;					// number of input data
    ClassDef(AliDAJetFinder,1)
};// 
#endif
