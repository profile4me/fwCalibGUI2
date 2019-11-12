#ifndef __CalibItem__
#define __CalibItem__

#include <TNamed.h>
#include <TFile.h>
#include <map>
#include <TChain.h>
#include <TH1.h>
#include <TRandom.h>
#include "DayMeta.h"



class CalibItem : public TNamed
{
public:
	static TFile *projFile;
	static const int HLDs_PER_ITEM;
	static const int NCELLS = 208; //304;;

	static const char *PROJ_FILE;
private:
	static DayMeta *meta; 
	static const int MOCK;
	static const char *HLD_ROOT;
	static TRandom *r;
	static const char *TREES_FILE;
	static TFile *treesFile;
	static const int NBINS;
	static const float MIN_RAW_WIDTH;
	static const float MAX_RAW_WIDTH;

	//fit constants
	static const int SMOOTH_ITER;
	static const int BG_ITER;
	static const float SP_THRES;
	static const int GAUS_SIGMA;


	int day;
	int firstMinute;
	int duration;
	std::map<std::string, int> srcIdMap;
	TChain ch;					//!

	TH1 *rawWidth_Hs[NCELLS];
	float beZ1[NCELLS];
	float beZ2[NCELLS];

public:
	static int initMOCK();
	
	CalibItem();
	CalibItem(int, int, int, bool = true);
	void loadSrc();
	void fillHists();
	TH1 *deriveHist(TH1 *h, int niter, const char *name);
	TFitResultPtr makeFit(int npeaks, TH1* &h);
	void fit(int);
	

	int getDay() const { return day; }
	int getFirstMinute() const { return firstMinute; }
	int getDuration() const { return duration; }
	float getZ1(int cellId) { return beZ1[cellId]; }
	float getZ2(int cellId) { return beZ2[cellId]; }
	TH1 *getWidth(int cellId) { return rawWidth_Hs[cellId]; }
	std::map<std::string, int> *getSrcIdMap() { return &srcIdMap; }
	void addSrc();
	bool isOverlaps(int, int, int);
	// bool operator==(const CalibItem* &) const;
	~CalibItem();
	ClassDef(CalibItem,1);	
};

#endif