// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Projections/FinalState.hh"
#include "Rivet/Projections/PromptFinalState.hh"
#include "Rivet/Projections/ChargedFinalState.hh"
#include "Rivet/Tools/AliceCommon.hh"
#include "Rivet/Projections/AliceCommon.hh"
#include "Rivet/Projections/PromptFinalState.hh"
#include "Rivet/Projections/PrimaryParticles.hh"
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include "../Centralities/RHICCentrality.hh"

#define _USE_MATH_DEFINES
static const int numTrigPtBins = 4;
static const float pTTrigBins[] = {2.0,3.0,4.0,5.0,10.0};
static const int numAssocPtBins = 6;
static const float pTAssocBins[] = {0.4,1.0,2.0,3.0,4.0,5.0,10.0};
static const int numCentBins = 5;
static const float CentBins[] = {0.0,12.0,20.0,40.0,60.0,92.0};
static const int numDeltaPhiBins = 26;
static const float DeltaPhiBins[] = {-1.5048,-1.275,-0.98,-0.685,-0.415,-0.22,-0.1,0,0.1,0.22,0.415,0.685,0.98,
1.275,1.595,1.965,2.305,2.55,2.75,2.945,3.14,3.44,3.535,3.73,3.935,4.32,4.7012};
static const int numpTAssocBins2 = 4;
static const float pTAssocBins2[] = {1.0,1.5,2.0,2.5,3.0};

using namespace std;

namespace Rivet {

	class Correlator {
     
    private:
      int _index;
      int _subindex;
      int _subsubindex;
      string _collSystemAndEnergy;
      pair<double,double> _centrality;
      pair<double,double> _triggerRange;
      pair<double,double> _associatedRange;
      vector<int> _pid;
      bool _noCentrality = false;
    public:
   
      /// Constructor
      Correlator(int index, int subindex, int subsubindex) {
        _index = index;
        _subindex = subindex;
        _subsubindex = subsubindex;
      }
      void SetCollSystemAndEnergy(string s){ _collSystemAndEnergy = s; }
      void SetCentrality(double cmin, double cmax){ _centrality = make_pair(cmin, cmax); }
      void SetNoCentrality(){ _noCentrality = true; }
      void SetTriggerRange(double tmin, double tmax){ _triggerRange = make_pair(tmin, tmax); }
      void SetAssociatedRange(double amin, double amax){ _associatedRange = make_pair(amin, amax); }
      void SetPID(std::initializer_list<int> pid){ _pid = pid; }
   
      string GetCollSystemAndEnergy(){ return _collSystemAndEnergy; }
      pair<double,double> GetCentrality(){ return _centrality; }
      double GetCentralityMin(){ return _centrality.first; }
      double GetCentralityMax(){ return _centrality.second; }
      pair<double,double> GetTriggerRange(){ return _triggerRange; }
      double GetTriggerRangeMin(){ return _triggerRange.first; }
      double GetTriggerRangeMax(){ return _triggerRange.second; }
      pair<double,double> GetAssociatedRange(){ return _associatedRange; }
      double GetAssociatedRangeMin(){ return _associatedRange.first; }
      double GetAssociatedRangeMax(){ return _associatedRange.second; }
      vector<int> GetPID(){ return _pid; }
   
      int GetIndex(){ return _index; }
      int GetSubIndex(){ return _subindex; }
      int GetSubSubIndex(){ return _subsubindex; }
      string GetFullIndex()
      {
          string fullIndex = to_string(GetIndex()) + to_string(GetSubIndex())+ to_string(GetSubIndex());
          return fullIndex;
      }
   
      bool CheckCollSystemAndEnergy(string s){ return _collSystemAndEnergy.compare(s) == 0 ? true : false; }
     bool CheckCentrality(double cent){ return ((cent>_centrality.first && cent<_centrality.second) || _noCentrality == true) ? true : false; }
      bool CheckTriggerRange(double tpt){ return (tpt>_triggerRange.first && tpt<_triggerRange.second) ? true : false; }
      bool CheckAssociatedRange(double apt){ return (apt>_associatedRange.first && apt<_associatedRange.second) ? true : false; }
      bool CheckAssociatedRangeMaxTrigger(double apt, double tpt){ return (apt>_associatedRange.first && apt<tpt) ? true : false; }
      bool CheckPID(std::initializer_list<int> pid)

      {
         
          bool inList = false;
         
          for(int id : pid)
          {
              auto it = std::find(_pid.begin(), _pid.end(), id);
             
              if(it != _pid.end())
              {
                  inList = true;
                  break;
              }
          }
         
          return inList;
         
      }
     
      bool CheckConditions(string s, double cent, double tpt, double apt)
      {
        if(!CheckConditions(s, cent, tpt)) return false;
        if(!CheckAssociatedRange(apt)) return false;
       
        return true;
       
      }
   
      bool CheckConditions(string s, double cent, double tpt)
      {
        if(!CheckConditions(s, cent)) return false;
        if(!CheckTriggerRange(tpt)) return false;
       
        return true;
       
      }
   
      bool CheckConditions(string s, double cent)
      {
        if(!CheckCollSystemAndEnergy(s)) return false;
        if(!CheckCentrality(cent)) return false;
       
        return true;
       
      }
     
      bool CheckConditionsMaxTrigger(string s, double cent, double tpt, double apt)
      {
        if(!CheckCollSystemAndEnergy(s)) return false;
        if(!CheckCentrality(cent)) return false;
        if(!CheckTriggerRange(tpt)) return false;
        if(!CheckAssociatedRangeMaxTrigger(apt,tpt)) return false;
       
        return true;
       
      }
     };

  /// @brief Add a short analysis description here
  class PHENIX_2008_I778396 : public Analysis {
  public:

    /// Constructor
    DEFAULT_RIVET_ANALYSIS_CTOR(PHENIX_2008_I778396);

    bool isSameParticle(const Particle& p1, const Particle& p2)
      {
        //if pT, eta and phi are equal, they are the same particle
        if(p1.pt() != p2.pt()) return false;
        if(p1.eta() != p2.eta()) return false;
        if(p1.phi() != p2.phi()) return false;
   
        return true;
      }
   
      bool isSecondary(Particle p)
      {
        //return true if is secondary
        if (( p.hasAncestor(310) || p.hasAncestor(-310)  ||     // K0s
          p.hasAncestor(130)  || p.hasAncestor(-130)  ||     // K0l
          p.hasAncestor(3322) || p.hasAncestor(-3322) ||     // Xi0
          p.hasAncestor(3122) || p.hasAncestor(-3122) ||     // Lambda
          p.hasAncestor(3222) || p.hasAncestor(-3222) ||     // Sigma+/-
          p.hasAncestor(3312) || p.hasAncestor(-3312) ||     // Xi-/+
          p.hasAncestor(3334) || p.hasAncestor(-3334) ))    // Omega-/+
          return true;
        else return false;
       
      }
    double CalculateVn(YODA::Histo1D& hist, int nth)
    {
        int nBins = hist.numBins();
        double integral = 0.;
       
        double Vn = 0.;
        for (int i = 0; i < nBins; i++)
        {
            integral += hist.bin(i).sumW();
            Vn += hist.bin(i).sumW()*cos(nth*hist.bin(i).xMid());
        }
        Vn /= integral;
        return Vn;
    }
   
    int FindBinAtMinimum(YODA::Histo1D& hist, double bmin, double bmax)
    {
        int minBin = -1;
        double minVal = 999.;
       
        for(unsigned int i = 0; i < hist.numBins(); i++)
        {
            if(hist.bin(i).xMid() < bmin || hist.bin(i).xMid() > bmax) continue;
            if( (hist.bin(i).sumW()/hist.bin(i).xWidth()) < minVal )
            {
                minVal = hist.bin(i).sumW()/hist.bin(i).xWidth();
                minBin = i;
            }
        }
       
        return minBin;
       
    }
   
    void SubtractBackground(YODA::Histo1D& fullHist, YODA::Histo1D& hist, vector<int> n, double bmin, double bmax)
    {
        vector<double> Vn(n.size(), 0);
        for(unsigned int i = 0; i < n.size(); i++)
        {
            Vn[i] = CalculateVn(fullHist, n[i]);
        }
       
        double bmod = 1.;
        int minBin = FindBinAtMinimum(fullHist, bmin, bmax);
       
        for(unsigned int i = 0; i < Vn.size(); i++)
        {
            bmod += 2*Vn[i]*cos(n[i]*fullHist.bin(minBin).xMid());
        }
       
        double b = (fullHist.bin(minBin).sumW()/fullHist.bin(minBin).xWidth())/bmod; //Divided by bin width in order to generalize it and enable it to be used for histograms with different binning
               
        for(unsigned int ibin = 0; ibin < hist.numBins(); ibin++)
        {
            double modulation = 1;
            for(unsigned int i = 0; i < Vn.size(); i++)
            {
                modulation += 2*Vn[i]*cos(n[i]*hist.bin(ibin).xMid());
            }
            modulation *= b;
            hist.bin(ibin).scaleW(1 - (modulation/(hist.bin(ibin).sumW()/hist.bin(ibin).xWidth()))); //Divided by bin width to compensate the calculation of "b"
        }
       
    }

    void init() {
        const ChargedFinalState cfs(Cuts::abseta < 0.35);
        declare(cfs, "CFS");
        
        const PrimaryParticles pp(pdgPi0, Cuts::abseta < 0.35);
        declare(pp, "PP");
        
        const PromptFinalState pfs(Cuts::abseta < 0.35 && Cuts::pid == 22);
        declare(pfs, "PFS");
// The following will book the histograms for Figure 36-38
        int pta;
        int ptt;
        int pta2;
        int cb;
for(pta = 0; pta<numAssocPtBins; pta++){
	for(ptt = 0; ptt<numTrigPtBins; ptt++){

      Correlator c1(pta,ptt,-1);
      c1.SetCollSystemAndEnergy("pp200GeV");
     	c1.SetNoCentrality();
     	c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
     	c1.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
     	//c1.SetPID(pdgPi0);
     	Correlators38.push_back(c1);

      Correlator c2(pta,ptt,1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
     	c2.SetCentrality(0,20);
     	c2.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
     	c2.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
     	//c1.SetPID(pdgPi0);
     	Correlators38.push_back(c2);

      Correlator c3(pta,ptt,2);
      c3.SetCollSystemAndEnergy("AuAu200GeV");
      c3.SetCentrality(20,40);
      c3.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c3.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
      //c1.SetPID(pdgPi0);
      Correlators38.push_back(c3);

      Correlator c4(pta,ptt,3);
      c4.SetCollSystemAndEnergy("AuAu200GeV");
      c4.SetCentrality(60,92);
      c4.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c4.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
      //c1.SetPID(pdgPi0);
      Correlators38.push_back(c4);
	
	}
}

// The following will book the histograms for Figure 31
for(ptt = 0; ptt<numTrigPtBins; ptt++){
  for(cb = 0; cb<numCentBins; cb++){
        Correlator c1(pta,ptt,cb);
        c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetCentrality(CentBins[cb],CentBins[cb+1]);
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
      //c1.SetPID(pdgPi0);
      Correlators31.push_back(c1);
  }
}

// The following will book the histograms for Figures 28-30
for(ptt = 0; ptt<numTrigPtBins; ptt++){
  for(cb = 0; cb<numCentBins; cb++){
        Correlator c1(pta,ptt,cb);
        c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetCentrality(CentBins[cb],CentBins[cb+1]);
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
      //c1.SetPID(pdgPi0);
      Correlators30.push_back(c1);
  }
}

// The following will book the histograms for Figure 26 
for(pta2 = 0; pta2<numpTAssocBins2; pta2++){
      Correlator c1(pta,ptt,-1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(3, 4);
      c1.SetAssociatedRange(pTAssocBins2[pta2],7);
      //c1.SetPID(pdgPi0);
      Correlators26.push_back(c1);

      Correlator c2(pta,ptt,-1);
      c2.SetCollSystemAndEnergy("AuAu200GeV");
      c2.SetNoCentrality();
      c2.SetTriggerRange(4, 5);
      c2.SetAssociatedRange(pTAssocBins2[pta2],7);
      //c1.SetPID(pdgPi0);
      Correlators26.push_back(c2);
}

// The following will book the histograms for Figure 25 
for(ptt = 0; ptt<numTrigPtBins; ptt++){
      Correlator c1(pta,ptt,-1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(1,5);
      //c1.SetPID(pdgPi0);
      Correlators25.push_back(c1);

      Correlator c2(pta,ptt,-1);
      c2.SetCollSystemAndEnergy("AuAu200GeV");
      c2.SetNoCentrality();
      c2.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c2.SetAssociatedRange(1,5);
      //c1.SetPID(pdgPi0);
      Correlators25.push_back(c2);
}

// The following will book the histograms for Figure 24 
for(ptt = 0; ptt<2; ptt++){
      Correlator c1(1,1,1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetCentrality(0,20);
      c1.SetTriggerRange(2, 3);
      c1.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators24.push_back(c1);

      Correlator c2(2,2,1);
      c2.SetCollSystemAndEnergy("AuAu200GeV");
      c2.SetCentrality(0,20);
      c2.SetTriggerRange(3, 4);
      c2.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators24.push_back(c2);

      Correlator c3(3,3,1);
      c3.SetCollSystemAndEnergy("AuAu200GeV");
      c3.SetCentrality(0,20);
      c3.SetTriggerRange(3, 4);
      c3.SetAssociatedRange(3,5);
      //c1.SetPID(pdgPi0);
      Correlators24.push_back(c3);

      Correlator c4(4,4,1);
      c4.SetCollSystemAndEnergy("AuAu200GeV");
      c4.SetCentrality(0,20);
      c4.SetTriggerRange(5,10);
      c4.SetAssociatedRange(5,10);
      //c1.SetPID(pdgPi0);
      Correlators24.push_back(c4);

}

// The following will book the histograms for Figure 23 
for(ptt = 0; ptt<1; ptt++){
      Correlator c1(1,ptt,1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(2, 3);
      c1.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c1);

      Correlator c2(2,ptt,1);
      c2.SetCollSystemAndEnergy("AuAu200GeV");
      c2.SetNoCentrality();
      c2.SetTriggerRange(3, 4);
      c2.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c2);

      Correlator c3(3,ptt,1);
      c3.SetCollSystemAndEnergy("AuAu200GeV");
      c3.SetNoCentrality();
      c3.SetTriggerRange(3, 4);
      c3.SetAssociatedRange(3,5);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c3);

      Correlator c4(4,ptt,1);
      c4.SetCollSystemAndEnergy("AuAu200GeV");
      c4.SetNoCentrality();
      c4.SetTriggerRange(5,10);
      c4.SetAssociatedRange(5,10);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c4);

      Correlator c5(5,5,1);
      c5.SetCollSystemAndEnergy("AuAu200GeV");
      c5.SetCentrality(0,20);
      c5.SetTriggerRange(2, 3);
      c5.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c5);

      Correlator c6(6,6,1);
      c6.SetCollSystemAndEnergy("AuAu200GeV");
      c6.SetCentrality(0,20);
      c6.SetTriggerRange(3, 4);
      c6.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c6);

      Correlator c7(7,7,1);
      c7.SetCollSystemAndEnergy("AuAu200GeV");
      c7.SetCentrality(0,20);
      c7.SetTriggerRange(3, 4);
      c7.SetAssociatedRange(3,5);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c7);

      Correlator c8(8,8,1);
      c8.SetCollSystemAndEnergy("AuAu200GeV");
      c8.SetCentrality(0,20);
      c8.SetTriggerRange(5,10);
      c8.SetAssociatedRange(5,10);
      //c1.SetPID(pdgPi0);
      Correlators23.push_back(c8);

}

// The following will book the histograms for Figure 18 

for(ptt = 0; ptt<numTrigPtBins; ptt++){
      Correlator c1(-1,ptt,-1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(1,5);
      //c1.SetPID(pdgPi0);
      Correlators18.push_back(c1);

      Correlator c2(1,ptt,1);
      c2.SetCollSystemAndEnergy("AuAu200GeV");
      c2.SetCentrality(0,20);
      c2.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c2.SetAssociatedRange(1,5); //FIXME on all of these... 
      //c1.SetPID(pdgPi0);
      Correlators18.push_back(c2);
}

// The following will book the histograms for Figure 23 
for(ptt = 0; ptt<1; ptt++){
      Correlator c1(1,ptt,1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(2, 3);
      c1.SetAssociatedRange(0.4,1);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c1);

      Correlator c2(2,ptt,1);
      c2.SetCollSystemAndEnergy("AuAu200GeV");
      c2.SetNoCentrality();
      c2.SetTriggerRange(2,3);
      c2.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c2);

      Correlator c3(3,ptt,1);
      c3.SetCollSystemAndEnergy("AuAu200GeV");
      c3.SetNoCentrality();
      c3.SetTriggerRange(3, 4);
      c3.SetAssociatedRange(3,4);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c3);

      Correlator c4(4,ptt,1);
      c4.SetCollSystemAndEnergy("AuAu200GeV");
      c4.SetNoCentrality();
      c4.SetTriggerRange(4,5);
      c4.SetAssociatedRange(4,5);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c4);

      Correlator c5(5,ptt,1);
      c5.SetCollSystemAndEnergy("AuAu200GeV");
      c5.SetNoCentrality();
      c5.SetTriggerRange(5,10);
      c5.SetAssociatedRange(5,10);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c5);

      Correlator c6(1,ptt,1);
      c6.SetCollSystemAndEnergy("AuAu200GeV");
      c6.SetCentrality(0,20);
      c6.SetTriggerRange(2, 3);
      c6.SetAssociatedRange(0.4,1);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c6);

      Correlator c7(2,ptt,1);
      c7.SetCollSystemAndEnergy("AuAu200GeV");
      c7.SetCentrality(0,20);
      c7.SetTriggerRange(2,3);
      c7.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c7);

      Correlator c8(3,ptt,1);
      c8.SetCollSystemAndEnergy("AuAu200GeV");
      c8.SetCentrality(0,20);
      c8.SetTriggerRange(3, 4);
      c8.SetAssociatedRange(3,4);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c8);

      Correlator c9(4,ptt,1);
      c9.SetCollSystemAndEnergy("AuAu200GeV");
      c9.SetCentrality(0,20);
      c9.SetTriggerRange(4,5);
      c9.SetAssociatedRange(4,5);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c9);

      Correlator c10(5,ptt,1);
      c10.SetCollSystemAndEnergy("AuAu200GeV");
      c10.SetCentrality(0,20);
      c10.SetTriggerRange(5,10);
      c10.SetAssociatedRange(5,10);
      //c1.SetPID(pdgPi0);
      Correlators16.push_back(c10);

}

int i;
// The following will book the histograms for Figure 12
for(ptt = 0; ptt<numTrigPtBins; ptt++){
  for(i=0;i<3;i++){
      Correlator c1(-1,ptt,-1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(1,5); //FIXME 2
      //c1.SetPID(pdgPi0);
      Correlators12.push_back(c1);

    for(cb = 0; cb<numCentBins; cb++){
        Correlator c1(-1,ptt,cb);
        c1.SetCollSystemAndEnergy("AuAu200GeV");
        c1.SetCentrality(CentBins[cb],CentBins[cb+1]);
        c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
        c1.SetAssociatedRange(1,5);
        //c1.SetPID(pdgPi0);
        Correlators12.push_back(c1);
    }  
  }
}

// The following will book the histograms for Figure 11
for(i=0;i<2;i++){
  for(ptt = 0; ptt<numTrigPtBins-1; ptt++){
      Correlator c1(-1,ptt,i);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetCentrality(0,20);
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(1,5);
      //c1.SetPID(pdgPi0);
      Correlators11.push_back(c1);
  }
}

// The following will book the histograms for Figure 10, where i represents FIT number 
for(i=0;i<3;i++){
      Correlator c1(1,1,i);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(2,3);
      c1.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators10.push_back(c1);
}

// The following will book the histograms for Figure 9 
for(i=0;i<1;i++){
      Correlator c1(1,1,-1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetCentrality(0,1);
      c1.SetTriggerRange(2,3);
      c1.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators10.push_back(c1);
}

// The following will book the histograms for Figure 8 
for(i=0;i<1;i++){
      Correlator c1(1,1,-1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(2,3);
      c1.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators8.push_back(c1);
}

// The following will book the histograms for Figure 7 

for(ptt = 0; ptt<numTrigPtBins; ptt++){
      Correlator c1(1,ptt,-1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(1,5);
      //c1.SetPID(pdgPi0);
      Correlators7.push_back(c1);

      Correlator c2(1,ptt,1);
      c2.SetCollSystemAndEnergy("AuAu200GeV");
      c2.SetCentrality(0,20);
      c2.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c2.SetAssociatedRange(1,5); //FIXME on all of these... 
      //c1.SetPID(pdgPi0);
      Correlators7.push_back(c2);
}

// The following will book the histograms for Figure 6 
for(i=0;i<1;i++){
  for(pta = 0; pta<4; pta++){
   for(ptt = 0; ptt<4; ptt++){

      Correlator c1(pta,ptt,i);
      c1.SetCollSystemAndEnergy("pp200GeV");
      c1.SetNoCentrality();
      c1.SetTriggerRange(3,4);
      c1.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c1);

    }
   } 

      Correlator c5(5,ptt,1);
      c5.SetCollSystemAndEnergy("AuAu200GeV");
      c5.SetNoCentrality();
      c5.SetTriggerRange(5,10);
      c5.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c5);

      Correlator c6(6,ptt,1);
      c6.SetCollSystemAndEnergy("AuAu200GeV");
      c6.SetNoCentrality();
      c6.SetTriggerRange(4,5);
      c6.SetAssociatedRange(4,5);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c6);

      Correlator c7(7,ptt,1);
      c7.SetCollSystemAndEnergy("AuAu200GeV");
      c7.SetNoCentrality();
      c7.SetTriggerRange(5,10);
      c7.SetAssociatedRange(3,5);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c7);

      Correlator c8(8,ptt,1);
      c8.SetCollSystemAndEnergy("AuAu200GeV");
      c8.SetNoCentrality();
      c8.SetTriggerRange(5,10);
      c8.SetAssociatedRange(5,10);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c8);

  for(pta = 0; pta<4; pta++){
   for(ptt = 0; ptt<4; ptt++){

      Correlator c9(pta,ptt,1);
      c9.SetCollSystemAndEnergy("pp200GeV");
      c9.SetCentrality(0,20);
      c9.SetTriggerRange(3,4);
      c9.SetAssociatedRange(pTAssocBins[pta],pTAssocBins[pta+1]);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c9);

    }
   } 

      Correlator c11(11,ptt,1);
      c11.SetCollSystemAndEnergy("AuAu200GeV");
      c11.SetCentrality(0,20);
      c11.SetTriggerRange(5,10);
      c11.SetAssociatedRange(2,3);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c11);

      Correlator c12(12,ptt,1);
      c12.SetCollSystemAndEnergy("AuAu200GeV");
      c12.SetCentrality(0,20);
      c12.SetTriggerRange(4,5);
      c12.SetAssociatedRange(4,5);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c12);

      Correlator c13(13,ptt,1);
      c13.SetCollSystemAndEnergy("AuAu200GeV");
      c13.SetCentrality(0,20);
      c13.SetTriggerRange(5,10);
      c13.SetAssociatedRange(3,5);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c13);

      Correlator c14(14,ptt,1);
      c14.SetCollSystemAndEnergy("AuAu200GeV");
      c14.SetCentrality(0,20);
      c14.SetTriggerRange(5,10);
      c14.SetAssociatedRange(5,10);
      //c1.SetPID(pdgPi0);
      Correlators6.push_back(c14);
}

// The following will book the histograms for Figure 4, where i is the NS,AS,AH 
for(i=0;i<3;i++){
  for(ptt = 0; ptt<numTrigPtBins-1; ptt++){
      Correlator c1(i,ptt,1);
      c1.SetCollSystemAndEnergy("AuAu200GeV");
      c1.SetCentrality(0,20);
      c1.SetTriggerRange(pTTrigBins[ptt], pTTrigBins[ptt+1]);
      c1.SetAssociatedRange(1,5);
      //c1.SetPID(pdgPi0);
      Correlators7.push_back(c1);
  }
}


//for(Correlator& corr : CorrelatorsB)
  //{
    //    book(_h["0" + to_string(corr.GetIndex()) + "11"], corr.GetIndex(), 1, 1);
    //    book(sow[corr.GetIndex()],"sow" + to_string(corr.GetIndex()));
    //    nTriggers[corr.GetIndex()] = 0;
   // }


    }
    void analyze(const Event& event) {
      const ChargedFinalState& cfs = apply<ChargedFinalState>(event, "CFS");
      const PrimaryParticles& ppTrigPi0 = apply<PrimaryParticles>(event, "PP");
      const PromptFinalState& pfsTrigPhotons = apply<PromptFinalState>(event, "PFS");
      //==================================================
      // Select the histograms accordingly to the collision system, beam energy and centrality
      // WARNING: Still not implemented for d-Au
      //==================================================
      double nNucleons = 0.;
      string CollSystem = "Empty";
      const ParticlePair& beam = beams();
          CollSystem = "pp";
          nNucleons = 1.;
      //if (beam.first.pid() == 1000290630 && beam.second.pid() == 1000010020) CollSystem = "dAu";
      //if (beam.first.pid() == 1000010020 && beam.second.pid() == 1000290630) CollSystem = "dAu";
    
      string cmsEnergy = "Empty";
      if (fuzzyEquals(sqrtS()/GeV, 200*nNucleons, 1E-3)) cmsEnergy = "200GeV";
    
      string SysAndEnergy = CollSystem + cmsEnergy;
   
      double triggerptMin = 999.;
      double triggerptMax = -999.;
      double associatedptMin = 999.;
      double associatedptMax = -999.;
   
      bool isVeto = true;
   
      for(Correlator& corr : Correlators) // Think about adding for Correlators38 etc. FIXME 
      {
        if(!corr.CheckCollSystemAndEnergy(SysAndEnergy)) continue;
        //if(!corr.CheckCentrality(centr)) continue;
       
        //If event is accepted for the correlator, fill event weights
        sow[corr.GetFullIndex()]->fill();
       
        isVeto = false;
       
        //Check min and max of the trigger and associated particles in order to speed up the particle loops
        if(corr.GetTriggerRangeMin() < triggerptMin) triggerptMin = corr.GetTriggerRangeMin();
        if(corr.GetTriggerRangeMax() > triggerptMax) triggerptMax = corr.GetTriggerRangeMax();
       
        if(corr.GetAssociatedRangeMin() < associatedptMin) associatedptMin = corr.GetAssociatedRangeMin();
        if(corr.GetAssociatedRangeMax() > associatedptMax) associatedptMax = corr.GetAssociatedRangeMax();
      }
   
      if(isVeto) vetoEvent;
    }

    void finalize() {
    }
 	
 	map<string, Histo1DPtr> _h;
    map<string, CounterPtr> sow;
    map<string, Histo1DPtr> _DeltaPhixE;
    map<int, Histo1DPtr> _DeltaPhiSub;
    map<string, int> nTriggers;
    vector<Correlator> Correlators;
    vector<Correlator> Correlators38;
    vector<Correlator> Correlators31;
    vector<Correlator> Correlators30;
    vector<Correlator> Correlators26;
    vector<Correlator> Correlators25;
    vector<Correlator> Correlators24;
    vector<Correlator> Correlators23;
    vector<Correlator> Correlators18;
    vector<Correlator> Correlators16;
    vector<Correlator> Correlators12;
    vector<Correlator> Correlators11;
    vector<Correlator> Correlators10;
    vector<Correlator> Correlators9;
    vector<Correlator> Correlators8;
    vector<Correlator> Correlators7;
    vector<Correlator> Correlators6;
    vector<Correlator> Correlators4;

    std::initializer_list<int> pdgPi0 = {111, -111};  // Pion 0
    std::initializer_list<int> pdgPhoton = {22};  // Pion 0

  };


  DECLARE_RIVET_PLUGIN(PHENIX_2008_I778396);

}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   