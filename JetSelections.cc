#include "JetSelections.h"
#include "ElectronSelections.h"
#include "MuonSelections.h" 
#include "Math/VectorUtil.h"

using namespace tas;

bool isLoosePFJet(unsigned int pfJetIdx){

  float pfjet_chf_  = pfjets_chargedHadronE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());
  float pfjet_nhf_  = pfjets_neutralHadronE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());
  float pfjet_cef_  = pfjets_chargedEmE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());
  float pfjet_nef_  = pfjets_neutralEmE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());
  int   pfjet_cm_  = pfjets_chargedMultiplicity()[pfJetIdx];
  float pfjet_eta  = fabs(pfjets_p4()[pfJetIdx].eta());

  if (pfjets_pfcandIndicies()[pfJetIdx].size() < 2) return false;
  if (pfjet_nef_ >= 0.99) return false;
  if (pfjet_nhf_ >= 0.99) return false;

  if (pfjet_eta < 2.4){
    if (pfjet_cm_ < 1) return false;
    if (pfjet_chf_ < 1e-6) return false;
    if (pfjet_cef_ >= 0.99) return false;
  }

  return true;
}

bool isMediumPFJet(unsigned int pfJetIdx){

  float pfjet_nhf_  = pfjets_neutralHadronE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());
  float pfjet_nef_  = pfjets_neutralEmE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());

  if (pfjet_nef_ >= 0.95) return false;
  if (pfjet_nhf_ >= 0.95) return false;

  if (!isLoosePFJet(pfJetIdx)) return false;

  return true;
}

bool isTightPFJet(unsigned int pfJetIdx){

  float pfjet_nhf_  = pfjets_neutralHadronE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());
  float pfjet_nef_  = pfjets_neutralEmE()[pfJetIdx] / (pfjets_undoJEC().at(pfJetIdx)*pfjets_p4()[pfJetIdx].energy());

  if (pfjet_nef_ >= 0.90) return false;
  if (pfjet_nhf_ >= 0.90) return false;

  if (!isLoosePFJet(pfJetIdx)) return false;

  return true;
}

bool loosePileupJetId(unsigned int pfJetIdx){

  float eta = fabs(pfjets_p4().at(pfJetIdx).eta());
  float value = pfjets_pileupJetId().at(pfJetIdx);

  if( (eta >= 0   ) && (eta <= 2.5 ) && (value > -0.63) ) return true;
  if( (eta > 2.5  ) && (eta <= 2.75) && (value > -0.60) ) return true;
  if( (eta > 2.75 ) && (eta <= 3.0 ) && (value > -0.55) ) return true;
  if( (eta > 3.0  ) && (eta <= 5.2 ) && (value > -0.45) ) return true;
    
  return false;
}

bool JetIsElectron(LorentzVector pfJet, id_level_t id_level, float ptcut, float deltaR){
  bool jetIsLep = false;
  for (unsigned int eidx = 0; eidx < tas::els_p4().size(); eidx++){
    LorentzVector electron = tas::els_p4().at(eidx);
    if (electron.pt() < ptcut) continue;
    if (!electronID(eidx,id_level)) continue;
    if (ROOT::Math::VectorUtil::DeltaR(pfJet, electron) > deltaR) continue;
    jetIsLep = true;
  }
  return jetIsLep;
}

bool JetIsMuon(LorentzVector pfJet, id_level_t id_level, float ptcut, float deltaR){
  bool jetIsLep = false;
  for (unsigned int muidx = 0; muidx < tas::mus_p4().size(); muidx++){
    LorentzVector muon = tas::mus_p4().at(muidx);
    if (muon.pt() < ptcut) continue;
    if (!muonID(muidx,id_level)) continue;
    if (ROOT::Math::VectorUtil::DeltaR(pfJet, muon) > deltaR) continue;
    jetIsLep = true;
  }
  return jetIsLep;
}
