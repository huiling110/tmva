// @(#)root/tmva $Id$
/**********************************************************************************
 * Project   : TMVA - a ROOT-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Root Macro: TMVAClassification                                                 *
 *                                                                                *
 * This macro provides examples for the training and testing of the               *
 * TMVA classifiers.                                                              *
 *                                                                                *
 * As input data is used a toy-MC sample consisting of four Gaussian-distributed  *
 * and linearly correlated input variables.                                       *
 *                                                                                *
 * The methods to be used can be switched on and off by means of booleans, or     *
 * via the prompt command, for example:                                           *
 *                                                                                *
 *    root -l ./TMVAClassification.C\(\"Fisher,Likelihood\"\)                     *
 *                                                                                *
 * (note that the backslashes are mandatory)                                      *
 * If no method given, a default set of classifiers is used.                      *
 *                                                                                *
 * The output file "TMVA.root" can be analysed with the use of dedicated          *
 * macros (simply say: root -l <macro.C>), which can be conveniently              *
 * invoked through a GUI that will appear at the end of the run of this macro.    *
 * Launch the GUI via the command:                                                *
 *                                                                                *
 *    root -l ./TMVAGui.C                                                         *
 *                                                                                *
 **********************************************************************************/

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"


#if not defined(__CINT__) || defined(__MAKECINT__)
// needs to be included when makecint runs (ACLIC)
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#endif

void TMVAClassification( TString myMethodList = "" )
{
   // The explicit loading of the shared libTMVA is done in TMVAlogon.C, defined in .rootrc
   // if you use your private .rootrc, or run from a different directory, please copy the
   // corresponding lines from .rootrc

   // methods to be processed can be given as an argument; use format:
   //
   // mylinux~> root -l TMVAClassification.C\(\"myMethod1,myMethod2,myMethod3\"\)
   //
   // if you like to use a method via the plugin mechanism, we recommend using
   //
   // mylinux~> root -l TMVAClassification.C\(\"P_myMethod\"\)
   // (an example is given for using the BDT as plugin (see below),
   // but of course the real application is when you write your own
   // method based)

   //---------------------------------------------------------------
   // This loads the library
   TMVA::Tools::Instance();

   // to get access to the GUI and all tmva macros
   TString thisdir = gSystem->DirName(gInterpreter->GetCurrentMacroName());
   gROOT->SetMacroPath(thisdir + ":" + gROOT->GetMacroPath());
   gROOT->ProcessLine(".L TMVAGui.C");

   // Default MVA methods to be trained + tested
   std::map<std::string,int> Use;

   // --- Cut optimisation
   Use["Cuts"]            = 0;//1
   Use["CutsD"]           = 0;//1
   Use["CutsPCA"]         = 0;
   Use["CutsGA"]          = 0;
   Use["CutsSA"]          = 1;//0
   // 
   // --- 1-dimensional likelihood ("naive Bayes estimator")
   Use["Likelihood"]      = 0;//1
   Use["LikelihoodD"]     = 0; // the "D" extension indicates decorrelated input variables (see option strings)
   Use["LikelihoodPCA"]   = 0;//1 // the "PCA" extension indicates PCA-transformed input variables (see option strings)
   Use["LikelihoodKDE"]   = 0;
   Use["LikelihoodMIX"]   = 0;
   //
   // --- Mutidimensional likelihood and Nearest-Neighbour methods
   Use["PDERS"]           = 0;//1
   Use["PDERSD"]          = 0;
   Use["PDERSPCA"]        = 0;
   Use["PDEFoam"]         = 0;//1
   Use["PDEFoamBoost"]    = 0; // uses generalised MVA method boosting
   Use["KNN"]             = 0;//1 // k-nearest neighbour method
   //
   // --- Linear Discriminant Analysis
   Use["LD"]              = 0;//1 // Linear Discriminant identical to Fisher
   Use["Fisher"]          = 0;
   Use["FisherG"]         = 0;
   Use["BoostedFisher"]   = 0; // uses generalised MVA method boosting
   Use["HMatrix"]         = 0;
   //
   // --- Function Discriminant analysis
   Use["FDA_GA"]          = 0;//1 // minimisation of user-defined function using Genetics Algorithm
   Use["FDA_SA"]          = 0;
   Use["FDA_MC"]          = 0;
   Use["FDA_MT"]          = 0;
   Use["FDA_GAMT"]        = 0;
   Use["FDA_MCMT"]        = 0;
   //
   // --- Neural Networks (all are feed-forward Multilayer Perceptrons)
   Use["MLP"]             = 0; // Recommended ANN
   Use["MLPBFGS"]         = 0; // Recommended ANN with optional training method
   Use["MLPBNN"]          = 0;//1 // Recommended ANN with BFGS training method and bayesian regulator
   Use["CFMlpANN"]        = 0; // Depreciated ANN from ALEPH
   Use["TMlpANN"]         = 0; // ROOT's own ANN
   //
   // --- Support Vector Machine 
   Use["SVM"]             = 0;//1
   // 
   // --- Boosted Decision Trees
   Use["BDT"]             = 1; // uses Adaptive Boost
   Use["BDTG"]            = 1;//0 // uses Gradient Boost
   Use["BDTB"]            = 0; // uses Bagging
   Use["BDTD"]            = 0; // decorrelation + Adaptive Boost
   Use["BDTF"]            = 0; // allow usage of fisher discriminant for node splitting 
   // 
   // --- Friedman's RuleFit method, ie, an optimised series of cuts ("rules")
   Use["RuleFit"]         = 0;//1
   // ---------------------------------------------------------------

   std::cout << std::endl;
   std::cout << "==> Start TMVAClassification" << std::endl;

   // Select methods (don't look at this code - not of interest)
   if (myMethodList != "") {
      for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) it->second = 0;

      std::vector<TString> mlist = TMVA::gTools().SplitString( myMethodList, ',' );
      for (UInt_t i=0; i<mlist.size(); i++) {
         std::string regMethod(mlist[i]);

         if (Use.find(regMethod) == Use.end()) {
            std::cout << "Method \"" << regMethod << "\" not known in TMVA under this name. Choose among the following:" << std::endl;
            for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) std::cout << it->first << " ";
            std::cout << std::endl;
            return;
         }
         Use[regMethod] = 1;
      }
   }

   // --------------------------------------------------------------------------------------------------

   // --- Here the preparation phase begins

   // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
   // TString outfileName( "TMVA.root" );
   // TString outfileName( "TMVA_study.root" );
   // TString outfileName( "TMVA_1Tau0L.root" );
   TString outfileName( "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/TMVAOutput/TMVA_1Tau0L_v1.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Create the factory object. Later you can choose the methods
   // whose performance you'd like to investigate. The factory is 
   // the only TMVA object you have to interact with
   //
   // The first argument is the base of the name of all the
   // weightfiles in the directory weight/
   //
   // The second argument is the output file for the training results
   // All TMVA output can be suppressed by removing the "!" (not) in
   // front of the "Silent" argument in the option string
   TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

   // If you wish to modify default settings
   // (please check "src/Config.h" to see all available global options)
   //    (TMVA::gConfig().GetVariablePlotting()).fTimesRMS = 8.0;
      // (TMVA::gConfig().GetIONames()).fWeightFileDir = "myWeightDirectory";
      (TMVA::gConfig().GetIONames()).fWeightFileDir = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/TMVAOutput/weight1Tau0L_v1";

   // Define the input variables that shall be used for the MVA training
   // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
   // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]
   // factory->AddVariable( "myvar1 := var1+var2", 'F' );
   // factory->AddVariable( "myvar2 := var1-var2", "Expression 2", "", 'F' );
   // factory->AddVariable( "var3",                "Variable 3", "units", 'F' );
   // factory->AddVariable( "var4",                "Variable 4", "units", 'F' );
    factory->AddVariable( "jetsL_number",                "jetsL_number", "units", 'F' );
    factory->AddVariable( "jetsL_transMass",         "jetsL_transMass"   , "units", 'F');
    factory->AddVariable( "jetsL_HT",         "jetsL_HT"   , "units", 'F');
    factory->AddVariable( "jetsL_8pt",                "jetsL_8pt", "units", 'F' );
    factory->AddVariable( "jetsL_6pt",                "jetsL_6pt", "units", 'F' );
    factory->AddVariable( "jetsL_7pt",                "jetsL_7pt", "units", 'F' );
    factory->AddVariable( "jetsL_5pt",                "jetsL_5pt", "units", 'F' );
    factory->AddVariable( "bjetsL_HT",         "bjetsL_HT"   , "units", 'F');
    factory->AddVariable( "bjetsL_transMass",         "bjetsL_transMass"   , "units", 'F');
    factory->AddVariable( "jetsL_4pt",                "jetsL_4pt", "units", 'F' );
    factory->AddVariable( "jetsL_bScore",         "jetsL_bScore"   , "units", 'F');
    factory->AddVariable( "bjetsL_invariantMass",                "bjetsL_invariantMass", "units", 'F' );
    factory->AddVariable( "jetsL_9pt",                "jetsL_9pt", "units", 'F' );
    factory->AddVariable( "jetsL_3pt",                "jetsL_3pt", "units", 'F' );
    factory->AddVariable( "jetsL_4largestBscoreSum",                "jetsL_invariantMass", "units", 'F' );
    factory->AddVariable( "bjetsL_3pt",                "bjetsL_3pt", "units", 'F' );
    factory->AddVariable( "bjetsM_HT",                "bjetsM_HT", "units", 'F' );
    factory->AddVariable( "bjetsM_invariantMass",                "bjetsM_invariantMass", "units", 'F' );
    factory->AddVariable( "bjetsM_transMass",                "bjetsM_transMass", "units", 'F' );
    factory->AddVariable( "bjetsM_num",                "bjetsM_num", "units", 'F' );
    factory->AddVariable( "bjetsL_num",                "bjetsL_num", "units", 'F' );
    factory->AddVariable( "bjetsL_2pt",                "bjetsL_2pt", "units", 'F' );
    factory->AddVariable( "bjetsL_4pt",                "jetsL_2pt", "units", 'F' );
    factory->AddVariable( "toptagger_transMass",                "toptagger_transMass", "units", 'F' );
    factory->AddVariable( "toptagger_HT",                "toptagger_HT", "units", 'F' );
    factory->AddVariable( "jetsL_10pt",                "jetsL_10pt", "units", 'F' );
    factory->AddVariable( "bjetsL_1pt",                "bjetsL_1pt", "units", 'F' );
    factory->AddVariable( "jetsL_1pt",                "bjetsM_2pt", "units", 'F' );
    factory->AddVariable( "bjetsT_HT",                "bjetsT_transMass", "units", 'F' );
    // factory->AddVariable( "",                "", "units", 'F' );

   // You can add so-called "Spectator variables", which are not used in the MVA training,
   // but will appear in the final "TestTree" produced by TMVA. This TestTree will contain the
   // input variables, the response values of all trained MVAs, and the spectator variables
   // factory->AddSpectator( "spec1 := var1*2",  "Spectator 1", "units", 'F' );
   // factory->AddSpectator( "spec2 := var1*3",  "Spectator 2", "units", 'F' );

   // Read training and test data
   // (it is also possible to use ASCII format as input -> see TMVA Users Guide)
   // TString fname = "./tmva_class_example.root";
   TString fname_signal = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/TTTT_TuneCUETP8M2T4_13TeV-amcatnlo-pythia8.root";
   //6
   TString fname_bg_TTJets     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/TTJets_TuneCUETP8M2T4_13TeV-amcatnloFXFX-pythia8.root";/*{{{*/
   TString fname_bg_TTGJets     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/TTGJets_TuneCUETP8M1_13TeV-amcatnloFXFX-madspin-pythia8.root";
   TString fname_bg_ttZJets     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ttZJets_13TeV_madgraphMLM-pythia8.root";
   TString fname_bg_ttWJets     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ttWJets_13TeV_madgraphMLM.root";
   TString fname_bg_ttH     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ttH_4f_ctcvcp_TuneCP5_13TeV_madgraph_pythia8.root";
   // TString fname_bg_ttbb     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ttbb_4FS_ckm_amcatnlo_madspin_pythia8.root";
    //12
   TString fname_bg_WZ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WZ_TuneCUETP8M1_13TeV-pythia8.root";
   // TString fname_bg_WWTo2L2Nu     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WWTo2L2Nu_DoubleScattering_13TeV-pythia8.root";
   TString fname_bg_WpWpJJ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WpWpJJ_EWK-QCD_TuneCUETP8M1_13TeV-madgraph-pythia8.root";
   TString fname_bg_ZZ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ZZ_TuneCUETP8M1_13TeV-pythia8.root";
   TString fname_bg_WGJets     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WGJets_MonoPhoton_PtG-40to130_TuneCUETP8M1_13TeV-madgraph.root";
   TString fname_bg_ZGJetsToLLG     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ZGJetsToLLG_EW_LO_13TeV-sherpa.root";
    //20
   TString fname_bg_WWW     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WWW_4F_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_WWZ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WWZ_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_WWG     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WWG_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_ZZZ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ZZZ_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_WZZ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WZZ_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_WZG     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WZG_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_WGG     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WGG_5f_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_ZGGJets     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ZGGJets_ZToHadOrNu_5f_LO_madgraph_pythia8.root";
    //22
   TString fname_bg_WJetsToLNu     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/WJetsToLNu_TuneCUETP8M1_13TeV-madgraphMLM-pythia8.root";

   TString fname_bg_DYJetsToTauTau     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/DYJetsToTauTau_ForcedMuEleDecay_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8_ext1.root";
    //28
   TString fname_bg_tZq_ll     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8.root";
   TString fname_bg_ST_tW_antitop     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ST_tW_antitop_5f_inclusiveDecays_13TeV-powheg-pythia8_TuneCUETP8M2T4.root";
   TString fname_bg_ST_tW_top     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ST_tW_top_5f_inclusiveDecays_13TeV-powheg-pythia8_TuneCUETP8M2T4.root";
   TString fname_bg_TGJets     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/TGJets_TuneCUETP8M1_13TeV_amcatnlo_madspin_pythia8.root";
   TString fname_bg_THW     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/THW_ctcvcp_HIncl_M125_TuneCP5_13TeV-madgraph-pythia8.root";
   TString fname_bg_THQ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/THQ_ctcvcp_Hincl_13TeV-madgraph-pythia8_TuneCUETP8M1.root";
    //38
   TString fname_bg_VHToNonbb     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/VHToNonbb_M125_13TeV_amcatnloFXFX_madspin_pythia8.root";
   TString fname_bg_ZHToTauTau     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ZHToTauTau_M125_13TeV_powheg_pythia8.root";
   TString fname_bg_ZH_HToBB     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/ZH_HToBB_ZToLL_M125_13TeV_powheg_pythia8.root";
   TString fname_bg_GluGluHToZZTo4L     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/GluGluHToZZTo4L_M125_13TeV_powheg2_JHUgenV6_pythia8.root";
   TString fname_bg_GluGluHToBB     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/GluGluHToBB_M125_13TeV_amcatnloFXFX_pythia8.root";
   TString fname_bg_GluGluHToGG     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/GluGluHToGG_M125_13TeV_amcatnloFXFX_pythia8.root";
   // TString fname_bg_GluGluHToMuMu     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/GluGluHToMuMu_M-125_TuneCP5_PSweights_13TeV_powheg_pythia8.root";
   // TString fname_bg_GluGluHToTauTau     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/GluGluHToTauTau_M125_13TeV_powheg_pythia8.root";
   // TString fname_bg_GluGluHToWWTo2L2Nu     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/GluGluHToWWTo2L2Nu_M125_13TeV_powheg_JHUgen_pythia8.root";
   // TString fname_bg_GluGluHToWWToLNuQQ     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/GluGluHToWWToLNuQQ_M125_13TeV_powheg_JHUGenV628_pythia8.root";
    //41
   // TString fname_bg_VBFHToWWTo2L2Nu     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/VBFHToWWTo2L2Nu_M125_13TeV_powheg_JHUgenv628_pythia8.root";
   // TString fname_bg_VBFHToMuMu     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/VBFHToMuMu_M-125_TuneCP5_PSweights_13TeV_powheg_pythia8.root";
   // TString fname_bg_VBFHToGG     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/VBFHToGG_M125_13TeV_amcatnlo_pythia8_v2.root";[>}}}<]
   // TString fname_bg_     = "/publicfs/cms/user/huahuil/TauOfTTTT/2016v1/forMVA/1tau0lTausT/NoJEC/";
   
   // if (gSystem->AccessPathName( fname ))  // file does not exist in local directory
      // gSystem->Exec("curl -O http://root.cern.ch/files/tmva_class_example.root");
   
   // TFile *input = TFile::Open( fname );
   TFile *input_signal = TFile::Open( fname_signal );

   TFile *input_bg_TTJets     = TFile::Open( fname_bg_TTJets );/*{{{*/
   TFile *input_bg_TTGJets   = TFile::Open( fname_bg_TTGJets )     ;
   TFile *input_bg_ttZJets   = TFile::Open( fname_bg_ttZJets )     ;
   TFile *input_bg_ttWJets   = TFile::Open( fname_bg_ttWJets )     ;
   TFile *input_bg_ttH   = TFile::Open( fname_bg_ttH )     ;
   // TFile *input_bg_ttbb   = TFile::Open( fname_bg_ttbb )     ;
    //12
   TFile *input_bg_WZ   = TFile::Open( fname_bg_WZ )     ;
   // TFile *input_bg_WWTo2L2Nu   = TFile::Open( fname_bg_WWTo2L2Nu )     ;
   TFile *input_bg_WpWpJJ   = TFile::Open( fname_bg_WpWpJJ )     ;
   TFile *input_bg_ZZ   = TFile::Open( fname_bg_ZZ )     ;
   TFile *input_bg_WGJets   = TFile::Open( fname_bg_WGJets )     ;
   TFile *input_bg_ZGJetsToLLG   = TFile::Open( fname_bg_ZGJetsToLLG )     ;
    //20
   TFile *input_bg_WWW   = TFile::Open( fname_bg_WWW )     ;
   TFile *input_bg_WWZ   = TFile::Open( fname_bg_WWZ )     ;
   TFile *input_bg_WWG   = TFile::Open( fname_bg_WWG )     ;
   TFile *input_bg_ZZZ   = TFile::Open( fname_bg_ZZZ )     ;
   TFile *input_bg_WZZ   = TFile::Open( fname_bg_WZZ )     ;
   TFile *input_bg_WZG   = TFile::Open( fname_bg_WZG )     ;
   TFile *input_bg_WGG   = TFile::Open( fname_bg_WGG )     ;
   TFile *input_bg_ZGGJets   = TFile::Open( fname_bg_ZGGJets )     ;
    //22
   TFile *input_bg_WJetsToLNu   = TFile::Open( fname_bg_WJetsToLNu )     ;

   TFile *input_bg_DYJetsToTauTau   = TFile::Open( fname_bg_DYJetsToTauTau )     ;
    //28
   TFile *input_bg_tZq_ll   = TFile::Open( fname_bg_tZq_ll )     ;
   TFile *input_bg_ST_tW_antitop   = TFile::Open( fname_bg_ST_tW_antitop )     ;
   TFile *input_bg_ST_tW_top   = TFile::Open( fname_bg_ST_tW_top )     ;
   TFile *input_bg_TGJets   = TFile::Open( fname_bg_TGJets )     ;
   TFile *input_bg_THW   = TFile::Open( fname_bg_THW )     ;
   TFile *input_bg_THQ   = TFile::Open( fname_bg_THQ )     ;
    //38
   TFile *input_bg_VHToNonbb   = TFile::Open( fname_bg_VHToNonbb )     ;
   TFile *input_bg_ZHToTauTau   = TFile::Open( fname_bg_ZHToTauTau )     ;
   TFile *input_bg_ZH_HToBB   = TFile::Open( fname_bg_ZH_HToBB )     ;
   TFile *input_bg_GluGluHToZZTo4L   = TFile::Open( fname_bg_GluGluHToZZTo4L )     ;
   TFile *input_bg_GluGluHToBB   = TFile::Open( fname_bg_GluGluHToBB )     ;
   TFile *input_bg_GluGluHToGG   = TFile::Open( fname_bg_GluGluHToGG )     ;
   // TFile *input_bg_GluGluHToMuMu   = TFile::Open( fname_bg_GluGluHToMuMu )     ;
   // TFile *input_bg_GluGluHToTauTau   = TFile::Open( fname_bg_GluGluHToTauTau )     ;
   // TFile *input_bg_GluGluHToWWTo2L2Nu   = TFile::Open( fname_bg_GluGluHToWWTo2L2Nu )     ;
   // TFile *input_bg_GluGluHToWWToLNuQQ   = TFile::Open( fname_bg_GluGluHToWWToLNuQQ )     ;
    //41
   // TFile *input_bg_VBFHToWWTo2L2Nu   = TFile::Open( fname_bg_VBFHToWWTo2L2Nu )     ;
   // TFile *input_bg_VBFHToMuMu   = TFile::Open( fname_bg_VBFHToMuMu )     ;
   // TFile *input_bg_VBFHToGG   = TFile::Open( fname_bg_VBFHToGG )     ;[>}}}<]
   
   // std::cout << "--- TMVAClassification       : Using input file: " << input->GetName() << std::endl;
   std::cout << "--- TMVAClassification       : Using input file: " << input_signal->GetName() << std::endl;
   std::cout << "--- TMVAClassification       : Using input file: " << input_bg_TTJets->GetName() << std::endl;
   
   // --- Register the training and test trees

   // TTree *signal     = (TTree*)input->Get("TreeS");
   // TTree *background = (TTree*)input->Get("TreeB");
   TTree *signal     = (TTree*)input_signal->Get("tree");

   TTree *bg_TTJets = (TTree*)input_bg_TTJets->Get("tree");/*{{{*/
   TTree *bg_TTGJets  = (TTree*)input_bg_TTGJets->Get("tree")         ;
   TTree *bg_ttZJets  = (TTree*)input_bg_ttZJets->Get("tree")         ;
   TTree *bg_ttWJets  = (TTree*)input_bg_ttWJets->Get("tree")         ;
   TTree *bg_ttH  = (TTree*)input_bg_ttH->Get("tree")         ;
   // TTree *bg_ttbb  = (TTree*)input_bg_ttbb->Get("tree")         ;
    //12
   TTree *bg_WZ  = (TTree*)input_bg_WZ->Get("tree")         ;
   // TTree *bg_WWTo2L2Nu  = (TTree*)input_bg_WWTo2L2Nu->Get("tree")         ;
   TTree *bg_WpWpJJ  = (TTree*)input_bg_WpWpJJ->Get("tree")         ;
   TTree *bg_ZZ  = (TTree*)input_bg_ZZ->Get("tree")         ;
   TTree *bg_WGJets  = (TTree*)input_bg_WGJets->Get("tree")         ;
   TTree *bg_ZGJetsToLLG  = (TTree*)input_bg_ZGJetsToLLG->Get("tree")         ;
    //20
   TTree *bg_WWW  = (TTree*)input_bg_WWW->Get("tree")         ;
   TTree *bg_WWZ  = (TTree*)input_bg_WWZ->Get("tree")         ;
   TTree *bg_WWG  = (TTree*)input_bg_WWG->Get("tree")         ;
   TTree *bg_ZZZ  = (TTree*)input_bg_ZZZ->Get("tree")         ;
   TTree *bg_WZZ  = (TTree*)input_bg_WZZ->Get("tree")         ;
   TTree *bg_WZG  = (TTree*)input_bg_WZG->Get("tree")         ;
   TTree *bg_WGG  = (TTree*)input_bg_WGG->Get("tree")         ;
   TTree *bg_ZGGJets  = (TTree*)input_bg_ZGGJets->Get("tree")         ;
    //22
   TTree *bg_WJetsToLNu  = (TTree*)input_bg_WJetsToLNu->Get("tree")         ;

   TTree *bg_DYJetsToTauTau  = (TTree*)input_bg_DYJetsToTauTau->Get("tree")         ;
    //28
   TTree *bg_tZq_ll  = (TTree*)input_bg_tZq_ll->Get("tree")         ;
   TTree *bg_ST_tW_antitop  = (TTree*)input_bg_ST_tW_antitop->Get("tree")         ;
   TTree *bg_ST_tW_top  = (TTree*)input_bg_ST_tW_top->Get("tree")         ;
   TTree *bg_TGJets  = (TTree*)input_bg_TGJets->Get("tree")         ;
   TTree *bg_THW  = (TTree*)input_bg_THW->Get("tree")         ;
   TTree *bg_THQ  = (TTree*)input_bg_THQ->Get("tree")         ;
    //38
   TTree *bg_VHToNonbb  = (TTree*)input_bg_VHToNonbb->Get("tree")         ;
   TTree *bg_ZHToTauTau  = (TTree*)input_bg_ZHToTauTau->Get("tree")         ;
   TTree *bg_ZH_HToBB  = (TTree*)input_bg_ZH_HToBB->Get("tree")         ;
   TTree *bg_GluGluHToZZTo4L  = (TTree*)input_bg_GluGluHToZZTo4L->Get("tree")         ;
   TTree *bg_GluGluHToBB  = (TTree*)input_bg_GluGluHToBB->Get("tree")         ;
   TTree *bg_GluGluHToGG  = (TTree*)input_bg_GluGluHToGG->Get("tree")         ;
   // TTree *bg_GluGluHToMuMu  = (TTree*)input_bg_GluGluHToMuMu->Get("tree")         ;
   // TTree *bg_GluGluHToTauTau  = (TTree*)input_bg_GluGluHToTauTau->Get("tree")         ;
   // TTree *bg_GluGluHToWWTo2L2Nu  = (TTree*)input_bg_GluGluHToWWTo2L2Nu->Get("tree")         ;
   // TTree *bg_GluGluHToWWToLNuQQ  = (TTree*)input_bg_GluGluHToWWToLNuQQ->Get("tree")         ;
    //41
   // TTree *bg_VBFHToWWTo2L2Nu  = (TTree*)input_bg_VBFHToWWTo2L2Nu->Get("tree")         ;
   // TTree *bg_VBFHToMuMu  = (TTree*)input_bg_VBFHToMuMu->Get("tree")         ;
   // TTree *bg_VBFHToGG  = (TTree*)input_bg_VBFHToGG->Get("tree")         ;[>}}}<]
   
   // global event weights per tree (see below for setting event-wise weights)
   // Double_t signalWeight     = 1.0;
   // Double_t backgroundWeight = 1.0;
   Double_t LUMI = 35900;
   Double_t signalWeight     = (LUMI*0.01197)/(1709406-704054);
   // Double_t bgWeight_TTJets =  (LUMI*746.7)/(29509487-14335648);
    //6
    Double_t wTTJets = (LUMI*746.7)/(29509487-14335648);//746.7 // TTJets_TuneCUETP8M2T4_13TeV-amcatnloFXFX-pythia8.root:  Positive:29509487  Negtive:14335648{{{
    Double_t wTTGJets = (LUMI*3.773)/(3224372-1646539);// TTGJets_TuneCUETP8M1_13TeV-amcatnloFXFX-madspin-pythia8.root:  Positive:3224372  Negtive:1646539  ;
    Double_t wttZJets = (LUMI*0.6559)/(9883364-0) ;// ttZJets_13TeV_madgraphMLM-pythia8.root:  Positive:9883364  Negtive:0      //Special care is taken when scaling the ttZ background to the cr
    Double_t wttWJets = (LUMI*0.2014)/(6700440-0);// ttWJets_13TeV_madgraphMLM.root:  Positive:6700440  Negtive:0  ;
    Double_t wttH= (LUMI*0.3372)/(9566400-0);// ttH_4f_ctcvcp_TuneCP5_13TeV_madgraph_pythia8.root:  Positive:9566400  Negtive:0  ;
    // Double_t wttbb= (LUMI*1.393)/(2556073-1427835);// ttbb_4FS_ckm_amcatnlo_madspin_pythia8.root:  Positive:2556073  Negtive:1427835  ;
    //12
    Double_t wWZ= (LUMI*2.343)/(2997571-0);// WZ_TuneCUETP8M1_13TeV-pythia8.root:  Positive:2997571  Negtive:0  ;
    // Double_t wWWTo2L2Nu = (LUMI*0.1697)/(999367-0); // WWTo2L2Nu_DoubleScattering_13TeV-pythia8.root:  Positive:999367  Negtive:0  ;
    Double_t wWpWpJJ= (LUMI*0.05390)/(149681-0);// WpWpJJ_EWK-QCD_TuneCUETP8M1_13TeV-madgraph-pythia8.root:  Positive:149681  Negtive:0  ;
    Double_t wZZ  = (LUMI*1.016)/(998034-0);// ZZ_TuneCUETP8M1_13TeV-pythia8.root:  Positive:998034  Negtive:0  ;
    Double_t wWGJets = (LUMI*1.269)/(5077680-0);// WGJets_MonoPhoton_PtG-40to130_TuneCUETP8M1_13TeV-madgraph.root:  Positive:5077680  Negtive:0  ;
    Double_t wZGJetsToLLG = (LUMI*0.1319)/(498406-0);// ZGJetsToLLG_EW_LO_13TeV-sherpa.root:  Positive:498406  Negtive:394  ;
    //20
    Double_t wWWW= (LUMI*0.2086)/(225269-14731);// WWW_4F_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root:  Positive:225269  Negtive:14731  ;
    Double_t wWWZ= (LUMI*0.1651)/(235734-14266);// WWZ_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root:  Positive:235734  Negtive:14266  ;
    Double_t wWWG = (LUMI*0.2147)/(913515-85885);// WWG_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root:  Positive:913515  Negtive:85885  ;
    Double_t wZZZ= (LUMI*0.01398)/(231217-18020);// ZZZ_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root:  Positive:231217  Negtive:18020  ;
    Double_t wWZZ= (LUMI*0.05565)/(231583-15217);// WZZ_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root:  Positive:231583  Negtive:15217  ;
    Double_t wWZG= (LUMI*0.04123)/(921527-76673);// WZG_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root:  Positive:921527  Negtive:76673  ;
    Double_t wWGG =(LUMI*1.819)/(889832-110168); // WGG_5f_TuneCUETP8M1_13TeV-amcatnlo-pythia8.root:  Positive:889832  Negtive:110168  ;
    Double_t wZGGJets= (LUMI*0.3717)/(291922-0);// ZGGJets_ZToHadOrNu_5f_LO_madgraph_pythia8.root:  Positive:291922  Negtive:0  ;
    //22
    Double_t wWJetsToLNu= (LUMI*50300)/(29514020-0) ;// WJetsToLNu_TuneCUETP8M1_13TeV-madgraphMLM-pythia8.root:  Positive:29514020  Negtive:0  ;
    Double_t wDYJetsToTauTau= (LUMI*1983)/(21891344-4380454);// DYJetsToTauTau_ForcedMuEleDecay_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8_ext1.root
    //28
    Double_t wtZq_ll= (LUMI*0.07358)/(8784890-5147710);// tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8.root:  Positive:8784890  Negtive:5147710  ;
    Double_t wST_tW_antitop= (LUMI*38.06)/(418378-0);// ST_tW_antitop_5f_inclusiveDecays_13TeV-powheg-pythia8_TuneCUETP8M2T4.root:  Positive:418378  Negtive:0  ;
    Double_t wST_tW_top= (LUMI*38.09)/(992024-0);//  ST_tW_top_5f_inclusiveDecays_13TeV-powheg-pythia8_TuneCUETP8M2T4.root:  Positive:992024  Negtive:0  ;
    Double_t wTGJets= (LUMI*2.967)/(933719-623277);// TGJets_TuneCUETP8M1_13TeV_amcatnlo_madspin_pythia8.root:  Positive:933719  Negtive:623277  ;
    Double_t wTHW= (LUMI*0.1467)/(4995329-2967);// THW_ctcvcp_HIncl_M125_TuneCP5_13TeV-madgraph-pythia8.root:  Positive:4995329  Negtive:2967  ;
    Double_t wTHQ= (LUMI*0.8816)/(9829911-0);// THQ_ctcvcp_Hincl_13TeV-madgraph-pythia8_TuneCUETP8M1.root:  Positive:9829911  Negtive:0  ;

    //38
    Double_t wVHToNonbb= (LUMI*2.137)/(799942-297663);// VHToNonbb_M125_13TeV_amcatnloFXFX_madspin_pythia8.root:  Positive:799942  Negtive:297663  ;
    Double_t wZHToTauTau= (LUMI*0.7524)/(581490-18313);// ZHToTauTau_M125_13TeV_powheg_pythia8.root:  Positive:581490  Negtive:18313  ;
    Double_t wZH_HToBB_ZToLL= (LUMI*0.07523)/(1920440-59420);// ZH_HToBB_ZToLL_M125_13TeV_powheg_pythia8.root:  Positive:1920440  Negtive:59420  ;
    Double_t wGluGluHToZZTo4L= (LUMI*2.999)/(999800-0);// GluGluHToZZTo4L_M125_13TeV_powheg2_JHUgenV6_pythia8.root:  Positive:999800  Negtive:0  ;
    Double_t wGluGluHToBB= (LUMI*32.10)/(2946318-853055);// GluGluHToBB_M125_13TeV_amcatnloFXFX_pythia8.root:  Positive:2946318  Negtive:853055  ;
    Double_t wGluGluHToGG= (LUMI*31.98)/(335240-96369);// GluGluHToGG_M125_13TeV_amcatnloFXFX_pythia8.root:  Positive:335240  Negtive:96369  ;
    // Double_t wGluGluHToMuMu= (LUMI*29.99)/(1991200-0);// GluGluHToMuMu_M-125_TuneCP5_PSweights_13TeV_powheg_pythia8.root:  Positive:1991200  Negtive:0  ;
    // Double_t wGluGluHToTauTau= (LUMI*30.52)/(1497800-0);// GluGluHToTauTau_M125_13TeV_powheg_pythia8.root:  Positive:1497800  Negtive:0  ;
    // Double_t wGluGluHToWWTo2L2Nu= (LUMI*30.52)/(492200-0);// GluGluHToWWTo2L2Nu_M125_13TeV_powheg_JHUgen_pythia8.root:  Positive:492200  Negtive:0  ;
    // Double_t wGluGluHToWWToLNuQQ= (LUMI*29.99)/(198000-0);// GluGluHToWWToLNuQQ_M125_13TeV_powheg_JHUGenV628_pythia8.root:  Positive:198000  Negtive:0  ;
    //41
    // Double_t wVBFHToWWTo2L2Nu= (LUMI*3.769)/(99931-69);// VBFHToWWTo2L2Nu_M125_13TeV_powheg_JHUgenv628_pythia8.root:  Positive:99931  Negtive:69  ;
    // Double_t wVBFHToMuMu= (LUMI*0.000823)/(996835-765);// VBFHToMuMu_M-125_TuneCP5_PSweights_13TeV_powheg_pythia8.root:  Positive:996835  Negtive:765  ;
    // Double_t wVBFHToGG= (LUMI*3.992)/(639138-338962);// VBFHToGG_M125_13TeV_amcatnlo_pythia8_v2.root:  Positive:639138  Negtive:338962  ;}}}



   // You can add an arbitrary number of signal or background trees
   factory->AddSignalTree    ( signal,     signalWeight     );
   // factory->AddBackgroundTree( bg_TTJets, bgWeight_TTJets );
   /*{{{*/

   factory->AddBackgroundTree(bg_TTJets , wTTJets );
   factory->AddBackgroundTree(bg_TTGJets  , wTTGJets          );
   factory->AddBackgroundTree(bg_ttZJets  , wttZJets          );
   factory->AddBackgroundTree(bg_ttWJets  , wttWJets          );
   factory->AddBackgroundTree(bg_ttH  , wttH          );
   // factory->AddBackgroundTree(bg_ttbb  , wttbb          );
    //12
   factory->AddBackgroundTree(bg_WZ  , wWZ          );
   // factory->AddBackgroundTree(bg_WWTo2L2Nu  , wWWTo2L2Nu          );
   factory->AddBackgroundTree(bg_WpWpJJ  , wWpWpJJ          );
   factory->AddBackgroundTree(bg_ZZ  , wZZ          );
   factory->AddBackgroundTree(bg_WGJets  , wWGJets          );
   factory->AddBackgroundTree(bg_ZGJetsToLLG  , wZGJetsToLLG          );
    //20
   factory->AddBackgroundTree(bg_WWW  , wWWW          );
   factory->AddBackgroundTree(bg_WWZ  , wWWZ          );
   factory->AddBackgroundTree(bg_WWG  , wWWG          );
   factory->AddBackgroundTree(bg_ZZZ  , wZZZ          );
   factory->AddBackgroundTree(bg_WZZ  , wWZZ          );
   factory->AddBackgroundTree(bg_WZG  , wWZG          );
   factory->AddBackgroundTree(bg_WGG  , wWGG          );
   factory->AddBackgroundTree(bg_ZGGJets  , wZGGJets          );
    //22
   factory->AddBackgroundTree(bg_WJetsToLNu  , wWJetsToLNu          );

   factory->AddBackgroundTree(bg_DYJetsToTauTau  , wDYJetsToTauTau          );
    //28
   factory->AddBackgroundTree(bg_tZq_ll  , wtZq_ll          );
   factory->AddBackgroundTree(bg_ST_tW_antitop  , wST_tW_antitop          );
   factory->AddBackgroundTree(bg_ST_tW_top  , wST_tW_top          );
   factory->AddBackgroundTree(bg_TGJets  , wTGJets          );
   factory->AddBackgroundTree(bg_THW  , wTHW          );
   factory->AddBackgroundTree(bg_THQ  , wTHQ          );
    //38
   factory->AddBackgroundTree(bg_VHToNonbb  , wVHToNonbb          );
   factory->AddBackgroundTree(bg_ZHToTauTau  , wZHToTauTau          );
   factory->AddBackgroundTree(bg_ZH_HToBB  , wZH_HToBB_ZToLL          );// not consistent
   factory->AddBackgroundTree(bg_GluGluHToZZTo4L  , wGluGluHToZZTo4L          );
   factory->AddBackgroundTree(bg_GluGluHToBB  , wGluGluHToBB          );
   std::cout << __LINE__ << endl; 
   factory->AddBackgroundTree(bg_GluGluHToGG  , wGluGluHToGG          );//0 events
   // factory->AddBackgroundTree(bg_GluGluHToMuMu  , wGluGluHToMuMu          );
   // factory->AddBackgroundTree(bg_GluGluHToTauTau  , wGluGluHToTauTau          );
   // factory->AddBackgroundTree(bg_GluGluHToWWTo2L2Nu  , wGluGluHToWWTo2L2Nu          );
   // factory->AddBackgroundTree(bg_GluGluHToWWToLNuQQ  , wGluGluHToWWToLNuQQ          );
    //41
   // factory->AddBackgroundTree(bg_VBFHToWWTo2L2Nu  , wVBFHToWWTo2L2Nu          );
   // factory->AddBackgroundTree(bg_VBFHToMuMu  , wVBFHToMuMu          );
   // factory->AddBackgroundTree(bg_VBFHToGG  , wVBFHToGG          );[>}}}<]
   
   // To give different trees for training and testing, do as follows:
   //    factory->AddSignalTree( signalTrainingTree, signalTrainWeight, "Training" );
   //    factory->AddSignalTree( signalTestTree,     signalTestWeight,  "Test" );
   
   // Use the following code instead of the above two or four lines to add signal and background
   // training and test events "by hand"
   // NOTE that in this case one should not give expressions (such as "var1+var2") in the input
   //      variable definition, but simply compute the expression before adding the event
   //
   //     // --- begin ----------------------------------------------------------
   //     std::vector<Double_t> vars( 4 ); // vector has size of number of input variables
   //     Float_t  treevars[4], weight;
   //     
   //     // Signal
   //     for (UInt_t ivar=0; ivar<4; ivar++) signal->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   //     for (UInt_t i=0; i<signal->GetEntries(); i++) {
   //        signal->GetEntry(i);
   //        for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
   //        // add training and test events; here: first half is training, second is testing
   //        // note that the weight can also be event-wise
   //        if (i < signal->GetEntries()/2.0) factory->AddSignalTrainingEvent( vars, signalWeight );
   //        else                              factory->AddSignalTestEvent    ( vars, signalWeight );
   //     }
   //   
   //     // Background (has event weights)
   //     background->SetBranchAddress( "weight", &weight );
   //     for (UInt_t ivar=0; ivar<4; ivar++) background->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   //     for (UInt_t i=0; i<background->GetEntries(); i++) {
   //        background->GetEntry(i);
   //        for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
   //        // add training and test events; here: first half is training, second is testing
   //        // note that the weight can also be event-wise
   //        if (i < background->GetEntries()/2) factory->AddBackgroundTrainingEvent( vars, backgroundWeight*weight );
   //        else                                factory->AddBackgroundTestEvent    ( vars, backgroundWeight*weight );
   //     }
         // --- end ------------------------------------------------------------
   //
   // --- end of tree registration 

   // Set individual event weights (the variables must exist in the original TTree)
   //    for signal    : factory->SetSignalWeightExpression    ("weight1*weight2");
   //    for background: factory->SetBackgroundWeightExpression("weight1*weight2");
   // factory->SetBackgroundWeightExpression( "weight" );//bg tree has a weight branch

   // Apply additional cuts on the signal and background samples (can be different)
   TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
   TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

   // Tell the factory how to use the training and testing events
   //
   // If no numbers of events are given, half of the events in the tree are used 
   // for training, and the other half for testing:
   //    factory->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );
   // To also specify the number of testing events, use:
   //    factory->PrepareTrainingAndTestTree( mycut,
   //                                         "NSigTrain=3000:NBkgTrain=3000:NSigTest=3000:NBkgTest=3000:SplitMode=Random:!V" );
   factory->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
   //?how to prepare 70% and 30%?
   // ---- Book MVA methods
   //
   // Please lookup the various method configuration options in the corresponding cxx files, eg:
   // src/MethoCuts.cxx, etc, or here: http://tmva.sourceforge.net/optionRef.html
   // it is possible to preset ranges in the option string in which the cut optimisation should be done:
   // "...:CutRangeMin[2]=-1:CutRangeMax[2]=1"...", where [2] is the third input variable

   // Cut optimisation
   if (Use["Cuts"])
      factory->BookMethod( TMVA::Types::kCuts, "Cuts",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart" );

   if (Use["CutsD"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsD",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=Decorrelate" );

   if (Use["CutsPCA"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsPCA",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=PCA" );

   if (Use["CutsGA"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsGA",
                           "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=30:Cycles=3:PopSize=400:SC_steps=10:SC_rate=5:SC_factor=0.95" );

   if (Use["CutsSA"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsSA",
                           "!H:!V:FitMethod=SA:EffSel:MaxCalls=150000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

   // Likelihood ("naive Bayes estimator")
   if (Use["Likelihood"])
      factory->BookMethod( TMVA::Types::kLikelihood, "Likelihood",
                           "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" );

   // Decorrelated likelihood
   if (Use["LikelihoodD"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodD",
                           "!H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" );

   // PCA-transformed likelihood
   if (Use["LikelihoodPCA"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodPCA",
                           "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" ); 

   // Use a kernel density estimator to approximate the PDFs
   if (Use["LikelihoodKDE"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodKDE",
                           "!H:!V:!TransformOutput:PDFInterpol=KDE:KDEtype=Gauss:KDEiter=Adaptive:KDEFineFactor=0.3:KDEborder=None:NAvEvtPerBin=50" ); 

   // Use a variable-dependent mix of splines and kernel density estimator
   if (Use["LikelihoodMIX"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodMIX",
                           "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" ); 

   // Test the multi-dimensional probability density estimator
   // here are the options strings for the MinMax and RMS methods, respectively:
   //      "!H:!V:VolumeRangeMode=MinMax:DeltaFrac=0.2:KernelEstimator=Gauss:GaussSigma=0.3" );
   //      "!H:!V:VolumeRangeMode=RMS:DeltaFrac=3:KernelEstimator=Gauss:GaussSigma=0.3" );
   if (Use["PDERS"])
      factory->BookMethod( TMVA::Types::kPDERS, "PDERS",
                           "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" );

   if (Use["PDERSD"])
      factory->BookMethod( TMVA::Types::kPDERS, "PDERSD",
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=Decorrelate" );

   if (Use["PDERSPCA"])
      factory->BookMethod( TMVA::Types::kPDERS, "PDERSPCA",
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" );

   // Multi-dimensional likelihood estimator using self-adapting phase-space binning
   if (Use["PDEFoam"])
      factory->BookMethod( TMVA::Types::kPDEFoam, "PDEFoam",
                           "!H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=2000:nBin=5:Nmin=100:Kernel=None:Compress=T" );

   if (Use["PDEFoamBoost"])
      factory->BookMethod( TMVA::Types::kPDEFoam, "PDEFoamBoost",
                           "!H:!V:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T" );

   // K-Nearest Neighbour classifier (KNN)
   if (Use["KNN"])
      factory->BookMethod( TMVA::Types::kKNN, "KNN",
                           "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" );

   // H-Matrix (chi2-squared) method
   if (Use["HMatrix"])
      factory->BookMethod( TMVA::Types::kHMatrix, "HMatrix", "!H:!V:VarTransform=None" );

   // Linear discriminant (same as Fisher discriminant)
   if (Use["LD"])
      factory->BookMethod( TMVA::Types::kLD, "LD", "H:!V:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

   // Fisher discriminant (same as LD)
   if (Use["Fisher"])
      factory->BookMethod( TMVA::Types::kFisher, "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

   // Fisher with Gauss-transformed input variables
   if (Use["FisherG"])
      factory->BookMethod( TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" );

   // Composite classifier: ensemble (tree) of boosted Fisher classifiers
   if (Use["BoostedFisher"])
      factory->BookMethod( TMVA::Types::kFisher, "BoostedFisher", 
                           "H:!V:Boost_Num=20:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2:!Boost_DetailedMonitoring" );

   // Function discrimination analysis (FDA) -- test of various fitters - the recommended one is Minuit (or GA or SA)
   if (Use["FDA_MC"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_MC",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:SampleSize=100000:Sigma=0.1" );

   if (Use["FDA_GA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_GA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" );

   if (Use["FDA_SA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_SA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=SA:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

   if (Use["FDA_MT"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_MT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" );

   if (Use["FDA_GAMT"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_GAMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=1:PopSize=5:Steps=5:Trim" );

   if (Use["FDA_MCMT"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_MCMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=20" );

   // TMVA ANN: MLP (recommended ANN) -- all ANNs in TMVA are Multilayer Perceptrons
   if (Use["MLP"])
      factory->BookMethod( TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator" );

   if (Use["MLPBFGS"])
      factory->BookMethod( TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" );

   if (Use["MLPBNN"])
      factory->BookMethod( TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" ); // BFGS training with bayesian regulators

   // CF(Clermont-Ferrand)ANN
   if (Use["CFMlpANN"])
      factory->BookMethod( TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=2000:HiddenLayers=N+1,N"  ); // n_cycles:#nodes:#nodes:...  

   // Tmlp(Root)ANN
   if (Use["TMlpANN"])
      factory->BookMethod( TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  ); // n_cycles:#nodes:#nodes:...

   // Support Vector Machine
   if (Use["SVM"])
      factory->BookMethod( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" );

   // Boosted Decision Trees
   if (Use["BDTG"]) // Gradient Boost
      factory->BookMethod( TMVA::Types::kBDT, "BDTG",
                           "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );

   if (Use["BDT"])  // Adaptive Boost
      factory->BookMethod( TMVA::Types::kBDT, "BDT",
                           "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );

   if (Use["BDTB"]) // Bagging
      factory->BookMethod( TMVA::Types::kBDT, "BDTB",
                           "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20" );

   if (Use["BDTD"]) // Decorrelation + Adaptive Boost
      factory->BookMethod( TMVA::Types::kBDT, "BDTD",
                           "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );

   if (Use["BDTF"])  // Allow Using Fisher discriminant in node splitting for (strong) linearly correlated variables
      factory->BookMethod( TMVA::Types::kBDT, "BDTMitFisher",
                           "!H:!V:NTrees=50:MinNodeSize=2.5%:UseFisherCuts:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:SeparationType=GiniIndex:nCuts=20" );

   // RuleFit -- TMVA implementation of Friedman's method
   if (Use["RuleFit"])
      factory->BookMethod( TMVA::Types::kRuleFit, "RuleFit",
                           "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" );

   // For an example of the category classifier usage, see: TMVAClassificationCategory

   // --------------------------------------------------------------------------------------------------

   // ---- Now you can optimize the setting (configuration) of the MVAs using the set of training events

   // ---- STILL EXPERIMENTAL and only implemented for BDT's ! 
   // factory->OptimizeAllMethods("SigEffAt001","Scan");
   // factory->OptimizeAllMethods("ROCIntegral","FitGA");

   // --------------------------------------------------------------------------------------------------

   // ---- Now you can tell the factory to train, test, and evaluate the MVAs

   // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();

   // --------------------------------------------------------------

   // Save the output
   outputFile->Close();

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> TMVAClassification is done!" << std::endl;

   delete factory;

   // Launch the GUI for the root macros
   if (!gROOT->IsBatch()) TMVAGui( outfileName );
}
