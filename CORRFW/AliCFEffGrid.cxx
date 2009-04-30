/* $Id$ */
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
//--------------------------------------------------------------------//
//                                                                    //
// AliCFEffGrid Class                                              //
// Class to handle efficiency grids                                   // 
//                                                                    //
// -- Author : S.Arcelli                                              //
//                                                                    //
//                                                                    //
//                                                                    //
//--------------------------------------------------------------------//
//
//
#include "TMath.h"
#include "AliLog.h"
#include "AliCFEffGrid.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"

//____________________________________________________________________
ClassImp(AliCFEffGrid)

//____________________________________________________________________
AliCFEffGrid::AliCFEffGrid() : 
  AliCFGridSparse(),
  fContainer(0x0),
  fSelNum(-1),
  fSelDen(-1)
{
  //
  // default constructor
  //
}

//____________________________________________________________________
AliCFEffGrid::AliCFEffGrid(const Char_t* name, const Char_t* title, const Int_t nVarIn, const Int_t * nBinIn, const Double_t *binLimitsIn) :  
  AliCFGridSparse(name,title,nVarIn,nBinIn,binLimitsIn),
  fContainer(0x0),
  fSelNum(-1),
  fSelDen(-1)
{
  //
  // ctor
  //
  SumW2();
}
//____________________________________________________________________
AliCFEffGrid::AliCFEffGrid(const Char_t* name, const Char_t* title, const AliCFContainer &c) :  
  AliCFGridSparse(name,title,c.GetNVar(),c.GetNBins(),c.GetBinLimits()),
  fContainer(NULL),
  fSelNum(-1),
  fSelDen(-1)
{
  //
  // main constructor
  //
  SumW2();
  //assign the container;
  fContainer=&c;
}
//____________________________________________________________________
AliCFEffGrid::AliCFEffGrid(const AliCFEffGrid& eff) : AliCFGridSparse(),
  fContainer(0x0),
  fSelNum(-1),
  fSelDen(-1)
{
  //
  // copy constructor
  //
  ((AliCFEffGrid &)eff).Copy(*this);
}

//____________________________________________________________________
AliCFEffGrid::~AliCFEffGrid()
{
  //
  // destructor
  //
}

//____________________________________________________________________
AliCFEffGrid &AliCFEffGrid::operator=(const AliCFEffGrid &eff)
{
  //
  // assigment operator
  //
  if (this != &eff)
    ((AliCFEffGrid &) eff).Copy(*this);
  return *this;
} 
//____________________________________________________________________

void AliCFEffGrid::CalculateEfficiency(Int_t istep1,Int_t istep2, Option_t *option)
{
  //
  // Calculate the efficiency matrix and its error between selection
  // Steps istep1 and istep2
  //

  fSelNum=istep1;
  fSelDen=istep2;
  AliCFVGrid *num=GetNum();
  AliCFVGrid *den=GetDen();
  num->SumW2();
  den->SumW2();
  this->SumW2();
  this->Divide(num,den,1.,1.,option);

  Int_t nEmptyBinsNum=0;
  Int_t nEmptyBinsNumAndDen=0;
  for(Int_t iel=0;iel<fNDim;iel++){
    if(den->GetElement(iel)>0){
      if(num->GetElement(iel)==0)nEmptyBinsNum++; //num==0,den!=0
    }
    else{
      nEmptyBinsNumAndDen++;
    }
  }    
  // Some monitoring printout:
  AliInfo(Form("Efficiency calculated for steps %i and %i: %i empty bins in the numerator && !denominator and %i empty bins in numerator && denominator were found.",fSelNum,fSelDen,nEmptyBinsNumAndDen,nEmptyBinsNum));
  AliInfo(Form("The correction map contains %i empty bins ",nEmptyBinsNum+nEmptyBinsNumAndDen));
} 
//_____________________________________________________________________
Double_t AliCFEffGrid::GetAverage() const 
{
  //
  // Get the average efficiency 
  //

  Double_t val=0;
  Double_t valnum=0;
  Double_t valden=0;
  for(Int_t i=0;i<fNDim;i++){
    valnum+=GetNum()->GetElement(i);
    valden+=GetDen()->GetElement(i);
  }
  if(valden>0)val=valnum/valden;
  AliInfo(Form(" The Average Efficiency = %f ",val)); 

  return val;
} 
//_____________________________________________________________________
Double_t AliCFEffGrid::GetAverage(Double_t *varMin, Double_t* varMax ) const 
{
  //
  // Get ave efficiency in a range
  //


  Double_t val=0;
  Int_t *indexMin = new Int_t[fNVar];
  Int_t *indexMax = new Int_t[fNVar];
  Int_t *index    = new Int_t[fNVar];
  
  //Find out the min and max bins
  
  for(Int_t i=0;i<fNVar;i++){
    Double_t xmin=varMin[i]; // the min values  
    Double_t xmax=varMax[i]; // the max values  
    Int_t nbins=fNVarBins[i]+1;
    Double_t *bins=new Double_t[nbins];
    for(Int_t ibin =0;ibin<nbins;ibin++){
      bins[ibin] = fVarBinLimits[ibin+fOffset[i]];
    }
    indexMin[i] = TMath::BinarySearch(nbins,bins,xmin);
    indexMax[i] = TMath::BinarySearch(nbins,bins,xmax);
    if(xmax>=bins[nbins-1]){
      indexMax[i]=indexMax[i]-1;
    }  
    delete [] bins;
  }
  
  Double_t valnum=0;
  Double_t valden=0;
  for(Int_t i=0;i<fNDim;i++){
    for (Int_t j=0;j<fNVar;j++)index[j]=GetBinIndex(j,i);
    Bool_t isIn=kTRUE;
    for (Int_t j=0;j<fNVar;j++){
      if(!(index[j]>=indexMin[j] && index[j]<=indexMax[j]))isIn=kFALSE;   
    }
    if(isIn){
      valnum+=GetNum()->GetElement(i);
      valden+=GetDen()->GetElement(i);
    }
  } 
  delete [] index;
  delete [] indexMin;
  delete [] indexMax;
  if(valden>0)val=valnum/valden;
  AliInfo(Form(" the Average Efficiency = %f ",val)); 
  return val;
} 
//____________________________________________________________________
void AliCFEffGrid::Copy(TObject& eff) const
{
  //
  // copy function
  //
  Copy(eff);
  AliCFEffGrid& target = (AliCFEffGrid &) eff;
  
  target.fSelNum=fSelNum; 
  target.fSelDen=fSelDen; 
  if(fContainer)
    target.fContainer=fContainer;
}
//___________________________________________________________________
TH1D *AliCFEffGrid::Project(Int_t ivar) const
{
  //
  // Make a 1D projection along variable ivar 
  //
 
  const Int_t nDim = 1 ;
  Int_t dim[nDim] = {ivar} ;
  THnSparse* hNum = ((AliCFGridSparse*)GetNum())->GetGrid()->Projection(nDim,dim);
  THnSparse* hDen = ((AliCFGridSparse*)GetDen())->GetGrid()->Projection(nDim,dim);
  THnSparse* ratio = (THnSparse*)hNum->Clone();
  ratio->Divide(hNum,hDen,1.,1.,"B");
  delete hNum; delete hDen;
  return ratio->Projection(ivar);
} 
//___________________________________________________________________
TH2D *AliCFEffGrid::Project(Int_t ivar1,Int_t ivar2) const
{
  //
  // Make a 2D projection along variable ivar1,ivar2 
  //
  
  const Int_t nDim = 2 ;
  Int_t dim[nDim] = {ivar1,ivar2} ;
  THnSparse* hNum = ((AliCFGridSparse*)GetNum())->GetGrid()->Projection(nDim,dim);
  THnSparse* hDen = ((AliCFGridSparse*)GetDen())->GetGrid()->Projection(nDim,dim);
  THnSparse* ratio = (THnSparse*)hNum->Clone();
  ratio->Divide(hNum,hDen,1.,1.,"B");
  delete hNum; delete hDen;
  return ratio->Projection(ivar1,ivar2);
} 
//___________________________________________________________________
TH3D *AliCFEffGrid::Project(Int_t ivar1, Int_t ivar2, Int_t ivar3) const
{
  //
  // Make a 3D projection along variable ivar1,ivar2,ivar3 
  //

  const Int_t nDim = 3 ;
  Int_t dim[nDim] = {ivar1,ivar2,ivar3} ;
  THnSparse* hNum = ((AliCFGridSparse*)GetNum())->GetGrid()->Projection(nDim,dim);
  THnSparse* hDen = ((AliCFGridSparse*)GetDen())->GetGrid()->Projection(nDim,dim);
  THnSparse* ratio = (THnSparse*)hNum->Clone();
  ratio->Divide(hNum,hDen,1.,1.,"B");
  delete hNum; delete hDen;
  return ratio->Projection(ivar1,ivar2,ivar3);
} 

