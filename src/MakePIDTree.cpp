#include <iostream>
#include <string>
#include "TArtStoreManager.hh"
#include "TArtEventStore.hh"
#include "TArtBigRIPSParameters.hh"
#include "TArtFocalPlane.hh"
#include "TArtDALIParameters.hh"
#include "TArtCalibPID.hh"
#include "TArtCalibDALI.hh"
#include "TArtCalibPPAC.hh"
#include "TArtCalibPlastic.hh"
#include "TArtEventInfo.hh"
#include "TArtPlastic.hh"
#include "TArtPPAC.hh"
#include "TArtRecoPID.hh"
#include "TArtRecoRIPS.hh"
#include "TArtRecoTOF.hh"
#include "TArtRecoBeam.hh"
#include "TArtBeam.hh"
#include "TArtTOF.hh"

#include "TSystem.h"
#include "TTree.h"
#include "TFile.h"

#include "TClonesArray.h"

#include "TVector3.h"

#include "signal.h"

#include "BigRIPSTreeData.hpp"

using namespace std;

/** prints usage **/
void usage(char *argv0)
{
    std::cout << "[MakePIDTree]: Usage: "
              << argv0 << "-i [input_file] -o [output_file]"
              << std::endl;
}

int main(int argc, char **argv)
{
    std::string input_file_name;
    std::string output_file_name("output.root");

    /** parsing commandline arguments **/
    int opt = 0;
    while ((opt = getopt(argc, argv, "o:i:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            output_file_name = optarg;
            break;
        case 'i':
            input_file_name = optarg;
            break;
        default:
            usage(argv[0]);
            return 1;
            break;
        }
    }

    if (input_file_name.empty())
    {
        usage(argv[0]);
        return 1;
    }

    TreeData treeData;
    //  signal(SIGINT,stop_interrupt); // CTRL + C , interrupt

    // Create StoreManager both for calibration "TArtCalib..." and treatment "TArtReco..."
    //------------------------------------------------------------------------------------
    TArtStoreManager *sman = TArtStoreManager::Instance();

    // Create EventStore to control the loop and get the EventInfo
    //------------------------------------------------------------
    TArtEventStore *estore = new TArtEventStore();
    // estore->SetInterrupt(&stoploop);
    estore->Open(input_file_name.c_str());
    std::cout << "estore ->" << input_file_name << std::endl;

    // Create BigRIPSParameters to get Plastics, PPACs, ICs and FocalPlanes parameters from ".xml" files
    //--------------------------------------------------------------------------------------------------
    TArtBigRIPSParameters *para = TArtBigRIPSParameters::Instance();
    para->LoadParameter("db/BigRIPSPPAC.xml");
    para->LoadParameter("db/BigRIPSPlastic.xml");
    para->LoadParameter("db/BigRIPSIC.xml");
    para->LoadParameter("db/FocalPlane.xml");
    para->SetFocusPosOffset(8, 231);

    // Create CalibPID to get and calibrate raw data ( CalibPID ->
    //[CalibPPAC , CalibIC, CalibPlastic , CalibFocalPlane]
    TArtCalibPID *brcalib = new TArtCalibPID();
    TArtCalibPPAC *ppaccalib = brcalib->GetCalibPPAC();
    TArtCalibPlastic *plasticcalib = brcalib->GetCalibPlastic();

    // Create RecoPID to get calibrated data and to reconstruct TOF, AoQ, Z, ... (RecoPID ->
    //[ RecoTOF , RecoRIPS , RecoBeam] )
    TArtRecoPID *recopid = new TArtRecoPID();

    para->PrintListOfPPACPara();
    // return;

    // Definition of observables we want to reconstruct
    std::cout << "Defining bigrips parameters" << std::endl;

    // modified by Kasia, 2015-04-06
    TArtRIPS *rips3to5 = recopid->DefineNewRIPS(3, 5, "matrix/mat1.mat", "D3");                    // F3 - F5
    TArtRIPS *rips5to7 = recopid->DefineNewRIPS(5, 7, "matrix/mat2.mat", "D5");                    // F5 - F7
    TArtRIPS *rips8to10 = recopid->DefineNewRIPS(8, 10, "matrix/F8F10_LargeAccAchr.mat", "D7");    // F8 - F10
    TArtRIPS *rips10to11 = recopid->DefineNewRIPS(10, 11, "matrix/F10F11_LargeAccAchr.mat", "D8"); // F10 - F11

    // Reconstruction of TOF DefineNewTOF(fisrt plane, second plane, time offset)
    TArtTOF *tof3to7 = recopid->DefineNewTOF("F3pl", "F7pl", 234.7, 5); // F3 - F7
    // TArtTOF *tof8to11 = recopid->DefineNewTOF("F8pl","F11pl-1",199.4,10); // F8 - F11
    TArtTOF *tof8to11 = recopid->DefineNewTOF("F8pl", "F11long", 199.4, 10); // F8 - F11

    // Reconstruction of IC observables for ID
    TArtBeam *beam_br_35 = recopid->DefineNewBeam(rips3to5, tof3to7, "F7IC");
    TArtBeam *beam_br_57 = recopid->DefineNewBeam(rips5to7, tof3to7, "F7IC");
    TArtBeam *beam_br_37 = recopid->DefineNewBeam(rips3to5, rips5to7, tof3to7, "F7IC");
    TArtBeam *beam_zd_810 = recopid->DefineNewBeam(rips8to10, tof8to11, "F11IC");
    TArtBeam *beam_zd_1011 = recopid->DefineNewBeam(rips10to11, tof8to11, "F11IC");
    TArtBeam *beam_zd_811 = recopid->DefineNewBeam(rips8to10, rips10to11, tof8to11, "F11IC");

    // to get trigger pattern
    TArtEventInfo *evtinfo = new TArtEventInfo();
    int trg = -777;
    cout << "trigger " << trg << endl;

    TFile *fout = new TFile(output_file_name.c_str(), "RECREATE");
    TTree *tree = new TTree("tree", "tree");

    // define data nodes which are supposed to be dumped to tree
    tree->Branch("pid", "TreeData", &treeData);

    TClonesArray *info_array = (TClonesArray *)sman->FindDataContainer("EventInfo");
    auto info = (TArtEventInfo *)info_array->At(0);

    TClonesArray *fpl_array =
        (TClonesArray *)sman->FindDataContainer("BigRIPSFocalPlane");
    auto f11 = (TArtFocalPlane *)fpl_array->FindObject("fpl11");
    auto f5 = (TArtFocalPlane *)fpl_array->FindObject("fpl5");

    TClonesArray *tof_array =
        (TClonesArray *)sman->FindDataContainer("BigRIPSTOF");
    auto tof37 = (TArtTOF *)tof_array->At(0);

    TClonesArray *beam_array =
        (TClonesArray *)sman->FindDataContainer("BigRIPSBeam");
    auto beam37 = (TArtBeam *)beam_array->At(2);

    int neve = 0;
    //  while(estore->GetNextEvent()&& neve<1000){
    while (estore->GetNextEvent())
    {
        if (neve % 1000 == 0)
            std::cout << "event: " << neve << std::endl;

        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        // Making the BigRIPS tree calibration
        brcalib->ClearData();
        brcalib->ReconstructData();
        // Reconstructiong the PID
        recopid->ClearData();
        recopid->ReconstructData();

        treeData.tof = tof37->GetTOF();
        treeData.zet = beam37->GetZet();
        treeData.aoq = beam37->GetAoQ();
        treeData.f5x = f5->GetX();
        treeData.f11x = f11->GetX();
        treeData.f11y = f11->GetY();
        treeData.f11dt = 0;
        treeData.beta = 0;
        treeData.ts = info->GetTimeStamp();
        treeData.sts = 0;
        treeData.EventId = info->GetEventNumber();
        treeData.RunId = info->GetRunNumber();

        tree->Fill();
        neve++;
    }
    cout << "Writing the tree." << endl;

    fout->Write();
    fout->Close();
    return 0;
}
