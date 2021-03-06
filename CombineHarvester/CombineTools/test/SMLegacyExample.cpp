#include <string>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include "boost/filesystem.hpp"
#include "CombineTools/interface/CombineHarvester.h"
#include "CombineTools/interface/Utilities.h"
#include "CombineTools/interface/HttSystematics.h"
#include "CombineTools/interface/CardWriter.h"
#include "CombineTools/interface/CopyTools.h"
#include "CombineTools/interface/BinByBin.h"

using namespace std;

int main() {
  ch::CombineHarvester cb;

  typedef vector<pair<int, string>> Categories;
  typedef vector<string> VString;

  string auxiliaries  = string(getenv("CMSSW_BASE")) + "/src/auxiliaries/";
  string aux_shapes   = auxiliaries +"shapes/";
  string aux_pruning  = auxiliaries +"pruning/";

  VString chns =
      {"et", "mt", "em", "ee", "mm", "tt"};

  map<string, string> input_folders = {
      {"et", "Imperial"},
      {"mt", "Imperial"},
      {"em", "MIT"},
      {"ee", "DESY-KIT"},
      {"mm", "DESY-KIT"},
      {"tt", "CERN"}
  };

  map<string, VString> bkg_procs;
  bkg_procs["et"] = {"ZTT", "W", "QCD", "ZL", "ZJ", "TT", "VV"};
  bkg_procs["mt"] = {"ZTT", "W", "QCD", "ZL", "ZJ", "TT", "VV"};
  bkg_procs["em"] = {"Ztt", "EWK", "Fakes", "ttbar", "ggH_hww125", "qqH_hww125"};
  bkg_procs["ee"] = {"ZTT", "WJets", "QCD", "ZEE", "TTJ", "Dibosons", "ggH_hww125", "qqH_hww125"};
  bkg_procs["mm"] = {"ZTT", "WJets", "QCD", "ZMM", "TTJ", "Dibosons", "ggH_hww125", "qqH_hww125"};
  bkg_procs["tt"] = {"ZTT", "W", "QCD", "ZL", "ZJ", "TT", "VV"};

  VString sig_procs = {"ggH", "qqH", "WH", "ZH"};

  map<string, Categories> cats;
  cats["et_7TeV"] = {
      {1, "eleTau_0jet_medium"}, {2, "eleTau_0jet_high"},
      {3, "eleTau_1jet_medium"}, {5, "eleTau_1jet_high_mediumhiggs"},
      {6, "eleTau_vbf"}};

  cats["et_8TeV"] = {
      {1, "eleTau_0jet_medium"}, {2, "eleTau_0jet_high"},
      {3, "eleTau_1jet_medium"}, {5, "eleTau_1jet_high_mediumhiggs"},
      {6, "eleTau_vbf_loose"}, {7, "eleTau_vbf_tight"}};

  cats["mt_7TeV"] = {
      {1, "muTau_0jet_medium"}, {2, "muTau_0jet_high"},
      {3, "muTau_1jet_medium"}, {4, "muTau_1jet_high_lowhiggs"}, {5, "muTau_1jet_high_mediumhiggs"},
      {6, "muTau_vbf"}};

  cats["mt_8TeV"] = {
      {1, "muTau_0jet_medium"}, {2, "muTau_0jet_high"},
      {3, "muTau_1jet_medium"}, {4, "muTau_1jet_high_lowhiggs"}, {5, "muTau_1jet_high_mediumhiggs"},
      {6, "muTau_vbf_loose"}, {7, "muTau_vbf_tight"}};

  cats["em_7TeV"] = {
      {0, "emu_0jet_low"}, {1, "emu_0jet_high"},
      {2, "emu_1jet_low"}, {3, "emu_1jet_high"},
      {4, "emu_vbf_loose"}};

  cats["em_8TeV"] = {
      {0, "emu_0jet_low"}, {1, "emu_0jet_high"},
      {2, "emu_1jet_low"}, {3, "emu_1jet_high"},
      {4, "emu_vbf_loose"}, {5, "emu_vbf_tight"}};

  cats["ee_7TeV"] = {
      {0, "ee_0jet_low"}, {1, "ee_0jet_high"},
      {2, "ee_1jet_low"}, {3, "ee_1jet_high"},
      {4, "ee_vbf"}};
  cats["ee_8TeV"] = cats["ee_7TeV"];

  cats["mm_7TeV"] = {
      {0, "mumu_0jet_low"}, {1, "mumu_0jet_high"},
      {2, "mumu_1jet_low"}, {3, "mumu_1jet_high"},
      {4, "mumu_vbf"}};
  cats["mm_8TeV"] = cats["mm_7TeV"];

  cats["tt_8TeV"] = {
      {0, "tauTau_1jet_high_mediumhiggs"}, {1, "tauTau_1jet_high_highhiggs"},
      {2, "tauTau_vbf"}};

  vector<string> masses = ch::ValsFromRange("110:145|5");

  cout << ">> Creating processes and observations...\n";
  for (string era : {"7TeV", "8TeV"}) {
    for (auto chn : chns) {
      cb.AddObservations(
        {"*"}, {"htt"}, {era}, {chn}, cats[chn+"_"+era]);
      cb.AddProcesses(
        {"*"}, {"htt"}, {era}, {chn}, bkg_procs[chn], cats[chn+"_"+era], false);
      cb.AddProcesses(
        masses, {"htt"}, {era}, {chn}, sig_procs, cats[chn+"_"+era], true);
    }
  }
  // Have to drop ZL from tautau_vbf category
  cb.FilterProcs([](ch::Process const* p) {
    return p->bin() == "tauTau_vbf" && p->process() == "ZL";
  });

  cout << ">> Adding systematic uncertainties...\n";
  ch::AddSystematics_et_mt(cb);
  ch::AddSystematics_em(cb);
  ch::AddSystematics_ee_mm(cb);
  ch::AddSystematics_tt(cb);

  cout << ">> Extracting histograms from input root files...\n";
  for (string era : {"7TeV", "8TeV"}) {
    for (string chn : chns) {
      // Skip 7TeV tt:
      if (chn == "tt" && era == "7TeV") continue;
      string file = aux_shapes + input_folders[chn] + "/htt_" + chn +
                    ".inputs-sm-" + era + "-hcg.root";
      cb.cp().channel({chn}).era({era}).backgrounds().ExtractShapes(
          file, "$BIN/$PROCESS", "$BIN/$PROCESS_$SYSTEMATIC");
      cb.cp().channel({chn}).era({era}).signals().ExtractShapes(
          file, "$BIN/$PROCESS$MASS", "$BIN/$PROCESS$MASS_$SYSTEMATIC");
    }
  }

  cout << ">> Scaling signal process rates...\n";
  map<string, TGraph> xs;
  // Get the table of H->tau tau BRs vs mass
  xs["htt"] = ch::TGraphFromTable("input/xsecs_brs/htt_YR3.txt", "mH", "br");
  for (string const& e : {"7TeV", "8TeV"}) {
    for (string const& p : sig_procs) {
      // Get the table of xsecs vs mass for process "p" and era "e":
      xs[p+"_"+e] = ch::TGraphFromTable("input/xsecs_brs/"+p+"_"+e+"_YR3.txt", "mH", "xsec");
      cout << ">>>> Scaling for process " << p << " and era " << e << "\n";
      cb.cp().process({p}).era({e}).ForEachProc([&](ch::Process *proc) {
        double m = boost::lexical_cast<double>(proc->mass());
        proc->set_rate(proc->rate() * xs[p+"_"+e].Eval(m) * xs["htt"].Eval(m));
      });
    }
  }
  xs["hww_over_htt"] = ch::TGraphFromTable("input/xsecs_brs/hww_over_htt.txt", "mH", "ratio");
  for (string const& e : {"7TeV", "8TeV"}) {
    for (string const& p : {"ggH", "qqH"}) {
     cb.cp().channel({"em"}).process({p+"_hww125"}).era({e})
       .ForEachProc([&](ch::Process *proc) {
         proc->set_rate(proc->rate() * xs[p+"_"+e].Eval(125.) * xs["htt"].Eval(125.));
         proc->set_rate(proc->rate() * xs["hww_over_htt"].Eval(125.));
      });
    }
  }

  cout << ">> Merging bin errors and generating bbb uncertainties...\n";

  auto bbb = ch::BinByBinFactory()
      .SetAddThreshold(0.1)
      .SetMergeThreshold(0.5)
      .SetFixNorm(true);

  ch::CombineHarvester cb_et = cb.cp().channel({"et"});
  bbb.MergeAndAdd(cb_et.cp().era({"7TeV"}).bin_id({1, 2}).process({"ZL", "ZJ", "QCD", "W"}), cb);
  bbb.MergeAndAdd(cb_et.cp().era({"7TeV"}).bin_id({3, 5}).process({"W"}), cb);
  bbb.MergeAndAdd(cb_et.cp().era({"8TeV"}).bin_id({1, 2}).process({"ZL", "ZJ", "QCD", "W"}), cb);
  bbb.MergeAndAdd(cb_et.cp().era({"8TeV"}).bin_id({3, 5}).process({"W"}), cb);
  bbb.MergeAndAdd(cb_et.cp().era({"7TeV"}).bin_id({6}).process({"ZL", "ZJ", "W", "ZTT"}), cb);
  bbb.MergeAndAdd(cb_et.cp().era({"8TeV"}).bin_id({6}).process({"ZL", "ZJ", "W"}), cb);
  bbb.MergeAndAdd(cb_et.cp().era({"8TeV"}).bin_id({7}).process({"ZL", "ZJ", "W", "ZTT"}), cb);

  ch::CombineHarvester cb_mt = cb.cp().channel({"mt"});
  bbb.MergeAndAdd(cb_mt.cp().era({"7TeV"}).bin_id({1, 2, 3, 4}).process({"W", "QCD"}), cb);
  bbb.MergeAndAdd(cb_mt.cp().era({"8TeV"}).bin_id({1, 2, 3, 4}).process({"W", "QCD"}), cb);
  bbb.MergeAndAdd(cb_mt.cp().era({"7TeV"}).bin_id({5}).process({"W"}), cb);
  bbb.MergeAndAdd(cb_mt.cp().era({"7TeV"}).bin_id({6}).process({"W", "ZTT"}), cb);
  bbb.MergeAndAdd(cb_mt.cp().era({"8TeV"}).bin_id({5, 6}).process({"W"}), cb);
  bbb.MergeAndAdd(cb_mt.cp().era({"8TeV"}).bin_id({7}).process({"W", "ZTT"}), cb);

  ch::CombineHarvester cb_em = cb.cp().channel({"em"});
  bbb.MergeAndAdd(cb_em.cp().era({"7TeV"}).bin_id({1, 3}).process({"Fakes"}), cb);
  bbb.MergeAndAdd(cb_em.cp().era({"8TeV"}).bin_id({1, 3}).process({"Fakes"}), cb);
  bbb.MergeAndAdd(cb_em.cp().era({"7TeV"}).bin_id({4}).process({"Fakes", "EWK", "Ztt"}), cb);
  bbb.MergeAndAdd(cb_em.cp().era({"8TeV"}).bin_id({5}).process({"Fakes", "EWK", "Ztt"}), cb);
  bbb.MergeAndAdd(cb_em.cp().era({"8TeV"}).bin_id({4}).process({"Fakes", "EWK"}), cb);

  ch::CombineHarvester cb_tt = cb.cp().channel({"tt"});
  bbb.MergeAndAdd(cb_tt.cp().era({"8TeV"}).bin_id({0, 1, 2}).process({"ZTT", "QCD"}), cb);

  bbb.SetAddThreshold(0.);  // ee and mm use a different threshold
  ch::CombineHarvester cb_ll = cb.cp().channel({"ee", "mm"});
  bbb.MergeAndAdd(cb_ll.cp().era({"7TeV"}).bin_id({1, 3, 4}).process({"ZTT", "ZEE", "ZMM", "TTJ"}), cb);
  bbb.MergeAndAdd(cb_ll.cp().era({"8TeV"}).bin_id({1, 3, 4}).process({"ZTT", "ZEE", "ZMM", "TTJ"}), cb);

  cout << ">> Setting standardised bin names...\n";
  ch::SetStandardBinNames(cb);
  VString droplist = ch::ParseFileLines(
    aux_pruning + "uncertainty-pruning-drop-131128-sm.txt");
  cout << ">> Droplist contains " << droplist.size() << " entries\n";

  set<string> to_drop;
  for (auto x : droplist) to_drop.insert(x);

  auto pre_drop = cb.syst_name_set();
  cb.syst_name(droplist, false);
  auto post_drop = cb.syst_name_set();
  cout << ">> Systematics dropped: " << pre_drop.size() - post_drop.size()
            << "\n";

  // The following is an example of duplicating existing objects and modifying
  // them in the process. Here we clone all mH=125 signals, adding "_SM125" to
  // the process name, switching it to background and giving it the generic mass
  // label. This would let us create a datacard for doing a second Higgs search

  // ch::CloneProcsAndSysts(cb.cp().signals().mass({"125"}), cb,
  //                        [](ch::Object* p) {
  //   p->set_process(p->process() + "_SM125");
  //   p->set_signal(false);
  //   p->set_mass("*");
  // });

  string folder = "output/sm_cards/LIMITS";
  boost::filesystem::create_directories(folder);
  boost::filesystem::create_directories(folder + "/common");
  for (auto m : masses) {
    boost::filesystem::create_directories(folder + "/" + m);
  }

  for (string chn : chns) {
    TFile output((folder + "/common/htt_" + chn + ".input.root").c_str(),
                 "RECREATE");
    auto bins = cb.cp().channel({chn}).bin_set();
    for (auto b : bins) {
      for (auto m : masses) {
        cout << ">> Writing datacard for bin: " << b << " and mass: " << m
                  << "\r" << flush;
        cb.cp().channel({chn}).bin({b}).mass({m, "*"}).WriteDatacard(
            folder + "/" + m + "/" + b + ".txt", output);
      }
    }
    output.Close();
  }

  /*
  Alternatively use the ch::CardWriter class to automate the datacard writing.
  This makes it simple to re-produce the LIMITS directory format employed during
  the Run I analyses.
  Uncomment the code below to test:
  */

  /*
  // Here we define a CardWriter with a template for how the text datacard
  // and the root files should be named.
  ch::CardWriter writer("$TAG/$MASS/$ANALYSIS_$CHANNEL_$BINID_$ERA.txt",
                        "$TAG/common/$ANALYSIS_$CHANNEL.input_$ERA.root");
  writer.SetVerbosity(1);
  writer.WriteCards("output/sm_cards/LIMITS/cmb", cb);
  for (auto chn : cb.channel_set()) {
    writer.WriteCards("output/sm_cards/LIMITS/" + chn, cb.cp().channel({chn}));
  }
  */
  cout << "\n>> Done!\n";
}
