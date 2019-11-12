#include "CalibItem.h"
#include <TFile.h>
#include <TString.h>
#include <TPRegexp.h>
#include <TObjString.h>
#include <TFileMerger.h>
#include <TSystemDirectory.h>
#include <TCanvas.h>
#include <TH1.h>
#include <THStack.h>
#include <TGaxis.h>
#include <fstream>
#include <list>
#include <algorithm>
#include <iostream>

using namespace std;

const int FIRST_DAY = 62;
const int LAST_DAY = 86;
const char *TASK_FILE =  "task.list";
const char *SUBMIT_DIR = "/lustre/nyx/hades/user/dborisen/dst_cache";
const char *TREES_DIR = "/lustre/nyx/hades/user/dborisen/dst_cache/trees";
const int COLORS[] = {1,   920,  632,  416, 600, 400, 616, 432, 800, 820, 840,  860, 880, 900};

list<CalibItem*> oldItems;
list<CalibItem*> newItems;
vector<float> dbins, hbins;
vector<const char*> dlabels, hlabels;


int getColor(int n) {
	return COLORS[n%14];
}

void initItemByOlds(int day, int firstMinute, int duration) {
	TPRegexp re("be19[0-9]{3}([0-9]{2})([0-9]{2})");
	CalibItem *item = new CalibItem(day, firstMinute, duration, false);
	for (auto *i: oldItems) {
		if (i->isOverlaps(day, firstMinute, duration)) {
			for (auto p: *i->getSrcIdMap()) {
				int hldMinutes = ((TObjString*) re.MatchS(p.first.data())->At(1) )->String().Atoi()*60+ ((TObjString*) re.MatchS(p.first.data())->At(2) )->String().Atoi();
				if (hldMinutes>=firstMinute && hldMinutes<=firstMinute+duration) (*item->getSrcIdMap())[p.first] = p.second;
			}
		}
	}
	int lackingHLDs = CalibItem::HLDs_PER_ITEM-item->getSrcIdMap()->size();
	while (lackingHLDs-- > 0) item->addSrc(); 
	newItems.push_back(item);
}

void purgeProjFile() {
	for (auto *i: oldItems) {
		TObject *obj = CalibItem::projFile->Get(i->GetName());
		CalibItem::projFile->Remove(obj);
	}
}

void initItems(int itemsPerDay) {
	for (auto *k: *CalibItem::projFile->GetListOfKeys()) {
		TObject *obj = CalibItem::projFile->Get(k->GetName());
		if (!obj->InheritsFrom(CalibItem::Class())) continue;
		oldItems.push_back((CalibItem*)obj);
	}

	int step = 24*60/itemsPerDay;
	
	for (int d=FIRST_DAY; d<=LAST_DAY; d++) {
		for (int b=0; b<itemsPerDay; b++) {
			list<CalibItem*>::iterator it = find_if(oldItems.begin(), oldItems.end(), [&](CalibItem* item) { 
																							return item->getDay()==d && item->getFirstMinute()==b*step && item->getDuration()==step; 
																						});
			if (it != oldItems.end()) {
				newItems.push_back(*it); 
				oldItems.erase(it);
			} else {
				initItemByOlds(d, b*step, step);
			}
		}
	}

	purgeProjFile();
}

//format of task.list
// #1 submitDir
// #2 hld_file			srcID 			

void combineTrees() {
	TFileMerger merger;
	TSystemDirectory treesDir("treesDir",TREES_DIR);
	TPRegexp reg(".*.root");
	for (auto *l: *treesDir.GetListOfFiles()) {
		if (reg.Match(l->GetName())) merger.AddFile(Form("%s/%s",TREES_DIR,l->GetName()));
	}
	merger.OutputFile("trees.root");
	merger.Merge();
}

void fillHists() {
	for (auto *i : newItems) {
		i->fillHists();
	}
}

void fitHists() {
	for (auto *i : newItems) {
		for (int cellId=0; cellId<CalibItem::NCELLS; cellId++) i->fit(cellId);
	}
}


TCanvas *drawZ1trends(int fcell=0, int lcell=10) {
	//form bins and labels
	int fday = newItems.front()->getDay();
	int lday = newItems.back()->getDay();
	int xrange = (lday-fday+1)*1440;
	int curDay = 0;
	for (auto *i: newItems) {
		if (i->getDay()!=curDay) {
			curDay = i->getDay();
			dbins.push_back(i->getDay()*1440+i->getFirstMinute());
			dlabels.push_back(Form("%d",i->getDay()));
		}
		if (i->getFirstMinute()%60==0 && i->getDuration()%60==0 /*&& i.duration!=1440*/) {
			hbins.push_back(i->getDay()*1440+i->getFirstMinute());
			hlabels.push_back(Form("%02dh",i->getFirstMinute()/60));
		}
	}
	dbins.push_back((lday+1)*1440);
	hbins.push_back((lday+1)*1440);

	//create and fill hists
	THStack *st = new THStack;
	for (int cellId=fcell; cellId<=lcell; cellId++) {
		TH1 *hist = new TH1F(Form("Z1trend_cell%d",cellId), Form("cell %d",cellId), hbins.size()-1, hbins.data());
		hist->GetXaxis()->SetAxisColor(0);
		hist->GetXaxis()->SetLabelColor(0);
		int bin=1;
		for (auto *i: newItems) {
			if (bin==1) printf("Cell %d z1=%.2f\n", cellId, i->getZ1(cellId));
			hist->SetBinContent(bin, i->getZ1(cellId));
			bin++;
		}
		hist->SetLineColor(getColor(cellId));
		st->Add(hist);
	}
	TCanvas *c = new TCanvas;
	st->Draw("nostack");
	// st->Draw();
	st->GetXaxis()->SetAxisColor(0);
	st->GetXaxis()->SetLabelColor(0);
	c->BuildLegend();

	//custom x-axis
	float dayAxisPos = -.05*st->GetMaximum("nostack");
	for(UShort_t bin = 0; bin < dbins.size()-1 ; bin++ ){
	    TGaxis *ax = new TGaxis(dbins.at(bin), dayAxisPos, dbins.at(bin+1), dayAxisPos, dbins.at(bin), dbins.at(bin+1), 1 ,"US");
	    ax->SetTickSize( 0.2 );
	    if(bin == 0 || bin == dbins.size() -2 ) ax->SetTickSize( 0. );
	    ax->SetTitle( dlabels.at(bin) );
	    ax->SetTitleSize(0.02);
	    ax->SetTitleOffset(.4);
	    ax->CenterTitle( kTRUE );
	    ax->Draw();
	}
	int emptyRegionSize = 0;
	int lastBinSize = 0;
	for(UShort_t bin = 0; bin < hbins.size()-1 ; bin++ ){
	    TGaxis *ax = new TGaxis(hbins.at(bin), 0, hbins.at(bin+1), 0, hbins.at(bin), hbins.at(bin+1), 1 ,"US");
	    ax->SetTickSize( 0.2 );
	    if(bin == 0 || bin == hbins.size() -2 ) ax->SetTickSize( 0. );

	    ////
	    int binSize = (int)(hbins.at(bin+1)-hbins.at(bin));
	   	if (binSize!=lastBinSize) emptyRegionSize=0;
	    if (binSize<0.03*xrange) {
	    	emptyRegionSize+=binSize;
	    	if (emptyRegionSize>0.03*xrange) {
	    		ax->SetTitle(hlabels.at(bin));
	    		emptyRegionSize=0;
	    	}
	    } else ax->SetTitle(hlabels.at(bin));
	    lastBinSize = binSize;
	    ////

	    ax->SetTitleSize(0.015);
	    ax->SetTitleOffset(.4);
	    ax->CenterTitle( kTRUE );
	    ax->Draw();
	}

	c->Update();
	return c;
}

int main(int argc, char const *argv[])
{
	TString str(argv[1]);
	if (!str.CompareTo("-merge")) {
		combineTrees();
		return 0;
	}
	int itemsPerDay = str.Atoi();
	initItems(itemsPerDay==0 ? 1 : itemsPerDay);
	// for (auto *i: newItems) i->loadSrc();
	if (!str.CompareTo("-fill")) {
		fillHists();
		// return 0;
	}
	if (!str.CompareTo("-fit")) {
		fitHists();
	}
	if (!str.CompareTo("-trends")) {
		drawZ1trends()->Print("test.pdf)");
	}
	if (!str.CompareTo("-width")) {
		THStack st;
		for (auto *i: newItems) {
			TH1 *h = i->getWidth(0); 
			printf("Entries in h: %f\n", h->GetEntries());
			st.Add(h);
		}
		

		TCanvas c;
		c.Print("test.pdf[");
		st.Draw("PADS");
		c.Update();
		c.Print("test.pdf");
		c.Print("test.pdf]");
	}

	ofstream oS(TASK_FILE, ofstream::out);
	int day = 0;
	// oS << SUBMIT_DIR << endl;
	for (auto *i : newItems) {
		if (i->getDay()!= day) {
			day = i->getDay();
			oS << "#";
			oS.width(3);
			oS.fill('0');
			oS << day << endl;
			// oS.reset();
		}
		for (auto&& p : *i->getSrcIdMap()) {
			oS << p.first << "\t" << p.second << endl;
		}
	}
	oS.close();

	CalibItem::projFile->cd();
	for (auto *i : newItems) {
		i->Write("", TObject::kOverwrite);
	}
	for (auto *i : newItems) delete i;
	return 0;
}