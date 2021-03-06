#include "MuonSelections.h"
#include "Math/VectorUtil.h"
#include "IsolationTools.h"

bool muIDCacheSet = false;
muIDcache muID_cache;
void muID::setCache(int idx, float miniiso, float ptratio, float ptrel) {
  assert(muIDCacheSet==false);//you must unset it before setting it again
  muID_cache.setCacheValues(idx,miniiso,ptratio,ptrel);
  muIDCacheSet=true;
}
void muID::unsetCache() {
  muIDCacheSet=false;
}

using namespace tas;

bool isMediumMuonPOG_forICHEP( unsigned int muIdx ){
  bool isGlobal  = true;
  if (((mus_type().at(muIdx)) & (1<<1)) == 0) isGlobal  = false;
  bool goodGlb = isGlobal && mus_gfit_chi2().at(muIdx)/get_mus_gfit_ndof(muIdx)<3. && 
                 mus_chi2LocalPosition().at(muIdx)<12. && mus_trkKink().at(muIdx)<20.;
  double validFraction = mus_validHits().at(muIdx)/(double)(mus_validHits().at(muIdx)+mus_lostHits().at(muIdx)+mus_exp_innerlayers().at(muIdx)+mus_exp_outerlayers().at(muIdx));
  bool good = isLooseMuonPOG(muIdx) && validFraction > 0.49 && mus_segmCompatibility().at(muIdx) >= (goodGlb ? 0.303 : 0.451);
  return good;
}

bool isLooseMuonPOG(unsigned int muIdx){
  if (!mus_pid_PFMuon().at(muIdx)) return false;    
  bool isGlobal  = true;
  bool isTracker = true;
  if (((mus_type().at(muIdx)) & (1<<1)) == 0) isGlobal  = false;
  if (((mus_type().at(muIdx)) & (1<<2)) == 0) isTracker = false;
  if (!(isGlobal || isTracker)) return false;  
  return true;
}

bool isMediumMuonPOG(unsigned int muIdx){
  bool isGlobal  = true;
  if (((mus_type().at(muIdx)) & (1<<1)) == 0) isGlobal  = false;
  bool goodGlb = isGlobal && mus_gfit_chi2().at(muIdx)/get_mus_gfit_ndof(muIdx)<3. && 
                 mus_chi2LocalPosition().at(muIdx)<12. && mus_trkKink().at(muIdx)<20.;
  double validFraction = mus_validHits().at(muIdx)/(double)(mus_validHits().at(muIdx)+mus_lostHits().at(muIdx)+mus_exp_innerlayers().at(muIdx)+mus_exp_outerlayers().at(muIdx));
  bool good = isLooseMuonPOG(muIdx) && validFraction > 0.8 &&  mus_segmCompatibility().at(muIdx) >= (goodGlb ? 0.303 : 0.451);
  return good;
}

bool isTightMuonPOG(unsigned int muIdx){
  if (!mus_pid_PFMuon()            .at(muIdx)         ) return false;    
  if (((mus_type()                 .at(muIdx))
	   & (1<<1)) == 0                                 ) return false;//global muon
  if (mus_gfit_chi2()              .at(muIdx)		 
	  /get_mus_gfit_ndof          (muIdx)  >= 10  ) return false; 
  if (mus_gfit_validSTAHits()      .at(muIdx)  == 0   ) return false; 
  if (mus_numberOfMatchedStations().at(muIdx)  <  2   ) return false;
  if (mus_validPixelHits()         .at(muIdx)  == 0   ) return false;
  if (mus_nlayers()                .at(muIdx)  <  6   ) return false;
  if (fabs(mus_dxyPV()             .at(muIdx)) >  0.2 ) return false;
  if (fabs(mus_dzPV()              .at(muIdx)) >  0.5 ) return false;
  return true;
}

bool isHighPtMuonPOG(unsigned int muIdx){
  if (((mus_type()                 .at(muIdx))
	   & (1<<1))                               == 0   ) return false;//global muon
  if (mus_gfit_validSTAHits()      .at(muIdx)      == 0   ) return false; 
  if (mus_numberOfMatchedStations().at(muIdx)      <  2   ) return false;
  if (mus_bfit_ptErr()             .at(muIdx)
           /mus_bfit_p4()          .at(muIdx).pt() > 0.3  ) return false;
  if (fabs(mus_dxyPV()             .at(muIdx))     > 0.2  ) return false; //should be using best fit tracks
  if (fabs(mus_dzPV()              .at(muIdx))     > 0.5  ) return false; //should be using best fit tracks
  if (mus_validPixelHits()         .at(muIdx)      == 0   ) return false;
  if (mus_nlayers()                .at(muIdx)      <  6   ) return false;

  return true;
}

// Read bit MuonPOG IDs from muon selectors from (available starting from 94X)
bool passesMuonPOG(muID::Selector selection, int muIdx) {
  return (mus_selectors().at(muIdx) & selection) == selection;
}

// from https://github.com/gpetruc/cmssw/blob/badMuonFilters_80X/RecoMET/METFilters/plugins/BadGlobalMuonTagger.cc
//   with selectClones_ = false
bool isBadGlobalMuon(unsigned int muIdx, bool selectClones){
  if (!mus_pid_PFMuon().at(muIdx)) return false;//discard if not PF muon
  if (mus_algo().at(muIdx) < 0) return false; // only negative if there is no inner track
  bool isGlobalMuon = bool(((mus_type().at(muIdx)) & (1<<1)) > 0);
  // HACK: should be checking algoMask size and entry when available
  bool outInOnly = bool(mus_algo().at(muIdx) == 14 && mus_algoOrig().at(muIdx) == 14);
  bool preselection = bool(isGlobalMuon && (!selectClones || outInOnly));
  if (!preselection) return false;
  // HACK: should replace numberOfMatchedStations with muonStationsWithValidHits when available
  bool tightGlobal = bool(mus_numberOfMatchedStations().at(muIdx) >= 3 && mus_gfit_chi2().at(muIdx)/get_mus_gfit_ndof(muIdx) <= 20.);
  // HACK: should replace validPixelHits with pixelLayersWithMeasurement when available
  bool ipLoose = bool((fabs(mus_dxyPV().at(muIdx)) < 0.5 && fabs(mus_dzPV().at(muIdx)) < 2.0) || (mus_validPixelHits().at(muIdx) >= 2));
  bool ipTight = bool(fabs(mus_dxyPV().at(muIdx)) < 0.2 && fabs(mus_dzPV().at(muIdx)) < 0.5);
  bool tighterId = bool(isMediumMuonPOG(muIdx) && mus_numberOfMatchedStations().at(muIdx) >= 2);
  bool safeId = bool((mus_bfit_ptErr().at(muIdx) < 0.2 * mus_bfit_p4().at(muIdx).pt()) && ((mus_numberOfMatchedStations().at(muIdx) >= 1) || tightGlobal));
  if (tighterId) {
    return (!(ipLoose || (!selectClones && tightGlobal))); // muon is OK if it passes either selection
  }
  else if (safeId) {
    return !ipTight; // muon is OK if it passes ipTight
  }
  // PF and global muon but doesn't pass any of the other selections: flag as bad
  return true;
}    

bool muonID(unsigned int muIdx, id_level_t id_level){

  analysis_t analysis = whichAnalysis(id_level);

  switch (id_level){

   ///////////////////
   /// SS veto v1, v2 ///
   ///////////////////
  
    case(SS_veto_noiso_v1):
    case(SS_veto_noiso_v2):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      return isLooseMuonPOG(muIdx);
      break;

    case(SS_veto_v1):
    case(SS_veto_v2):
      if (muonID(muIdx, SS_veto_noiso_v1)==0) return false;
      if (muRelIso03(muIdx, SS) > 0.50) return false;
      return true;
      break;

   ///////////////////
   /// SS veto v3 ///
   ///////////////////
  
    case(SS_veto_noiso_v3):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

    case(SS_veto_v3):
      if (muonID(muIdx, SS_veto_noiso_v3)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx) > 0.40) return false;
      return true;
      break;

    case(SS_veto_noiso_noip_v3):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      //if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

   ///////////////////
   /// SS veto v4 ///
   ///////////////////
  
    case(SS_veto_noiso_v4):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

    case(SS_veto_v4):
      if (muonID(muIdx, SS_veto_noiso_v4)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,1) > 0.40) return false;
      return true;
      break;

    case(SS_veto_noiso_noip_v4):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      //if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

   ///////////////////
   /// SS veto v5 ///
   ///////////////////
  
    case(SS_veto_noiso_v5):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

    case(SS_veto_v5):
      if (muonID(muIdx, SS_veto_noiso_v5)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.40) return false;
      return true;
      break;

    case(SS_veto_noiso_noip_v5):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      //if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

   ///////////////////
   /// SS veto v6 ///
   ///////////////////
  
    case(SS_veto_noiso_v6):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdLoose, muIdx)    ) return false;
      } else {
        if (!isLooseMuonPOG(muIdx)                          ) return false;
      }
      break;

    case(SS_veto_v6):
      if (muonID(muIdx, SS_veto_noiso_v6)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.40) return false;
      return true;
      break;

    case(SS_veto_noiso_noip_v6):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      //if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdLoose, muIdx)    ) return false;
      } else {
        if (!isLooseMuonPOG(muIdx)                          ) return false;
      }
      break;

   ///////////////////
   /// WW veto v1 ///
   ///////////////////
  
    case(WW_veto_noiso_v1):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

    case(WW_veto_v1):
      if (muonID(muIdx, WW_veto_noiso_v1)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx) > 0.40) return false;
      return true;
      break;

    case(WW_veto_noiso_noip_v1):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      return isLooseMuonPOG(muIdx);
      break;

   ///////////////////
   /// WW veto v2 ////
   ///////////////////
  
    case(WW_veto_noiso_v2):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.02) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      return isLooseMuonPOG(muIdx);
      break;

    case(WW_veto_v2):
      if (muonID(muIdx, WW_veto_noiso_v2)==0) return false;
      if (muRelIso04DB(muIdx) > 0.40) return false;
      return true;
      break;

   ////////////////////
   /// HAD loose v1 ///
   ////////////////////
  
    case(HAD_loose_noiso_v1):
    case(HAD_loose_noiso_v2):
    case(HAD_loose_noiso_v3):
      if (!isLooseMuonPOG(muIdx)) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.5) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 1.0) return false;
      return true;
      break;

    case(HAD_loose_noiso_v4):
      if (!isLooseMuonPOG(muIdx)) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.2) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.5) return false;
      return true;
      break;

    case(HAD_loose_v1):
      if (muonID(muIdx, HAD_loose_noiso_v1)==0) return false;
      if (muRelIso03(muIdx, analysis) >= 0.15) return false; 
      return true;
      break;

   ////////////////////
   /// HAD loose v2 ///
   ////////////////////
  
    // // now using mini iso
    // case(HAD_loose_v2):
    //   if (muonID(muIdx, HAD_loose_noiso_v2)==0) return false;
    //   if (muMiniRelIso(muIdx) > 0.2) return false;
    //   return true;
    //   break;

   ////////////////////
   /// HAD loose v3 ///
   ////////////////////

    // same ID as v2, but use CMS3 miniIso with EA corrections
    case(HAD_loose_v3):
      if (muonID(muIdx, HAD_loose_noiso_v3)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx) > 0.2) return false;
      return true;
      break;

   ////////////////////
   /// HAD loose v4 ///
   ////////////////////

    // same ID as v2 and v3, use updated EA values
    case(HAD_loose_v4):
      if (muonID(muIdx, HAD_loose_noiso_v4)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.2) return false;
      return true;
      break;

   ///////////////////
   /// SS FO v1 ///  same as tight, but no SIP3D cut and looser iso
   ///////////////////
  
    case(SS_fo_noiso_v1):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (!mus_pid_PFMuon().at(muIdx)) return false;    
      if (((mus_type().at(muIdx)) & (1<<1)) == 0) return false;//global muon
      if (mus_gfit_chi2().at(muIdx)/get_mus_gfit_ndof(muIdx) >= 10) return false; 
      if (mus_gfit_validSTAHits().at(muIdx) == 0) return false; 
      if (mus_numberOfMatchedStations().at(muIdx) < 2) return false;
      if (mus_validPixelHits().at(muIdx) == 0) return false;
      if (mus_nlayers().at(muIdx) < 6) return false;
      //if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      break;
 
   case(SS_fo_v1):
      if (muonID(muIdx, SS_fo_noiso_v1)==0) return false;
      if (muRelIso03(muIdx, SS) > 0.50) return false;
      return true;
      break;

  
   ///////////////////
   /// SS FO v2   ///  same as tight, but no SIP3D cut and looser iso
   ///////////////////

    case(SS_fo_noiso_v2):
      if (muonID(muIdx, SS_veto_noiso_v2)==0) return false;//make sure it's tighter than veto
      //if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_fo_v2):
      if (muonID(muIdx, SS_fo_noiso_v2)==0) return false;
      if (muRelIso03(muIdx, SS) > 0.50) return false;
      return true;
      break;

   ///////////////////
   /// SS FO v3   ///  same as tight, but looser iso
   ///////////////////

    case(SS_fo_noiso_v3):
      if (!muonID(muIdx, SS_veto_noiso_v3)) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

    case(SS_fo_noiso_noip_v3):
      if (!muonID(muIdx, SS_veto_noiso_noip_v3)) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_fo_v3):
      if (!muonID(muIdx, SS_fo_noiso_v3)) return false;
      if (muMiniRelIsoCMS3_EA(muIdx) > 0.40) return false;
      return true;
      break;

   ///////////////////
   /// SS FO v4   ///  same as tight, but looser iso
   ///////////////////

    case(SS_fo_noiso_v4):
      if (!muonID(muIdx, SS_veto_noiso_v4)) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

    case(SS_fo_noiso_noip_v4):
      if (!muonID(muIdx, SS_veto_noiso_noip_v4)) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_fo_v4):
      if (!muonID(muIdx, SS_fo_noiso_v4)) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,1) > 0.40) return false;
      return true;
      break;

   ///////////////////
   /// SS FO v5   ///  same as tight, but looser iso
   ///////////////////

    case(SS_fo_noiso_v5):
      if (!muonID(muIdx, SS_veto_noiso_v5)) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

    case(SS_fo_noiso_noip_v5):
      if (!muonID(muIdx, SS_veto_noiso_noip_v5)) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_fo_v5):
      if (!muonID(muIdx, SS_fo_noiso_v5)) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.40) return false;
      return true;
      break;

   ///////////////////
   /// SS FO v6   ///  same as tight, but looser iso
   ///////////////////

    case(SS_fo_noiso_v6):
      if (!muonID(muIdx, SS_veto_noiso_v6)) return false;
      if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdMedium, muIdx)   ) return false;
      } else {
        if (!isMediumMuonPOG(muIdx)                         ) return false;
      }
      break;

    case(SS_fo_noiso_noip_v6):
      if (!muonID(muIdx, SS_veto_noiso_noip_v6)) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdMedium, muIdx)   ) return false;
      } else {
        if (!isMediumMuonPOG(muIdx)                         ) return false;
      }
      break;

   case(SS_fo_v6):
      if (!muonID(muIdx, SS_fo_noiso_v6)) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.40) return false;
      return true;
      break;


   ///////////////////
   /// WW FO v1    ///  same as tight, but looser iso
   ///////////////////

    case(WW_fo_noiso_v1):
      if (!muonID(muIdx, WW_veto_noiso_v1)) return false;
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      return isMediumMuonPOG(muIdx);
      break;

    case(WW_fo_noiso_noip_v1):
      if (!muonID(muIdx, WW_veto_noiso_noip_v1)) return false;
       return isMediumMuonPOG(muIdx);
      break;

   case(WW_fo_v1):
      if (!muonID(muIdx, WW_fo_noiso_v1)) return false;
      if (muMiniRelIsoCMS3_EA(muIdx) > 0.40) return false;
      return true;
      break;
    
   ///////////////////
   /// WW FO v2    ///  same as tight, but looser iso
   ///////////////////

    case(WW_fo_noiso_v2):
      if (muonID(muIdx, WW_veto_noiso_v2)==0) return false;//make sure it's tighter than veto
      if (!isMediumMuonPOG(muIdx)) return false;
      return true;
      break;

   case(WW_fo_v2):
      if (muonID(muIdx, WW_veto_v2)==0) return false;//make sure it's tighter than veto
      if (mus_iso03_sumPt().at(muIdx)/mus_p4().at(muIdx).pt() > 0.4) return false;
      if (muRelIso04DB(muIdx) > 0.40) return false;
      if (!isMediumMuonPOG(muIdx)) return false;
      return true;
      break;

   ///////////////////
   /// WW FO v3    ///  
   ///////////////////

  case(WW_fo_noiso_v3):
    if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
    if (mus_p4().at(muIdx).pt() > 20. ? fabs(mus_dxyPV().at(muIdx)) > 0.02 : fabs(mus_dxyPV().at(muIdx)) > 0.01) return false;
    if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
    if (!isMediumMuonPOG(muIdx)) return false;
    return true;
    break;
    
  case(WW_fo_v3):
    if (muonID(muIdx, WW_fo_noiso_v3)==0) return false;
    if (mus_iso03_sumPt().at(muIdx)/mus_p4().at(muIdx).pt() > 0.4) return false;
    if (muRelIso04DB(muIdx) > 0.40) return false;
    return true;
    break;

   ///////////////////
   /// SS tight v1 ///
   ///////////////////
  
    case(SS_tight_noiso_v1):
      if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
      if (!mus_pid_PFMuon().at(muIdx)) return false;    
      if (((mus_type().at(muIdx)) & (1<<1)) == 0) return false;//global muon
      if (mus_gfit_chi2().at(muIdx)/get_mus_gfit_ndof(muIdx) >= 10) return false; 
      if (mus_gfit_validSTAHits().at(muIdx) == 0) return false; 
      if (mus_numberOfMatchedStations().at(muIdx) < 2) return false;
      if (mus_validPixelHits().at(muIdx) == 0) return false;
      if (mus_nlayers().at(muIdx) < 6) return false;
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      break;
 
   case(SS_tight_v1):
      if (muonID(muIdx, SS_tight_noiso_v1)==0) return false;
      if (muRelIso03(muIdx, SS) > 0.10) return false;
      return true;
      break;

   ///////////////////
   /// SS tight v2 ///
   ///////////////////
  
    case(SS_tight_noiso_v2):
      if (muonID(muIdx, SS_fo_noiso_v2)==0) return false;//make sure it's tighter than FO
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_tight_v2):
      if (muonID(muIdx, SS_tight_noiso_v2)==0) return false;
      if (muRelIso03(muIdx, SS) > 0.10) return false;
      return true;
      break;

   ///////////////////
   /// SS tight v3 ///
   ///////////////////
  
    case(SS_tight_noiso_v3):
      if (muonID(muIdx, SS_fo_noiso_v3)==0) return false;//make sure it's tighter than FO
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_tight_v3):
      if (muonID(muIdx, SS_tight_noiso_v3)==0) return false;
      if (muIDCacheSet) return passMultiIsoCuts(0.14, 0.68, 6.7, muID_cache.getMiniiso(muIdx), muID_cache.getPtratio(muIdx), muID_cache.getPtrel(muIdx) );
      else return passMultiIso(13, muIdx, 0.14, 0.68, 6.7, 0, 0);
      break;

   ///////////////////
   /// SS tight v4 ///
   ///////////////////
  
    case(SS_tight_noiso_v4):
      if (muonID(muIdx, SS_fo_noiso_v4)==0) return false;//make sure it's tighter than FO
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_tight_v4):
      if (muonID(muIdx, SS_tight_noiso_v4)==0) return false;
      if (muIDCacheSet) return passMultiIsoCuts(0.14, 0.73, 7.3, muID_cache.getMiniiso(muIdx), muID_cache.getPtratio(muIdx), muID_cache.getPtrel(muIdx) );
      else return passMultiIso(13, muIdx, 0.14, 0.73, 7.3, 1, 1);
      break;

   ///////////////////
   /// SS tight v5 ///
   ///////////////////
  
    case(SS_tight_noiso_v5):
      if (muonID(muIdx, SS_fo_noiso_v5)==0) return false;//make sure it's tighter than FO
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      return isMediumMuonPOG(muIdx);
      break;

   case(SS_tight_v5):
      if (muonID(muIdx, SS_tight_noiso_v5)==0) return false;
      if (muIDCacheSet) return passMultiIsoCuts(gconf.multiiso_mu_minireliso, gconf.multiiso_mu_ptratio, gconf.multiiso_mu_ptrel, muID_cache.getMiniiso(muIdx), muID_cache.getPtratio(muIdx), muID_cache.getPtrel(muIdx) );
      else return passMultiIso(13, muIdx, gconf.multiiso_mu_minireliso, gconf.multiiso_mu_ptratio, gconf.multiiso_mu_ptrel, gconf.ea_version, 2);
      break;

   ///////////////////
   /// SS tight v6 ///
   ///////////////////
  
    case(SS_tight_noiso_v6):
      if (muonID(muIdx, SS_fo_noiso_v6)==0) return false;//make sure it's tighter than FO
      if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
      if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
      if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdMedium, muIdx)   ) return false;
      } else {
        if (!isMediumMuonPOG(muIdx)                         ) return false;
      }
      break;

   case(SS_tight_v6):
      if (muonID(muIdx, SS_tight_noiso_v6)==0) return false;
      if (muIDCacheSet) return passMultiIsoCuts(gconf.multiiso_mu_minireliso, gconf.multiiso_mu_ptratio, gconf.multiiso_mu_ptrel, muID_cache.getMiniiso(muIdx), muID_cache.getPtratio(muIdx), muID_cache.getPtrel(muIdx) );
      else return passMultiIso(13, muIdx, gconf.multiiso_mu_minireliso, gconf.multiiso_mu_ptratio, gconf.multiiso_mu_ptrel, gconf.ea_version, 2);
      break;

   ////////////////////
   /// WW medium v1 ///
   ////////////////////
  
    case(WW_medium_noiso_v1):
      if (muonID(muIdx, WW_fo_noiso_v1)==0) return false;//make sure it's tighter than FO
      return isMediumMuonPOG(muIdx);
      break;

   case(WW_medium_v1):
      if (muonID(muIdx, WW_medium_noiso_v1)==0) return false;
      if (muIDCacheSet) return passMultiIsoCuts(0.14, 0.68, 7.0, muID_cache.getMiniiso(muIdx), muID_cache.getPtratio(muIdx), muID_cache.getPtrel(muIdx) );
      else return passMultiIso(13, muIdx, 0.14, 0.68, 7.0, 0, 0);
      break;

   ////////////////////
   /// WW medium v2 ///
   ////////////////////
  
    case(WW_medium_noiso_v2):
      if (muonID(muIdx, WW_fo_noiso_v2)==0) return false;//make sure it's tighter than FO
      return true;
      break;

   case(WW_medium_v2):
      if (muonID(muIdx, WW_fo_v2)==0) return false;
      if (muRelIso04DB(muIdx) > 0.15) return false;
      return true;
      break;

   ////////////////////
   /// WW medium v3 ///
   ////////////////////
  
    case(WW_medium_noiso_v3):
      if (muonID(muIdx, WW_fo_noiso_v3)==0) return false;//make sure it's tighter than FO
      return true;
      break;

    case(WW_medium_v3):
      if (muonID(muIdx, WW_fo_v3)==0) return false;
      if (muRelIso04DB(muIdx) > 0.15) return false;
      return true;
      break;

   /////////////////////
   /// stop loose v1 ///
   /////////////////////
    case(STOP_loose_v1):
      if (!isLooseMuonPOG(muIdx)) return false;
      return true;
      break;

    case(STOP_loose_v2):
      if (!isLooseMuonPOG(muIdx)) return false;
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.1   ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.5   ) return false;
      //if (muMiniRelIso(muIdx, true, 0.5, true, false) > 0.2) return false;
      if (muMiniRelIsoCMS3_DB(muIdx)               > 0.2    ) return false;
      return true;
      break;

    case(STOP_loose_v3):
      if (!isLooseMuonPOG(muIdx)) return false;
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.1   ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.5   ) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,1)             >  0.2   ) return false;
      return true;
      break;

    case(STOP_loose_v4):
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdLoose, muIdx)    ) return false;
      } else {
        if (!isLooseMuonPOG(muIdx)                          ) return false;
      }
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.1   ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.5   ) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.2 ) return false;
      return true;
      break;

   /////////////////////
   /// STOP medium ///
   /////////////////////
    case(STOP_medium_v1):
      if (!isMediumMuonPOG(muIdx)) return false;
      //if (muMiniRelIso(muIdx, true, 0.5, true, false) > 0.1) return false;
      if (muMiniRelIsoCMS3_DB(muIdx)                   > 0.1) return false;
       return true;
       break;

    case(STOP_medium_v2):
      if (!isLooseMuonPOG(muIdx) ) return false;
      if (!isMediumMuonPOG(muIdx)) return false;
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.02  ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.1   ) return false;
      //if (muMiniRelIso(muIdx, true, 0.5, true, false) > 0.1) return false;
      if (muMiniRelIsoCMS3_DB(muIdx)                   > 0.1) return false;
       return true;
       break;

    case(STOP_medium_v3):
      if (!isLooseMuonPOG(muIdx) ) return false;
      if (!isMediumMuonPOG(muIdx)) return false;
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.02  ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.1   ) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,1)                   > 0.1) return false;
       return true;
       break;

    case(STOP_medium_v4):
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdMedium, muIdx)   ) return false;
      } else {
        if (!isMediumMuonPOG(muIdx)                         ) return false;
      }
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.02  ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.1   ) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.1 ) return false;
      return true;
      break;

   /////////////////////
   /// STOP tight///
   /////////////////////
    case(STOP_tight_v1):
      if (!isTightMuonPOG(muIdx)) return false;
      if (muRelIso03DB(muIdx) >= 0.15) return false;
       return true;
       break;

    case(STOP_tight_v2):
      if (!isTightMuonPOG(muIdx)) return false;
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.02  ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.1   ) return false;
      //if (muMiniRelIso(muIdx, true, 0.5, true, false) > 0.1) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,1)                   > 0.1) return false;
       return true;
       break;

    case(STOP_tight_v3):

    case(STOP_tight_v4):
      if (gconf.cmssw_ver >= 94) {
        if (!passesMuonPOG(muID::CutBasedIdTight, muIdx)    ) return false;
      } else {
        if (!isTightMuonPOG(muIdx)                          ) return false;
      }
      if (fabs(mus_dxyPV()             .at(muIdx)) >  0.02  ) return false;
      if (fabs(mus_dzPV()              .at(muIdx)) >  0.1   ) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.1 ) return false;
      return true;
      break;

   /////////////////////
   ///  STOP sync    ///
   /////////////////////
    case(STOP_sync_v1):
      if (!isTightMuonPOG(muIdx)) return false;
      if (muRelIso03DB(muIdx) >= 0.15) return false;
       break;

   ////////////////////
   /// HAD tight v1 ///
   ////////////////////
  
    case(HAD_tight_noiso_v1):
    case(HAD_tight_noiso_v2):
    case(HAD_tight_noiso_v3):
    case(HAD_tight_noiso_v4):
      if (!isTightMuonPOG(muIdx)) return false;
      return true;
      break;

    case(HAD_tight_v1):
      if (muonID(muIdx, HAD_tight_noiso_v1)==0) return false;
      if (muRelIso03(muIdx, analysis) >= 0.15) return false; 
      return true;
      break;

   ////////////////////
   /// HAD tight v2 ///
   ////////////////////
  
    // // now using mini iso
    // case(HAD_tight_v2):
    //   if (muonID(muIdx, HAD_tight_noiso_v2)==0) return false;
    //   if (muMiniRelIso(muIdx) > 0.2) return false;
    //   return true;
    //   break;

   ////////////////////
   /// HAD tight v3 ///
   ////////////////////
  
    // same ID as v2, but use CMS3 miniIso with EA corrections
    case(HAD_tight_v3):
      if (muonID(muIdx, HAD_tight_noiso_v3)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx) > 0.2) return false;
      return true;
      break;

   ////////////////////
   /// HAD tight v4 ///
   ////////////////////
  
    // same ID as v2 and v3, use updated EA values
    case(HAD_tight_v4):
      if (muonID(muIdx, HAD_tight_noiso_v4)==0) return false;
      if (muMiniRelIsoCMS3_EA(muIdx,gconf.ea_version) > 0.2) return false;
      return true;
      break;

   /////////////////////
   /// ZMET loose v1 ///
   /////////////////////
  
    case(ZMET_loose_v1):
      if (!isLooseMuonPOG(muIdx)) return false;
      if (muRelIso03(muIdx, analysis) >= 0.15) return false; 
	  return true;
      break;

    case(ZMET_loose_noiso_v1):
      if (!isLooseMuonPOG(muIdx)) return false;
      // if (muRelIso03(muIdx, analysis) >= 0.15) return false; 
	  return true;
      break;

   //////////////////////
   /// ZMET medium v1 ///
   //////////////////////
   //

     case(ZMET_mediumMu_v4):
        if(gconf.cmssw_ver >= 94 || gconf.year >= 2017)
        {
            if(!passesMuonPOG(muID::CutBasedIdMedium,muIdx)) return false;
        }
        else
        {
            if (!isMediumMuonPOG(muIdx)               ) return false;
        }
	    if (muMiniRelIsoCMS3_EA( muIdx, 4) > 0.2  ) return false;
	    else return true;

        break;

     case(ZMET_mediumMu_veto_v4):
        if(gconf.cmssw_ver >= 94 || gconf.year >= 2017)
        {
          if(!passesMuonPOG(muID::CutBasedIdMedium,muIdx)) return false;
        }
        else
        {
            if (!isMediumMuonPOG(muIdx)               ) return false;
        }
	    if (muMiniRelIsoCMS3_EA( muIdx, 4) > 0.4  ) return false;
	    else return true;

        break;
  
    case(ZMET_mediumMu_v3):
      if(gconf.cmssw_ver >= 94 || gconf.year >= 2017)
      {
        if(!passesMuonPOG(muID::CutBasedIdMedium,muIdx)) return false;
      }
      else
      {
        if (!isMediumMuonPOG_forICHEP(muIdx)      ) return false;
      }
	  if (muMiniRelIsoCMS3_EA( muIdx, 1) > 0.2  ) return false;
	  else return true;
      break;
  
    case(ZMET_mediumMu_veto_v3):
      if(gconf.cmssw_ver >= 94 || gconf.year >= 2017)
      {
        if(!passesMuonPOG(muID::CutBasedIdMedium,muIdx)) return false;
      }
      else
      {
        if (!isMediumMuonPOG_forICHEP(muIdx)      ) return false;
      }
	  if (muMiniRelIsoCMS3_EA( muIdx, 1) > 0.4  ) return false;
	  else return true;
      break;
  
    case(ZMET_mediumMu_v2):
      if(gconf.cmssw_ver >= 94 || gconf.year >= 2017)
      {
        if(!passesMuonPOG(muID::CutBasedIdMedium,muIdx)) return false;
      }
      else
      {
        if (!isMediumMuonPOG(muIdx)               ) return false;
      }
	  if (muMiniRelIsoCMS3_EA( muIdx, 1) > 0.2  ) return false;
	  else return true;
      break;
  
    case(ZMET_mediumMu_veto_v2):
      if(gconf.cmssw_ver >= 94 || gconf.year >= 2017)
      {
        if(!passesMuonPOG(muID::CutBasedIdMedium,muIdx)) return false;
      }
      else
      {
        if (!isMediumMuonPOG(muIdx)               ) return false;
      }
	  if (muMiniRelIsoCMS3_EA( muIdx, 1) > 0.4  ) return false;
	  else return true;
      break;
  
    // case(ZMET_mediumMu_v1):
    //   if (!isMediumMuonPOG(muIdx)                             ) return false;
    //       if (fabs(mus_dxyPV()             .at(muIdx)) >  0.05    ) return false;
    //       if (fabs(mus_dzPV()              .at(muIdx)) >  0.1     ) return false;
    //       if( muMiniRelIso( muIdx, true, 0.5, false, true ) > 0.1 ) return false;
    //       else return true;
    //   break;

    case(ZMET_mediumMu_noiso_v1):
      if (!isMediumMuonPOG(muIdx)                             ) return false;
	  if (fabs(mus_dxyPV()             .at(muIdx)) >  0.05    ) return false;
	  if (fabs(mus_dzPV()              .at(muIdx)) >  0.1     ) return false;
	  else return true;
      break;

   /////////////////////
   /// ZMET tight v1 ///
   /////////////////////
  
    // case(ZMET_tight_v2):
    //   if (!isTightMuonPOG(muIdx)                              ) return false;
    //       if( muMiniRelIso( muIdx, true, 0.5, false, true ) > 0.1 ) return false;
    //       return true;
    //   break;

    case(ZMET_tight_noiso_v2):
      if (!isTightMuonPOG(muIdx)                              ) return false;
	  return true;
      break;
    
    case(ZMET_tight_v1):
      if (!isTightMuonPOG(muIdx)) return false;
      if (muRelIso03(muIdx, analysis) >= 0.15) return false; 
	  return true;
      break;

    case(ZMET_tight_noiso_v1):
      if (!isTightMuonPOG(muIdx)) return false;
      // if (muRelIso03(muIdx, analysis) >= 0.15) return false; 
	  return true;
      break;
   
  /////////////////////
  /// VVV Selection ///
  /////////////////////
    
  //-------------
  // Veto Leptons

  case(VVV_cutbased_veto):
    if (muonID(muIdx, VVV_cutbased_veto_noiso)==0) return false;
    if (muRelIso03EA(muIdx,1) > 0.40) return false;
    return true;
    break;


  case(VVV_cutbased_veto_noiso):
    if (muonID(muIdx, VVV_cutbased_veto_noiso_noip)==0) return false;
    if (fabs(mus_dxyPV().at(muIdx)) > 0.05) return false;
    if (fabs(mus_dzPV().at(muIdx)) > 0.1) return false;
    return true;
    break;

  case(VVV_cutbased_veto_noiso_noip):
    if (fabs(mus_p4().at(muIdx).eta()) > 2.4) return false;
    return isLooseMuonPOG(muIdx);
    break;


  //---------------
  //Fakable Objects

  case(VVV_cutbased_fo):
    if (!muonID(muIdx, VVV_cutbased_fo_noiso)) return false;
    if (muRelIso03EA(muIdx,1) > 0.40) return false;
    return true;
    break;

  case(VVV_cutbased_fo_noiso):
    if (!muonID(muIdx, VVV_cutbased_veto_noiso)) return false;
    if (fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx) >= 4) return false;
    if (mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt() >= 0.2) return false;
    return isMediumMuonPOG(muIdx);
    break;

  //---------------
  //Tight Selection

  case(VVV_baseline): //Should stay updated to currently used selection
    return muonID(muIdx, VVV_cutbased_tight);
    break;

  case(VVV_cutbased_tight):
    if (!muonID(muIdx, VVV_cutbased_tight_noiso)) return false;
    if (muRelIso03EA(muIdx,1) > 0.06) return false;
    return true;
    break;

  case(VVV_cutbased_tight_noiso):
    if (!muonID(muIdx, VVV_cutbased_fo_noiso)) return false;
    if (fabs(mus_ip3d().at(muIdx)) >= 0.015) return false;
    return true;
    break;

  //-------------
  // Veto Leptons

  case(VVV_cutbased_veto_noiso_v2):
    if (!( fabs(cms3.mus_p4().at(muIdx).eta()) <  2.4  )) return false;
    if (!( fabs(cms3.mus_dxyPV().at(muIdx))    <  0.05 )) return false;
    if (!( fabs(cms3.mus_dzPV().at(muIdx))     <  0.1  )) return false;
    if (!( isLooseMuonPOG(muIdx)                       )) return false;
    return true;
    break;

  case(VVV_cutbased_veto_v2):
    if (!( muPtRatio(muIdx) > 0.58                   )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v2) )) return false;
    return true;
    break;


  //---------------
  //Fakable Objects

  case(VVV_cutbased_fo_noiso_v2):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v2)                    )) return false;
    return true;
    break;

  case(VVV_cutbased_fo_v2):
    if (!( muPtRatio(muIdx) > 0.65                 )) return false;
    if (!( muonID(muIdx, VVV_cutbased_fo_noiso_v2) )) return false;
    return true;
    break;

  //---------------
  //Tight Selection

  case(VVV_cutbased_tight_noiso_v2):
    if (!( muonID(muIdx, VVV_cutbased_fo_noiso_v2) )) return false;
    return true;
    break;

  case(VVV_cutbased_tight_v2):
    if (!( muPtRatio(muIdx) > 0.9                  )) return false;
    if (!( muonID(muIdx, VVV_cutbased_tight_noiso_v2) )) return false;
    return true;
    break;

  //--------------------
  //Fakable Objects (3l)

  case(VVV_cutbased_3l_fo_noiso_v2):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v2)                    )) return false;
    return true;
    break;

  case(VVV_cutbased_3l_fo_v2):
    if (!( muPtRatio(muIdx) > 0.65                    )) return false;
    if (!( muonID(muIdx, VVV_cutbased_3l_fo_noiso_v2) )) return false;
    return true;
    break;

  //--------------------
  //Tight Selection (3l)

  case(VVV_cutbased_3l_tight_noiso_v2):
    if (!( muonID(muIdx, VVV_cutbased_3l_fo_noiso_v2) )) return false;
    return true;
    break;

  case(VVV_cutbased_3l_tight_v2):
    if (!( muPtRatio(muIdx) > 0.84                       )) return false;
    if (!( muonID(muIdx, VVV_cutbased_3l_tight_noiso_v2) )) return false;
    return true;
    break;

  //-------------
  // Veto Leptons

  case(VVV_cutbased_veto_noiso_v3):
    if (!( fabs(cms3.mus_p4().at(muIdx).eta()) <  2.4  )) return false;
    if (!( fabs(cms3.mus_dxyPV().at(muIdx))    <  0.05 )) return false;
    if (!( fabs(cms3.mus_dzPV().at(muIdx))     <  0.1  )) return false;
    if (!( isLooseMuonPOG(muIdx)                       )) return false;
    return true;
    break;

  case(VVV_cutbased_veto_v3):
    if (!( muPtRatio(muIdx) > 0.58                   )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v3) )) return false;
    return true;
    break;


  //---------------
  //Fakable Objects

  case(VVV_cutbased_fo_noiso_v3):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v3)                    )) return false;
    return true;
    break;

  case(VVV_cutbased_fo_v3):
    if (!( muPtRatio(muIdx) > 0.65                 )) return false;
    if (!( muonID(muIdx, VVV_cutbased_fo_noiso_v3) )) return false;
    return true;
    break;

  //---------------
  //Tight Selection

  case(VVV_cutbased_tight_noiso_v3):
    if (!( muonID(muIdx, VVV_cutbased_fo_noiso_v3) )) return false;
    return true;
    break;

  case(VVV_cutbased_tight_v3):
    if (fabs(cms3.mus_p4()[muIdx].eta()) <= 1.6)
    {
        if (!( muPtRatio(muIdx) > 0.9                 )) return false;
    }
    else
    {
        if (!( muPtRatio(muIdx) > 0.88                )) return false;
    }
    if (!( muonID(muIdx, VVV_cutbased_tight_noiso_v3) )) return false;
    return true;
    break;

  //--------------------
  //Fakable Objects (3l)

  case(VVV_cutbased_3l_fo_noiso_v3):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v3)                    )) return false;
    return true;
    break;

  case(VVV_cutbased_3l_fo_v3):
    if (!( muPtRatio(muIdx) > 0.65                    )) return false;
    if (!( muonID(muIdx, VVV_cutbased_3l_fo_noiso_v3) )) return false;
    return true;
    break;

  //--------------------
  //Tight Selection (3l)

  case(VVV_cutbased_3l_tight_noiso_v3):
    if (!( muonID(muIdx, VVV_cutbased_3l_fo_noiso_v3) )) return false;
    return true;
    break;

  case(VVV_cutbased_3l_tight_v3):
    if (!( muPtRatio(muIdx) > 0.84                 )) return false;
    if (!( muonID(muIdx, VVV_cutbased_3l_tight_noiso_v3) )) return false;
    return true;
    break;

  //-------------
  // Veto Leptons

  case(VVV_cutbased_veto_noiso_v4):
    if (!( fabs(cms3.mus_p4().at(muIdx).eta()) <  2.4  )) return false;
    if (!( fabs(cms3.mus_dxyPV().at(muIdx))    <  0.05 )) return false;
    if (!( fabs(cms3.mus_dzPV().at(muIdx))     <  0.1  )) return false;
    if (!( isLooseMuonPOG(muIdx)                       )) return false;
    return true;
    break;

  case(VVV_cutbased_veto_v4):
    if (!( muRelIso03EA(muIdx, 2, true) < 0.4        )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v4) )) return false;
    return true;
    break;


  //---------------
  //Fakable Objects

  case(VVV_cutbased_fo_noiso_v4):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v4)                    )) return false;
    return true;
    break;

  case(VVV_cutbased_fo_v4):
    if (!( muRelIso03EA(muIdx, 2, true) < 0.4      )) return false;
    if (!( muonID(muIdx, VVV_cutbased_fo_noiso_v4) )) return false;
    return true;
    break;

  //---------------
  //Tight Selection

  case(VVV_cutbased_tight_noiso_v4):
    if (!( muonID(muIdx, VVV_cutbased_fo_noiso_v4) )) return false;
    return true;
    break;

  case(VVV_cutbased_tight_v4):
    if (!( muRelIso03EA(muIdx, 2, true) < 0.03     )) return false;
    if (!( muonID(muIdx, VVV_cutbased_tight_noiso_v4) )) return false;
    return true;
    break;

  //--------------------
  //Fakable Objects (3l)

  case(VVV_cutbased_3l_fo_noiso_v4):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_cutbased_veto_noiso_v4)                    )) return false;
    return true;
    break;

  case(VVV_cutbased_3l_fo_v4):
    if (!( muRelIso03EA(muIdx, 2, true) < 0.4         )) return false;
    if (!( muonID(muIdx, VVV_cutbased_3l_fo_noiso_v4) )) return false;
    return true;
    break;

  //--------------------
  //Tight Selection (3l)

  case(VVV_cutbased_3l_tight_noiso_v4):
    if (!( muonID(muIdx, VVV_cutbased_3l_fo_noiso_v4) )) return false;
    return true;
    break;

  case(VVV_cutbased_3l_tight_v4):
    if (!( muRelIso03EA(muIdx, 2, true) < 0.07     )) return false;
    if (!( muonID(muIdx, VVV_cutbased_3l_tight_noiso_v4) )) return false;
    return true;
    break;

  //
  //
  // gconf version
  //
  //

  //-------------
  // Veto Leptons

  case(VVV_veto_noiso_v5):
    if (!( fabs(cms3.mus_p4().at(muIdx).eta()) <  2.4  )) return false;
    if (!( fabs(cms3.mus_dxyPV().at(muIdx))    <  0.05 )) return false;
    if (!( fabs(cms3.mus_dzPV().at(muIdx))     <  0.1  )) return false;
    if (!( isLooseMuonPOG(muIdx)                       )) return false;
    return true;
    break;

  case(VVV_veto_v5):
    if (!( muRelIso03EA(muIdx, gconf.ea_version, gconf.mu_addlep_veto) < gconf.mu_reliso_veto )) return false;
    if (!( muonID(muIdx, VVV_veto_noiso_v5) )) return false;
    return true;
    break;


  //---------------
  //Fakable Objects

  case(VVV_fo_noiso_v5):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_veto_noiso_v5)                             )) return false;
    return true;
    break;

  case(VVV_fo_v5):
    if (!( muRelIso03EA(muIdx, gconf.ea_version, gconf.mu_addlep_fo) < gconf.mu_reliso_fo )) return false;
    if (!( muonID(muIdx, VVV_fo_noiso_v5) )) return false;
    return true;
    break;

  //---------------
  //Tight Selection

  case(VVV_tight_noiso_v5):
    if (!( muonID(muIdx, VVV_fo_noiso_v5) )) return false;
    return true;
    break;

  case(VVV_tight_v5):
    if (!( muRelIso03EA(muIdx, gconf.ea_version, gconf.mu_addlep_tight) < gconf.mu_reliso_tight )) return false;
    if (!( muonID(muIdx, VVV_tight_noiso_v5) )) return false;
    return true;
    break;


  //---------------
  //Fakable Objects

  case(VVV_3l_fo_noiso_v5):
    if (!( fabs(cms3.mus_ip3d()[muIdx])                         < 0.015 )) return false;
    if (!( fabs(mus_ip3d().at(muIdx))/mus_ip3derr().at(muIdx)   < 4     )) return false;
    if (!( mus_ptErr().at(muIdx)/mus_trk_p4().at(muIdx).pt()    < 0.2   )) return false;
    if (!( isMediumMuonPOG(muIdx)                                       )) return false;
    if (!( muonID(muIdx, VVV_veto_noiso_v5)                             )) return false;
    return true;
    break;

  case(VVV_3l_fo_v5):
    if (!( muRelIso03EA(muIdx, gconf.ea_version, gconf.mu_addlep_3l_fo) < gconf.mu_reliso_3l_fo )) return false;
    if (!( muonID(muIdx, VVV_3l_fo_noiso_v5) )) return false;
    return true;
    break;

  //---------------
  //Tight Selection

  case(VVV_3l_tight_noiso_v5):
    if (!( muonID(muIdx, VVV_3l_fo_noiso_v5) )) return false;
    return true;
    break;

  case(VVV_3l_tight_v5):
    if (!( muRelIso03EA(muIdx, gconf.ea_version, gconf.mu_addlep_3l_tight) < gconf.mu_reliso_3l_tight )) return false;
    if (!( muonID(muIdx, VVV_3l_tight_noiso_v5) )) return false;
    return true;
    break;



   ///////////////
   /// Default ///
   ///////////////
    default:
      {
        cout << "Warning! Muon ID not defined for this id_level! " << id_level << endl;
        return false;
      }
   
  
  }//cases
  return true;
}

int muTightID(unsigned int muIdx, analysis_t analysis, int version){
  switch (analysis){
    case (POG):
      if (!isTightMuonPOG(muIdx)) return 1;
      if (!isLooseMuonPOG(muIdx)) return 0;
      break;
    case (WW):
      if (muonID(muIdx, WW_medium_v2)) return 2;
      if (muonID(muIdx, WW_fo_v2))     return 1;
      if (muonID(muIdx, WW_veto_v2))   return 0;
      break;
    case (SS):
      if (muonID(muIdx, SS_tight_v4)) return 2;  
      if (muonID(muIdx, SS_fo_v3))    return 1;
      if (muonID(muIdx, SS_veto_v3))  return 0;
      break;
    case (HAD):
      if (version == 1){
        if (muonID(muIdx, HAD_tight_v1)) return 1;
        if (muonID(muIdx, HAD_loose_v1)) return 0;
      }
      if (version == 2){
        if (muonID(muIdx, HAD_tight_v2)) return 1;
        if (muonID(muIdx, HAD_loose_v2)) return 0;
      }
      if (version == 3){
        if (muonID(muIdx, HAD_tight_v3)) return 1;
        if (muonID(muIdx, HAD_loose_v3)) return 0;
      }
      if (version == 4){
        if (muonID(muIdx, HAD_tight_v4)) return 1;
        if (muonID(muIdx, HAD_loose_v4)) return 0;
      }
      break;
    case (STOP):
      if (muonID(muIdx, STOP_tight_v1)) return 2;
      if (muonID(muIdx, STOP_medium_v2)) return 1;
      if (muonID(muIdx, STOP_loose_v1)) return 0;
      break;
    case (ZMET):
      if (muonID(muIdx, ZMET_tight_v1)) return 1;
      if (muonID(muIdx, ZMET_loose_v1)) return 0;
      break;
    case (VVV):
      if (muonID(muIdx, VVV_baseline)) return 1;
  }
  return -1;
}

int tightChargeMuon(unsigned int muIdx){
  if ( mus_ptErr().at(muIdx) / mus_p4().at(muIdx).pt() < 0.2 )          return 2;
  else                                                                  return 0;
}

bool PassSoftMuonCut(unsigned int muIdx) {
    
  if (mus_p4().at(muIdx).pt() <= 3) return false;
  if (((mus_type().at(muIdx)) & (1<<2)) == 0) return false;
  if (!mus_pid_TMLastStationTight().at(muIdx)) return false;
  if (mus_nlayers().at(muIdx) <= 5) return false;
  if (fabs(mus_dxyPV().at(muIdx)) > 0.2) return false;
  if (fabs(mus_dzPV().at(muIdx)) > 0.5) return false;
  if ((mus_iso03_emEt().at(muIdx)+mus_iso03_hadEt().at(muIdx)+mus_iso03_sumPt().at(muIdx)) < 0.10*mus_p4().at(muIdx).pt() && mus_p4().at(muIdx).pt() > 20.) return false;
  return true;

}

// check CMS3 version to see which c++ type is stored in the ntuples for mus_gfit_ndof
int get_mus_gfit_ndof( unsigned int muIdx ) {
  int gfit_ndof = 0;
  TString version = evt_CMS3tag().at(0);
  // convert last two digits of version number to int
  int small_version = TString(version(version.Length()-2,version.Length())).Atoi();
  if (version.Contains("CMS3_V08-00") && small_version <= 12) gfit_ndof = mus_gfit_ndof_float().at(muIdx);
  else gfit_ndof = mus_gfit_ndof().at(muIdx);
  return gfit_ndof;
}
