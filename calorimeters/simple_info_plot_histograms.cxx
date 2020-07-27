R__LOAD_LIBRARY($HOME/stow/development/lib/libGenDetectors.so)
R__LOAD_LIBRARY(libfmt.so)
#include "fmt/core.h"
R__LOAD_LIBRARY(libDDG4IO.so)
#include "DD4hep/Detector.h"
#include "DDG4/Geant4Data.h"
#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DD4hep/VolumeManager.h"
#include "DD4hep/detail/Handle.inl"
#include "DD4hep/detail/ObjectsInterna.h"
#include "DD4hep/detail/DetectorInterna.h"
#include "DD4hep/detail/VolumeManagerInterna.h"
#include "DDRec/Surface.h"
#include "DD4hep/Volumes.h"
#include "DD4hep/DetElement.h"
#include "DD4hep/NamedObject.h"
#include "DD4hep/IDDescriptor.h"
#include "DD4hep/ConditionsMap.h"
#include "TGeoMatrix.h"
#include "ROOT/RDataFrame.hxx"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TTree.h"
#include "TChain.h"
#include <random>
#include <iostream>

void simple_info_plot_histograms(const char* fname = "sim_output/output_zdc_photons.root"){

  TChain* t = new TChain("EVENT");
  t->Add(fname);

  ROOT::RDataFrame d0(*t);//, {"ZDCHits","MCParticles"});

  dd4hep::Detector& detector = dd4hep::Detector::getInstance();
  detector.fromCompact("ZDC_example.xml");  

  //dd4hep::VolumeManager volman = detector.volumeManager();
  //volman.getVolumeManager(detector);
  dd4hep::VolumeManager volman = dd4hep::VolumeManager::getVolumeManager(detector);

  // Number of hits
  auto nhits = [] (std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>& hits){ return (int) hits.size(); };

  // Cell ID
  auto cellID = [&] (const std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>& hits) {
        std::vector<double> result;
        for(const auto& h: hits)
                result.push_back(h->cellID);
  return result;
  };

  // Volume ID
  auto volID = [&] (const std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>& hits) {
        std::vector<double> result;
        for(const auto& h: hits) {
		auto detelement = volman.lookupDetElement(h->cellID);
		result.push_back(detelement.volumeID());
	}
  return result;
  };

  // Hit position X
  auto hit_x_position = [&] (const std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>& hits) {
	std::vector<double> result;
	for(const auto& h: hits)
		result.push_back(h->position.x()); //mm
  return result; 
  };

  // Hit position Y
  auto hit_y_position = [&] (const std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>& hits) {
        std::vector<double> result;
        for(const auto& h: hits)
		result.push_back(h->position.y()); //mm
  return result;
  };

  // Hit position Z
  auto hit_z_position = [&] (const std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>& hits) {
        std::vector<double> result;
        for(const auto& h: hits)
		result.push_back(h->position.z()); //mm
  return result;
  };

  // Energy deposition
  auto e_dep = [&] (const std::vector<dd4hep::sim::Geant4Calorimeter::Hit*>& hits) {
        std::vector<double> result;
        for(const auto& h: hits)
		result.push_back(h->energyDeposit); //GeV
  return result;
  };

  auto d1 = d0.Define("nhits", nhits, {"ZDCHits"})
	      .Define("cellID", cellID, {"ZDCHits"})
	      .Define("volID", volID, {"ZDCHits"})
    	      .Define("hit_x_position", hit_x_position, {"ZDCHits"})
	      .Define("hit_y_position", hit_y_position, {"ZDCHits"})
              .Define("hit_z_position", hit_z_position, {"ZDCHits"})
	      .Define("e_dep", e_dep, {"ZDCHits"})
  	      ;

  // Define Histograms
  auto h0 = d1.Histo1D({"h0", "nhits histogram; nhits; Events", 100, 0,5000}, "nhits");
  auto h1 = d1.Histo1D({"h1", "hit_x_position histogram; hit X position [mm]; Events", 60,-30,30}, "hit_x_position");
  auto h2 = d1.Histo1D({"h2", "hit_y_position histogram; hit Y position [mm]; Events", 100,-30,80}, "hit_y_position");
  auto h3 = d1.Histo1D({"h3", "hit_z_position histogram; hit Z position [mm]; Events", 100,1000,1300}, "hit_z_position");
  auto h4 = d1.Histo1D({"h4", "energy deposition histogram; energy deposition [GeV]; Events", 100,0,300}, "e_dep");
  auto h5 = d1.Histo1D({"h5", "volume ID; volumeID; Events", 92,-0.5,92.5}, "volID");

  auto n0 = d1.Filter([](int n){ return (n>0); },{"nhits"}).Count();

  d1.Snapshot("info_EVENT","./calorimeters/info_zdc_photons.root");
  std::cout << *n0 << " events with nonzero hits\n";

  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  c1->SetLogy(1);
  h0->GetYaxis()->SetTitleOffset(1.4);
  h0->SetLineWidth(2);
  h0->SetLineColor(kBlack);
  h0->DrawClone();
  c1->SaveAs("./calorimeters/nhits_histo_zdc_photons.png");

  TCanvas *c2 = new TCanvas("c2","c2",1000,1000);
  c2->Divide(2,2);
  c2->cd(1);
  h1->GetYaxis()->SetTitleOffset(1.7);
  h1->SetLineWidth(2);
  h1->SetLineColor(kBlack);
  h1->DrawClone();

  c2->cd(2);
  h2->GetYaxis()->SetTitleOffset(1.7);
  h2->SetLineWidth(2);
  h2->SetLineColor(kBlack);  
  h2->DrawClone();

  c2->cd(3);
  h3->GetYaxis()->SetTitleOffset(1.7);
  h3->SetLineWidth(2);
  h3->SetLineColor(kBlack);  
  h3->DrawClone();
  c2->SaveAs("./calorimeters/hit_postion_histo_zdc_photons.png");

  TCanvas *c3 = new TCanvas("c3","c3",600,600);
  c3->cd();
  c3->SetLogy(1);
  h4->GetYaxis()->SetTitleOffset(1.4);
  h4->SetLineWidth(2);
  h4->SetLineColor(kBlack);
  h4->DrawClone();
  c3->SaveAs("./calorimeters/edep_histo_zdc_photons.png");

  TCanvas *c4 = new TCanvas("c4","c4",600,600);
  c4->SetLogy(0);
  h5->GetYaxis()->SetTitleOffset(2.0);
  h5->SetLineWidth(2);
  h5->SetLineColor(kBlack);
  h5->DrawClone();
  c4->SaveAs("./calorimeters/volID_histo_zdc_photons.png");

  if(*n0<5) {
    std::quick_exit(1);
  }
}

