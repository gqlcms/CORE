#ifndef CONFIG_H
#define CONFIG_H
#include <string>

#include "Tools/JetCorrector.h"

/*
   Need to check the year in a CORE function (or your own)? `gconf` is a global instance.
       if (gconf.year == 2017) { ... }
   Want to update the year in your babymaker?
       gconf.year = 2018;
   Need another variable? Add a line with a dummy default. If an analysis uses
   a variable from this config, it's up to them to make sure the variables are set properly
   at runtime. Leaving dummy values here forces you to do that.
*/

class GlobalConfig {
    public:
        unsigned int year = 0;
        unsigned int cmssw_ver = 0; // 74, 80, 94, 101, ...
        std::string analysis = ""; 
        float btag_disc_wp = -1;
        int ea_version = -1;

        // MultiIso WPs for SS
        float multiiso_el_minireliso = -1;
        float multiiso_el_ptratio = -1;
        float multiiso_el_ptrel = -1;
        float multiiso_mu_minireliso = -1;
        float multiiso_mu_ptratio = -1;
        float multiiso_mu_ptrel = -1;

        //-------------------
        //
        // WWW (VVV) Analysis
        //
        //-------------------

        // Naming convention
        // <lep>_<var>_<idlevel>

        //_________________________________
        // Isolation configuration
        //

        // Same-sign muons
        float mu_reliso_veto      = -1;
        float mu_reliso_fo        = -1;
        float mu_reliso_tight     = -1;
        bool  mu_addlep_veto      = false;
        bool  mu_addlep_fo        = false;
        bool  mu_addlep_tight     = false;
        // Same-sign electrons
        float el_reliso_veto      = -1;
        float el_reliso_fo        = -1;
        float el_reliso_tight     = -1;
        bool  el_addlep_veto      = false;
        bool  el_addlep_fo        = false;
        bool  el_addlep_tight     = false;
        // Three-lepton muons (Shares same veto as same-sign)
        float mu_reliso_3l_fo     = -1;
        float mu_reliso_3l_tight  = -1;
        bool  mu_addlep_3l_fo     = false;
        bool  mu_addlep_3l_tight  = false;
        // Three-lepton electrons (Shares same veto as same-sign)
        float el_reliso_3l_fo     = -1;
        float el_reliso_3l_tight  = -1;
        bool  el_addlep_3l_fo     = false;
        bool  el_addlep_3l_tight  = false;

        //_________________________________
        // Electron MVA ID
        //
        //  ib = inner barrel (<= 0.8)
        //  ob = outer barrel (<= 1.479)
        //  ec = endcap       (>  1.479)
        //

        // Same-sign electrons
        float el_mva_ib_veto = -1;
        float el_mva_ob_veto = -1;
        float el_mva_ec_veto = -1;
        float el_mva_ib      = -1;
        float el_mva_ob      = -1;
        float el_mva_ec      = -1;
        // Three-lepton electrons (Shares same veto as same-sign)
        float el_mva_ib_3l   = -1;
        float el_mva_ob_3l   = -1;
        float el_mva_ec_3l   = -1;

        // JECs
        FactorizedJetCorrector * jet_corrector_L1 = 0;
        FactorizedJetCorrector * jet_corrector_L2L3 = 0;
        FactorizedJetCorrector * jet_corrector_L1L2L3 = 0;

        // ...
};

#ifndef __CINT__
extern GlobalConfig gconf;
#endif

#endif
