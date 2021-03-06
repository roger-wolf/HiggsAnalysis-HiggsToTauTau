#include "CombineTools/interface/TFileIO.h"
#include <memory>
#include <string>
#include <vector>
#include "TFile.h"
#include "TH1.h"
#include "TDirectory.h"
#include "CombineTools/interface/Logging.h"

namespace ch {

std::unique_ptr<TH1> GetClonedTH1(TFile* file, std::string const& path) {
  if (!file) {
    throw std::runtime_error(FNERROR("Supplied ROOT file pointer is null"));
  }
  TDirectory* backup_dir = gDirectory;
  file->cd();
  if (!gDirectory->Get(path.c_str())) {
    gDirectory = backup_dir;
    throw std::runtime_error(
        FNERROR("TH1 " + path + " not found in " + file->GetName()));
  }
  std::unique_ptr<TH1> res(
      dynamic_cast<TH1*>(gDirectory->Get(path.c_str())->Clone()));
  if (!res) {
    gDirectory = backup_dir;
    throw std::runtime_error(FNERROR("Object " + path + " in " +
                                     file->GetName() + " is not of type TH1"));
  }
  res->SetDirectory(0);
  gDirectory = backup_dir;
  return res;
}
}
