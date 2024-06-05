//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Jorge Agramunt Ros    @ IFIC(Valencia,Spain)  jorge.agramunt@ific.uv.es
// Alvaro Tolosa Delgado @ IFIC(Valencia,Spain)  alvaro.tolosa@ific.uv.es
// Copyright (c) 2016 Jorge Agramunt & Alvaro Tolosa. All rights reserved.

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BIGRIPSTREEDATA_H_
#define BIGRIPSTREEDATA_H_
#include <string>
#include <TTree.h>

class TreeData
{
public:
    TreeData() { clear(); }
    TreeData(const TreeData &tree_data)
    {
        tof = tree_data.tof;
        zet = tree_data.zet;
        aoq = tree_data.aoq;
        f5x = tree_data.f5x;
        f11x = tree_data.f11x;
        f11y = tree_data.f11y;
        f11dt = tree_data.f11dt;
        beta = tree_data.beta;
        ts = tree_data.ts;
        sts = tree_data.sts;
        EventId = tree_data.EventId;
        RunId = tree_data.RunId;
    }
    virtual ~TreeData(){};

    Long64_t ts;  // Calibrated time
    Long64_t sts; // Calibrated time
    Double_t tof;
    Double_t zet; // for TOF: f3-f5, f5-f7, f3-f7,  f8-f9, f9-f11, f8-f11
    Double_t aoq;
    Double_t f5x;
    Double_t f11x;
    Double_t f11y;
    Double_t f11dt;
    Double_t beta;
    ULong64_t EventId;
    ULong64_t RunId;

    void clear()
    {
        tof = 0;
        zet = 0;
        aoq = 0;
        f5x = 0;
        f11x = 0;
        f11y = 0;
        f11dt = 0;
        beta = 0;
        ts = 0;
        sts = 0;
        EventId = 0;
        RunId = 0;
    }
};

#endif