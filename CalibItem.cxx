#include "CalibItem.h"
#include <stdio.h>
#include <algorithm>
#include <TChain.h>
#include <TFitResult.h>
#include <TSpectrum.h>
#include <TMath.h>
#include <TF1.h>
ClassImp(CalibItem);

using namespace std;

// fm_1    lm_1
//      fm_2    lm_2


const char *CalibItem::PROJ_FILE = "proj.root";
TFile *CalibItem::projFile = new TFile(PROJ_FILE,"update");
DayMeta *CalibItem::meta = NULL;
const int CalibItem::MOCK = CalibItem::initMOCK(); 
const char *CalibItem::HLD_ROOT =  DayMeta::HLD_ROOT;
const int CalibItem::HLDs_PER_ITEM  = 2;
TRandom *CalibItem::r =  new TRandom;
const char *CalibItem::TREES_FILE = "trees.root";
TFile *CalibItem::treesFile = new TFile(TREES_FILE);
const int CalibItem::NBINS = 200;
const float CalibItem::MIN_RAW_WIDTH = 0;
const float CalibItem::MAX_RAW_WIDTH = 2500;
//FIT CONSTANTS
const int CalibItem::SMOOTH_ITER = 5;
const int CalibItem::BG_ITER = 15;
const float CalibItem::SP_THRES = 0.01;
const int CalibItem::GAUS_SIGMA = 100;



int CalibItem::initMOCK() {
	meta = (DayMeta*)projFile->Get("daysMeta");
	if (!meta) meta = new DayMeta("daysMeta");
	return 0;
}

CalibItem::CalibItem() {
	printf("Read\n");
	for (int cellId=0; cellId<NCELLS; cellId++) rawWidth_Hs[cellId]=NULL;
}
CalibItem::CalibItem(int day, int firstMinute, int duration, bool fillSrcMap) {
	printf("Created\n");
	this->day = day;
	this->firstMinute = firstMinute;
	this->duration = duration;
	SetName(Form("day%d_fmin%d_dur%d",day,firstMinute,duration));
	for (int cellId=0; cellId<NCELLS; cellId++) {
		rawWidth_Hs[cellId] = new TH1F(Form("day%d_fmin%d_rawWidth_cell%d",day,firstMinute,cellId), "", NBINS, MIN_RAW_WIDTH, MAX_RAW_WIDTH);
	}

	if (!fillSrcMap) return;
	for (int f=0; f<HLDs_PER_ITEM; f++) addSrc();

}

void CalibItem::loadSrc() {
	for (auto&& p: srcIdMap) {
		TObject *obj = treesFile->Get(Form("%d", p.second));
		if (obj) {
			ch.Add(Form("%s/%d", TREES_FILE, p.second));
			printf("Tree %d loaded\n", p.second);
		}
	}
}

void CalibItem::fillHists() {
	loadSrc();
	int nEntries;
	int cellId[NCELLS];
	float rawWidth[NCELLS];
	float rawTime[NCELLS];
	ch.SetBranchAddress("nEntries", &nEntries);
	ch.SetBranchAddress("cellId", cellId);
	ch.SetBranchAddress("rawWidth", rawWidth);
	ch.SetBranchAddress("rawTime", rawTime);
	for (int e=0; e<ch.GetEntries(); e++) {
		ch.GetEvent(e);
		// printf("Event %d entries: %d\n", e, nEntries);
		for (int h=0; h<nEntries; h++) {
			if (cellId[h]<NCELLS) rawWidth_Hs[cellId[h]]->Fill(rawWidth[h]);
			// if (cellId[h]==0) printf("Cell 0 filled with: %.2f\n", rawWidth[h]);
		}
	}
	printf("Entries in cell0 hist after filling: %f\n", rawWidth_Hs[0]->GetEntries());
}

void CalibItem::fit(int id) {
	printf("Fitting [%d]\n",id);
	
	//fit for be data
	TFitResultPtr res=makeFit(3,rawWidth_Hs[id]);
	TF1 *fit;
	if (!(int)res) {
		beZ1[id] = res->Parameter(1);
		if (res->Parameters().size()>3) beZ2[id] = res->Parameter(4);
		if (beZ1[id]<600 && res->Parameters().size()>3) {
			beZ1[id]=res->Parameter(4);  
			if (res->Parameters().size()>6) beZ2[id]=res->Parameter(7);  
		}
		fit = rawWidth_Hs[id]->GetFunction("fit");
		fit->SetName(Form("widthBeFit_cell%d",id));
	} else beZ1[id] = beZ2[id] = 0;
}

TH1 *CalibItem::deriveHist(TH1 *h, int niter, const char *name) {
	int nbins = h->GetXaxis()->GetNbins();
	int xmin = h->GetXaxis()->GetXmin();
	int xmax = h->GetXaxis()->GetXmax();
	float *src = new float[nbins];
	for (int b=1; b<=nbins; b++) src[b-1] = h->GetBinContent(b);
	TSpectrum sp(1);
	sp.Background(src,nbins,niter,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,kFALSE,TSpectrum::kBackSmoothing3,kFALSE);	
	
	TH1 *res = new TH1F(name, "", nbins,xmin,xmax);
	for (int b=1; b<=nbins; b++) res->SetBinContent(b, src[b-1]);
	return res;
}

TFitResultPtr CalibItem::makeFit(int npeaks, TH1* &h) {
	TSpectrum sp(npeaks);
	string name = h->GetName();
	TH1* bg=deriveHist(h,BG_ITER,"bg");
	h->Add(bg,-1);
	delete bg;
	TH1 *smoothed=deriveHist(h,SMOOTH_ITER,"smoothed");
	delete h;
	h=smoothed;
	h->SetName(name.data());
	// smoothed->SetLineColor(kGreen);

	int n = sp.Search(smoothed,npeaks,"",SP_THRES);
	float *posX = sp.GetPositionX();
	float *posY = sp.GetPositionY();
	int *ind = new int[n];
	TMath::Sort(n, posX, ind,0);

	TString formula;
	for (int p=0; p<n; p++) formula+=(p==0) ? Form("gaus(%d)",p*3) : Form("+gaus(%d)",p*3);
	TF1 *fit = new TF1("fit",formula.Data());
	for (int p=0; p<n-1; p++) {
		fit->FixParameter(p*3, 	0);	
		fit->FixParameter(p*3+1, 0);	
		fit->FixParameter(p*3+2, 0);	
	}
	TFitResultPtr res;
	for (int p=0; p<n; p++) {
		fit->ReleaseParameter(p*3); 	fit->SetParameter(p*3, 	posY[ind[p]]);	
		fit->ReleaseParameter(p*3+1); 	fit->SetParameter(p*3+1, posX[ind[p]]);	
		fit->ReleaseParameter(p*3+2); 	fit->SetParameter(p*3+2, GAUS_SIGMA);	
		res=smoothed->Fit(fit,"qS");
		fit->FixParameter(p*3, 	 fit->GetParameter(p*3));
		fit->FixParameter(p*3+1, fit->GetParameter(p*3+1));
		fit->FixParameter(p*3+2, fit->GetParameter(p*3+2));
	}
	return res;
}

void CalibItem::addSrc() {
	string ffile(Form("%s/%03d/be19%03d%02d%02d0000.hld", HLD_ROOT, day, day, firstMinute/60, firstMinute%60));
	string lfile(Form("%s/%03d/be19%03d%02d%02d5908.hld", HLD_ROOT, day, day, (firstMinute+duration)/60, (firstMinute+duration)%60));
	vector<string> *hldList = meta->getHldList(day); 
	vector<string>::iterator it1 = upper_bound(hldList->begin(), hldList->end(), ffile);
	if (it1==hldList->end()) return;
	vector<string>::iterator it2 = upper_bound(hldList->begin(), hldList->end(), lfile); 
	int randOffset = r->Integer(it2-it1);
	printf("Day %d: srcIdMap.size()=%d  rstep=%d\n", day, srcIdMap.size(), randOffset);
	srcIdMap[*(it1+randOffset)]= (*meta->getSrcID())++;
}

bool CalibItem::isOverlaps(int d, int fmin, int dur)  {
	return day==d && ( firstMinute<=fmin && fmin<=firstMinute+duration || firstMinute<=fmin+dur && fmin+dur<=firstMinute+duration );
}

// bool CalibItem::operator==(const CalibItem* &other) const {
	// return day==other->getDay() && firstMinute==other->getFirstMinute() && duration==other->getDuration();
// }


CalibItem::~CalibItem() {
	if (projFile) {
		projFile->cd();
		meta->Write("", TObject::kOverwrite);
		projFile->Close();
		projFile = NULL;
		printf("Project file closed\n");
	}
}

// bool operator==(CalibItem*, const CalibItem* ) {
	// return l_item->getDay()==r_item->getDay() && l_item->getFirstMinute()==r_item->getFirstMinute() && l_item->getDuration()==r_item->getDuration();
// }
