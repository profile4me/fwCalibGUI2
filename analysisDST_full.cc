
// base
#include "hades.h"
#include "hruntimedb.h"
#include "htask.h"
#include "hevent.h"
#include "hcategory.h"
#include "hdatasource.h"
#include "hdst.h"
#include "htime.h"
#include "hsrckeeper.h"


// tasksets
#include "hstarttaskset.h"
#include "hrich700taskset.h"
#include "hmdctaskset.h"
#include "htoftaskset.h"
#include "hrpctaskset.h"
#include "hemctaskset.h"
#include "hwalltaskset.h"
#include "hsplinetaskset.h"


//tasks
#include "hmdcdedx2maker.h"
#include "hmdctrackdset.h"
#include "hmdc12fit.h"
#include "hmdccalibrater1.h"
#include "hmetamatchF2.h"
#include "hparticlevertexfind.h"
#include "hparticlecandfiller.h"
#include "hparticletrackcleaner.h"
#include "hparticleevtinfofiller.h"
#include "hparticlestart2hitf.h"
#include "hparticlebt.h"
#include "hparticlet0reco.h"
#include "hstart2calibrater.h"
#include "hqamaker.h"
#include "hemcclusterf.h"

// defs
#include "haddef.h"
#include "richdef.h"
#include "hmdcdef.h"
#include "hmdctrackddef.h"
#include "hmdctrackgdef.h"
#include "rpcdef.h"
#include "tofdef.h"
#include "walldef.h"


// containers
#include "hmdcsetup.h"
#include "hmdclayercorrpar.h"
#include "htrbnetaddressmapping.h"
#include "hmagnetpar.h"
#include "hemcgeompar.h"
#include "hparasciifileio.h"
#include "hwalleventplanepar.h"


// ROOT
#include "TSystem.h"
#include "TROOT.h"
#include "TString.h"
#include "TStopwatch.h"
#include "TDatime.h"

// standard
#include <iostream>
#include <cstdio>

using namespace std;


// macros
#include "treeFilter.h"
#include "HMoveRichSector.h"

// Int_t analysisDST(TString inFile, TString outdir,Int_t nEvents=1, Int_t startEvt=0)
int main(int argc, char const *argv[])
{
    Int_t startEvt=0;
    TString inFile, outFile, events="", asciiParFile="";
    Int_t nEvents;
    switch (argc) {
        case 3: {
            inFile = argv[1];
            outFile = argv[2];
            break;
        }
        case 4: {
            inFile = argv[1];
            outFile = argv[2];
            events=argv[3];
            break;
        }
        case 5: {
          inFile = argv[1];
          outFile = argv[2];
          events=argv[3];
          asciiParFile=argv[4];      
        }
    }
    nEvents=events.Atoi();

    new Hades;
    gHades->setEmbeddingMode(0);
    TStopwatch timer;
    gHades->setTreeBufferSize(8000);
    gHades->makeCounter(1000);
    HRuntimeDb *rtdb = gHades -> getRuntimeDb();
    gHades->setBeamTimeID(Particle::kMar19);
    gHades->getSrcKeeper()->addSourceFile("analysisDST.cc");
    gHades->getSrcKeeper()->addSourceFile("treeFilter.h");
    gHades->getSrcKeeper()->addSourceFile("HMoveRichSector.h");
    // gHades->getSrcKeeper()->addSourceFile("sendScript_SL.sh");


    //####################################################################
    //######################## CONFIGURATION #############################
    printf("Setting configuration...+++\n");

    // TString asciiParFile     = "";
    //TString rootParFile      = "./params/allParam_Mar19_gen0_29052019.root";  // gen0
    //TString rootParFile      = "./params/allParam_Mar19_gen0_06062019.root";  // gen0
    TString rootParFile      = "/u/hadesdst/GE/mar19/scripts/dstreal/params/allParam_Mar19_gen0_16082019.root";  // gen0
    //TString paramSource      = "oracle"; // root, ascii, oracle
    TString paramSource      = !asciiParFile.CompareTo("") ? "root" : "ascii,root"; // root, ascii, oracle


    TString beamtime         = "mar19";
    //TString paramrelease     = "MAR19_v1a";  // now,
    TString paramrelease     = "MAR19_v1b";  // now,
    //TString paramrelease     = "now";  // now,

    Int_t  refId             = -1; //  first run mar19
    Bool_t kParamFile        = kFALSE;
    Bool_t doExtendedFit     = kTRUE; // switch on/off fit for initial params of segment fitter (10 x slower!)
    Bool_t doStartCorrection = kTRUE;  // kTRUE (default)=  use run by run start corrections
    Bool_t doRotateRich      = kTRUE;
    Bool_t doMetaMatch       = kFALSE;  // default : kTRUE, kFALSE switch off metamatch in clusterfinder
    Bool_t useOffVertex      = kTRUE;  // default : kTRUE,  kTRUE=use off vertex procedure  (apr12 gen8:kFALSE, gen9:kTRUE)
    Bool_t doMetaMatchScale  = kTRUE;
    Bool_t useWireStat       = kFALSE;
    Float_t metaScale        = 2; // apr12: 1.5

    //####################################################################
    //####################################################################




    //-------------- Default Settings for the File names -----------------
    TString outdir="mock";
    TString baseDir = outdir;
    if(!baseDir.EndsWith("/")) baseDir+="/";

    TString fileWoPath = gSystem->BaseName(inFile.Data());
    fileWoPath.ReplaceAll(".hld","");
    Int_t day      = HTime::getDayFileName(fileWoPath,kFALSE);
    Int_t evtBuild = HTime::getEvtBuilderFileName(fileWoPath,kFALSE);

    //TString outDir   = Form("%s%i/",baseDir.Data(),day);
    TString outDir   = Form("%s",baseDir.Data());
    TString outDirQA = outDir+"qa/";

    if(gSystem->AccessPathName(outDir.Data()) != 0){
	cout<<"Creating output dir :"<<outDir.Data()<<endl;
	gSystem->Exec(Form("mkdir -p %s",outDir.Data()));
    }
    if(gSystem->AccessPathName(outDirQA.Data()) != 0){
	cout<<"Creating QA output dir :"<<outDirQA.Data()<<endl;
	gSystem->Exec(Form("mkdir -p %s",outDirQA.Data()));
    }

    TString hldSuffix =".hld";
    TString hldFile   = gSystem->BaseName(inFile.Data());
    if (hldFile.EndsWith(hldSuffix)) hldFile.ReplaceAll(hldSuffix,"");

    TString hld      = hldFile+hldSuffix;
    outFile.ReplaceAll("//", "/");
    //--------------------------------------------------------------------

    //------ extended output for eventbuilder 1------------------------------------//
    //----------------------------------------------------------------------------//
    Cat_t notPersistentCatAll[] =
    {
	//catRich, catMdc, catShower, catTof, catTracks,
	catRich700Raw,
        //catRichDirClus, catRichHitHdr, 
        //catRichHit, catRichCal,
	catMdcRaw, catMdcCal1,catMdcCal2,catMdcClusInf,catMdcHit,
	catMdcSeg,
	catMdcTrkCand,catMdcRawEventHeader,
	catEmcRaw, catEmcCalQA,
	//catEmcCal, catEmcCal,catEmcCluster,
	catTofRaw,
	//catTofHit, catTofCluster,
	catRpcRaw,
	//catRpcCal,
	//catRpcHit, catRpcCluster,
	catRKTrackB, catSplineTrack,
	//catMetaMatch,
	//catParticleCandidate, catParticleEvtInfo, catParticleMdc,
	// catWallRaw,
     catWallOneHit,
	//catWallCal,
	catStart2Raw //catStart2Cal, catStart2Hit,
	// catTBoxChan
    };
    //--------------------------------------------------------------------

    //------standard output for dst production------------------------------------//
    //----------------------------------------------------------------------------//
    Cat_t notPersistentCat[] =
    {
	//catRich, catMdc, catShower, catTof, catTracks,
	catRich700Raw, //catRichDirClus,
	catRichHitHdr,
	//catRichHit, catRichCal,
	catMdcRaw, catMdcCal1,catMdcCal2,catMdcClusInf, catMdcHit,
	catMdcSeg, catMdcTrkCand, catMdcRawEventHeader,
	catEmcRaw,  catEmcCalQA,
        //catEmcCal, catEmcCluster,
        catTofRaw, catTofHit, catTofCluster,
	catRpcRaw, catRpcCal, catRpcHit, catRpcCluster,
	catRKTrackB, catSplineTrack,
	catMetaMatch,
	//catParticleCandidate, catParticleEvtInfo, catParticleMdc,
	// catWallRaw, 
    catWallOneHit, catWallCal,
	catStart2Raw //catStart2Cal, catStart2Hit,
	// catTBoxChan
    };
    //--------------------------------------------------------------------

    //####################################################################
    //####################################################################



    Int_t mdcMods[6][4]=
    { {1,1,1,1},
    {1,1,1,1},
    {1,1,1,1},
    {1,1,1,1},
    {1,1,1,1},
    {1,1,1,1} };

    // recommendations from Vladimir+Olga
    // according to params from 28.04.2011
    Int_t nLayers[6][4] = {
	{6,6,5,6},
	{6,6,5,6},
	{6,6,5,6},
	{6,6,5,6},
	{6,6,5,6},
	{6,6,5,6} };
    Int_t nLevel[4] = {10,50000,10,5000};

    HDst::setupSpectrometer(beamtime,mdcMods,"rich,mdc,tof,rpc,emc,wall,start,tbox");
    // HDst::setupSpectrometer(beamtime,mdcMods,"wall");
    // beamtime mdcMods_apr12, mdcMods_full
    // Int_t mdcset[6][4] setup mdc. If not used put NULL (default).
    // if not NULL it will overwrite settings given by beamtime
    // detectors (default)= rich,mdc,tof,rpc,shower,wall,tbox,start

    printf("asciiParFile: %s\n",asciiParFile.Data());
    HDst::setupParameterSources(paramSource,asciiParFile,rootParFile,paramrelease);  // now, APR12_gen2_dst
    //HDst::setupParameterSources("oracle",asciiParFile,rootParFile,"now"); // use to create param file
    // parsource = oracle,ascii,root (order matters)
    // if source is "ascii" a ascii param file has to provided
    // if source is "root" a root param file has to provided
    // The histDate paramter (default "now") is used wit the oracle source

    HDst::setDataSource(0,"",inFile,refId); // Int_t sourceType,TString inDir,TString inFile,Int_t refId, TString eventbuilder"
    // datasource 0 = hld, 1 = hldgrep 2 = hldremote, 3 root
    // like "lxhadeb02.gsi.de"  needed by dataosoure = 2
    // inputDir needed by dataosoure = 1,2
    // inputFile needed by dataosoure = 1,3


    HDst::setupUnpackers(beamtime,"rich,mdc,tof,rpc,emc,tbox,wall,latch,start");
    // HDst::setupUnpackers(beamtime,"wall");
    // beamtime mar19
    // detectors (default)= rich,mdc,tof,rpc,emc,wall,tbox,latch,start

    if(kParamFile) {
        TDatime time;
        TString paramfilename= Form("allParam_Mar19_gen0_%02i%02i%i",time.GetDay(),time.GetMonth(),time.GetYear());  // without .root

	if(gSystem->AccessPathName(Form("%s.root",paramfilename.Data())) == 0){
	    gSystem->Exec(Form("rm -f %s.root",paramfilename.Data()));
	}
	if(gSystem->AccessPathName(Form("%s.log",paramfilename.Data())) == 0){
	    gSystem->Exec(Form("rm -f %s.log" ,paramfilename.Data()));
	}

	if (!rtdb->makeParamFile(Form("%s.root",paramfilename.Data()),beamtime.Data(),"26-FEB-2019","02-APR-2019")) {
	    delete gHades;
	    exit(1);
	}
    }
    if(refId<0){
	refId = gHades->getDataSource()->getCurrentRunId();
    }
    //--------------------------------------------------------------------
    // ----------- Build TASK SETS (using H***TaskSet::make) -------------
    HStartTaskSet        *startTaskSet        = new HStartTaskSet();
    HRich700TaskSet      *richTaskSet         = new HRich700TaskSet();
    HRpcTaskSet          *rpcTaskSet          = new HRpcTaskSet();
    HEmcTaskSet          *emcTaskSet          = new HEmcTaskSet();

    HTofTaskSet          *tofTaskSet          = new HTofTaskSet();
    HWallTaskSet         *wallTaskSet         = new HWallTaskSet();
    HMdcTaskSet          *mdcTaskSet          = new HMdcTaskSet();
    //    mdcTaskSet->setVersionDeDx(1); // 0 = no dEdx, 1 = HMdcDeDx2


    HMdcSetup* mysetup = (HMdcSetup*)rtdb->getContainer("MdcSetup");
    HEmcGeomPar* emcgeompar= (HEmcGeomPar*)rtdb->getContainer("EmcGeomPar");

    HTrbnetAddressMapping* trbnetmap = (HTrbnetAddressMapping*)rtdb->getContainer("TrbnetAddressMapping");
   


    rtdb->initContainers(refId);

    //mysetup->setStatic();
    trbnetmap->setStatic();
    emcgeompar->setStatic();


    //mysetup->getMdcCommonSet()->setIsSimulation(0);                 // sim=1, real =0
    //mysetup->getMdcCommonSet()->setAnalysisLevel(4);                // fit=4
    //mysetup->getMdcCalibrater1Set()->setMdcCalibrater1Set(2, 1);    // 1 = NoStartandCal, 2 = StartandCal, 3 = NoStartandNoCal ::  0 = noTimeCut, 1 = TimeCut
    //mysetup->getMdcCalibrater1Set()->setMdcCalibrater1Set(2, 0);    // 1 = NoStartandCal, 2 = StartandCal, 3 = NoStartandNoCal ::  0 = noTimeCut, 1 = TimeCut
    //mysetup->getMdcTrackFinderSet()->setIsCoilOff(kFALSE);          // field is on
    //mysetup->getMdcTrackFinderSet()->setNLayers(nLayers[0]);
    //mysetup->getMdcTrackFinderSet()->setNLevel(nLevel);
    //mysetup->getMdc12FitSet()->setMdc12FitSet(2,1,0,kFALSE,kFALSE); // tuned fitter, seg


    HTask *startTasks         = startTaskSet       ->make("real","");
    HTask *richTasks          = richTaskSet        ->make("real","");
    HTask *tofTasks           = tofTaskSet         ->make("real","");
    HTask *wallTasks          = wallTaskSet        ->make("real");
    HTask *rpcTasks           = rpcTaskSet         ->make("real");
    HTask *emcTasks           = emcTaskSet         ->make("real","");
    /*
    // online dsts mar19
    HEmcClusterF* clFinder = (HEmcClusterF*) emcTasks->getTask("emc.clusf");

    clFinder->setRpcMatchingParDThDPh(5);
    clFinder->setRpcMatchingParDTheta(0,5);
    clFinder->setRpcMatchingParDTime(0,10);
    clFinder->setClFnParamDTimeWindow(-5,5);
    */

    // this should in future go to params container
    HEmcClusterF* clFinder = (HEmcClusterF*) emcTasks->getTask("emc.clusf");
    clFinder->setRpcMatchingParDThDPh(5);
    clFinder->setRpcMatchingParDTheta(0,2); //5);
    clFinder->setRpcMatchParDTimeInNs(0,5); //,10);
    clFinder->setClFnParamDTimeWindow(-5,5);
    clFinder->setCellEnergyCut(10.);


    HTask *mdcTasks           = mdcTaskSet         ->make("rtdb","");


    //----------------SPLINE and RUNGE TACKING----------------------------------------
    HSplineTaskSet         *splineTaskSet       = new HSplineTaskSet("","");
    HTask *splineTasks     = splineTaskSet      ->make("","spline,runge");

    HParticleStart2HitF    *pParticleStart2HitF = new HParticleStart2HitF   ("particlehitf"      ,"particlehitf","");
    HParticleCandFiller    *pParticleCandFiller = new HParticleCandFiller   ("particlecandfiller","particlecandfiller");
									     //,"NOMOMENTUMCORR,NOPATHLENGTHCORR");
									     //,"NOMETAQANORM,NOMOMENTUMCORR,NOPATHLENGTHCORR");  // online dst
    HParticleTrackCleaner  *pParticleCleaner    = new HParticleTrackCleaner ("particlecleaner"   ,"particlecleaner");
    HParticleVertexFind    *pParticleVertexFind = new HParticleVertexFind   ("particlevertexfind","particlevertexfind","");
    HParticleEvtInfoFiller *pParticleEvtInfo    = new HParticleEvtInfoFiller("particleevtinfo"   ,"particleevtinfo",beamtime);


    //----------------------- Quality Assessment -------------------------
    HQAMaker *qaMaker =0;
    if (!outDirQA.IsNull())
    {
	qaMaker = new HQAMaker("qamaker","qamaker");
	qaMaker->setOutputDir((Text_t *)outDirQA.Data());
	//qaMaker->setPSFileName((Text_t *)hldFile.Data());
	qaMaker->setUseSlowPar(kFALSE);
	//qaMaker->setUseSlowPar(kTRUE);
	qaMaker->setSamplingRate(1);
	qaMaker->setIntervalSize(50);
    }



    //------------------------ Master task set ---------------------------
    HTaskSet *masterTaskSet = gHades->getTaskSet("all");

    masterTaskSet->add(startTasks);

    masterTaskSet->add(tofTasks);
    masterTaskSet->add(rpcTasks);
    masterTaskSet->add(pParticleStart2HitF); // run before wall,mdc,sline,candfiller
    masterTaskSet->add(richTasks);
    if(doRotateRich)masterTaskSet->add(new HMoveRichSector("moveRichSector","moveRichSector"));
   
    masterTaskSet->add(wallTasks);
   
    masterTaskSet->add(emcTasks);

    masterTaskSet->add(mdcTasks);

    masterTaskSet->add(splineTasks);
    masterTaskSet->add(pParticleCandFiller);
    masterTaskSet->add(pParticleCleaner);
    masterTaskSet->add(pParticleVertexFind); // run after track cleaning
    masterTaskSet->add(pParticleEvtInfo);

    //masterTaskSet->add(new HParticleT0Reco("T0","T0",beamtime));

    //addFilter(masterTaskSet,inFile,outDir) ;  // treeFilter.h

    // if (qaMaker) masterTaskSet->add(qaMaker);

    //--------------------------------------------------------------------
    //  special settings
    HMdcTrackDSet::setTrackParam(beamtime);
    //HMdcTrackDSet::setFindOffVertTrkFlag(useOffVertex); //???


    if(!doMetaMatch)    HMdcTrackDSet::  setMetaMatchFlag(kFALSE,kFALSE);  //do not user meta match in clusterfinder
    if(doMetaMatchScale)HMetaMatchF2::   setScaleCut(metaScale,metaScale,metaScale); // (tof,rpc,shower) increase matching window, but do not change normalization of MetaQA
    if(useWireStat)     HMdcCalibrater1::setUseWireStat(kTRUE);
    
    //mdcTaskSet->getCalibrater1()->setUseMultCut(500,kTRUE,kTRUE,kTRUE); // nwires, docut, useTot, skip
    //mdcTaskSet->getCalibrater1()->setDoPrint(kTRUE);


    //--------------------------------------------------------------------
    // find best initial params for segment fit (takes long!)
    if(doExtendedFit) {
	HMdcTrackDSet::setCalcInitialValue(1);  // -  1 : for all clusters 2 : for not fitted clusters
    }
    //--------------------------------------------------------------------

    //HMdcDeDx2Maker::setFillCase(2);                      // 0 =combined, 1=combined+seg, 2=combined+seg+mod (default)
    HStart2Calibrater::setCorrection(doStartCorrection); // kTRUE (default)=  use
    //--------------------------------------------------------------------

    if (!gHades->init()){
	   Error("init()","Hades did not initialize ... once again");
	   exit(1);
    }

    //REINITIALIZATION OF WallEventPlanePar from txt
    // if (asciiParFile.CompareTo("")) {
        // HParAsciiFileIo txtInputer;
        // txtInputer.open("../cleanTest/recalib_params/EPparams_recalib_day63_hour0.txt","in");
        // HParSet *epPars = rtdb->getContainer("WallEventPlanePar");
        // epPars->init(&txtInputer);
    // }
    /////////////////////////////////////////////////
    printf ("==========> XSHIFT after gHades->init(): %.2f <=================\n", ( (HWallEventPlanePar*)rtdb->getContainer("WallEventPlanePar") )->getXShift() );
    printf ("==========> YSHIFT after gHades->init(): %.2f <=================\n", ( (HWallEventPlanePar*)rtdb->getContainer("WallEventPlanePar") )->getYShift() );


    //--------------------------------------------------------------------
    //----------------- Set not persistent categories --------------------
    HEvent *event = gHades->getCurrentEvent();

    HCategory *cat = ((HCategory *)event->getCategory(catRichCal));
    if(cat) cat->setPersistency(kTRUE);


    if(evtBuild == 1) {
	for(UInt_t i=0;i<sizeof(notPersistentCatAll)/sizeof(Cat_t);i++){
	    cat = ((HCategory *)event->getCategory(notPersistentCatAll[i]));
	    if(cat)cat->setPersistency(kFALSE);
	}
    } else {
	for(UInt_t i=0;i<sizeof(notPersistentCat)/sizeof(Cat_t);i++){
	    cat = ((HCategory *)event->getCategory(notPersistentCat[i]));
	    if(cat)cat->setPersistency(kFALSE);
	}
    }
    //--------------------------------------------------------------------

    // output file
    gHades->setOutputFile((Text_t*)outFile.Data(),"RECREATE","Test",2);
    gHades->makeTree();

    gHades->eventLoop(1,startEvt);
    printf ("==========> XSHIFT after gHades->eventLoop(): %.2f <=================\n", ( (HWallEventPlanePar*)rtdb->getContainer("WallEventPlanePar") )->getXShift() );
    printf ("==========> YSHIFT after gHades->eventLoop(): %.2f <=================\n", ( (HWallEventPlanePar*)rtdb->getContainer("WallEventPlanePar") )->getYShift() );
    Int_t nProcessed = gHades->eventLoop(nEvents-1,startEvt);
    printf("Events processed: %i\n",nProcessed);

    cout<<"--Input file      : "<<inFile  <<endl;
    cout<<"--QA directory is : "<<outDirQA<<endl;
    cout<<"--Output file is  : "<<outFile <<endl;

    printf("Real time: %f\n",timer.RealTime());
    printf("Cpu time: %f\n",timer.CpuTime());
    if (nProcessed) printf("Performance: %f s/ev\n",timer.CpuTime()/nProcessed);

    if(kParamFile) rtdb->saveOutput();

    delete gHades;
    timer.Stop();

    return 0;

}
/*
#ifndef __CINT__
int main(int argc, char **argv)
{
    TROOT AnalysisDST("AnalysisDST","compiled analysisDST macros");


    TString nevents,startevent;
    switch (argc)
    {
    case 3:
	return analysisDST(TString(argv[1]),TString(argv[2])); // inputfile + outdir
	break;
    case 4:  // inputfile + outdir + nevents
	nevents=argv[3];

	return analysisDST(TString(argv[1]),TString(argv[2]),nevents.Atoi());
	break;
	// inputfile + nevents + startevent
    case 5: // inputfile + outdir + nevents + startevent
	nevents   =argv[3];
	startevent=argv[4];
	return analysisDST(TString(argv[1]),TString(argv[2]),nevents.Atoi(),startevent.Atoi());
	break;
    default:
	cout<<"usage : "<<argv[0]<<" inputfile outputdir [nevents] [startevent]"<<endl;
	return 0;
    }
}
#endif

*/