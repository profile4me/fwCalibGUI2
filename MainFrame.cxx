#include "MainFrame.h"
#include <TApplication.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <TPRegexp.h>
#include <TGaxis.h>
#include <TH1.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TFileMerger.h>
#include <TSystemDirectory.h>
#include <TLegend.h>

using namespace std;

ClassImp(MainFrame);

const char *MainFrame::TASK_FILE =  "task.list";
// const char *MainFrame::TREES_DIR = "/lustre/nyx/hades/user/dborisen/dst_cache/trees";
const char *MainFrame::TREES_DIR = "/u/dborisen/lustre/user/dborisen/dst_cache/trees";
const char *MainFrame::LOG_DIR = "/u/dborisen/lustre/user/dborisen/dst_cache/log";

MainFrame::MainFrame(int w, int h) : TGMainFrame(gClient->GetRoot(),w,h) {
	fday = 367;
	lday = 0;
	itemsPerDay = 2;

	SetLayoutManager(new TGHorizontalLayout(this));
	
	//LEFT SIDE
	leftSide = new TGVerticalFrame(this/*, w*0.2, h*/);
	itemsView = new TGCanvas(leftSide, w*0.2, h*0.7);
	itemsContainer = new TGListTree(itemsView,0);
	taskBox = new TGListBox(leftSide);
	leftSide->AddFrame(itemsView, new TGLayoutHints(kLHintsExpandX /*| kLHintsTop*/));
	leftSide->AddFrame(taskBox, new TGLayoutHints(kLHintsExpandX /*| kLHintsBottom*/ | kLHintsExpandY));
		//controls
		controls2 = new TGHorizontalFrame(this);
		btnRun = new TGTextButton(controls2, "Run");
		btnCheck = new TGTextButton(controls2, "Check");
		btnAggregate = new TGTextButton(controls2, "Aggregate");
		controls2->AddFrame(btnRun, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls2->AddFrame(btnCheck, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls2->AddFrame(btnAggregate, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
	leftSide->AddFrame(controls2, new TGLayoutHints(kLHintsBottom | kLHintsExpandX));


	//RIGHT SIDE
	rightSide = new TGVerticalFrame(this/*, w*0.8,h*/);
		//controls
		controls = new TGHorizontalFrame(rightSide, 1, 20);
		btnLoad = new TGTextButton(controls, "Load");
		numField1 = new TGNumberEntry(controls, FIRST_DAY, 3, 0, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, 1, 366);
		numField2 = new TGNumberEntry(controls, LAST_DAY, 3, 1, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, 1, 366);
		numField3 = new TGNumberEntry(controls, 2, 2, 2, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMin, 1);
		btnRearrange = new TGTextButton(controls, "Rearrange");
		btnFill = new TGTextButton(controls, "Fill");
		btnFit = new TGTextButton(controls, "Fit");
		btnDrawTrends = new TGTextButton(controls, "DrawTrends");
		btnSave = new TGTextButton(controls, "Save");
		
		controls->AddFrame(btnLoad, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(numField1, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(numField3, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(numField2, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(btnRearrange, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(btnFill, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(btnFit, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(btnDrawTrends, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
		controls->AddFrame(btnSave, new TGLayoutHints(kLHintsCenterX | kLHintsExpandY));
	canva = new TRootEmbeddedCanvas("Trends", rightSide, w/2, h);
	canva->GetCanvas();
	rightSide->AddFrame(controls, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
	rightSide->AddFrame(canva, new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsExpandY));
	
	AddFrame(leftSide, new TGLayoutHints(kLHintsLeft | kLHintsExpandY ));
	AddFrame(rightSide, new TGLayoutHints(kLHintsRight | kLHintsExpandY | kLHintsExpandX));

	MapSubwindows();
	MapWindow();
	Layout();

	rootItem=NULL;
	initItems();
	// loadItems();

	//CONTROLS1
	Connect(btnLoad, "Clicked()", "MainFrame", this, "load()");
	Connect(numField1, "ValueSet(Long_t)", "MainFrame", this, "checkFields()");
	Connect(numField3, "ValueSet(Long_t)", "MainFrame", this, "checkFields()");
	Connect(numField2, "ValueSet(Long_t)", "MainFrame", this, "checkFields()");
	Connect(btnRearrange, "Clicked()", "MainFrame", this, "initItems()");
	Connect(btnFill, "Clicked()", "MainFrame", this, "fillHists()");
	Connect(btnFit, "Clicked()", "MainFrame", this, "fitHists()");
	Connect(btnDrawTrends, "Clicked()", "MainFrame", this, "DrawTrends()");
	Connect(btnSave, "Clicked()", "MainFrame", this, "save()");

	//CONTROLS2
	Connect(btnRun, "Clicked()", "MainFrame", this, "run()");
	Connect(btnAggregate, "Clicked()", "MainFrame", this, "combineTrees()");
}

void MainFrame::CloseWindow() {
	save();
	gApplication->Terminate(0);
}

void MainFrame::checkFields() {
	// Info("CheckFields()", "");
	int f1Val = (int) numField1->GetIntNumber();
	int f2Val = (int) numField2->GetIntNumber();
	if (f1Val<=f2Val) {
		btnRearrange->SetEnabled(1);
		fday = f1Val;
		lday = f2Val;
	}
	else btnRearrange->SetEnabled(0);
}


int MainFrame::getColor(int n) {
	return COLORS[n%14];
}

void MainFrame::initItemByOlds(int day, int firstMinute, int duration) {
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

void MainFrame::purgeProjFile() {
	for (auto *i: oldItems) {
		TObject *obj = CalibItem::projFile->Get(i->GetName());
		CalibItem::projFile->Remove(obj);
	}
	oldItems.clear();
}

void MainFrame::purgeDir(const char *dirPath) {
	TSystemDirectory treesDir("dir", dirPath);
	for (auto *o: *treesDir.GetListOfFiles()) {
		const char *fname = Form("%s/%s", dirPath, o->GetName());
		printf("Removing: %s\n", fname);
		gSystem->Unlink(fname);
	}
}

void MainFrame::initItems(int itemsPerDay) {
	itemsPerDay = numField3->GetIntNumber();

	if (CalibItem::projFile==NULL) {
		CalibItem::projFile = new TFile(CalibItem::PROJ_FILE,"update");
		CalibItem::initMOCK();
	}
	newItems.clear();

	int fday_l=367;
	int lday_l=0;
	bool projEmpty = true;
	for (auto *k: *CalibItem::projFile->GetListOfKeys()) {
		TObject *obj = CalibItem::projFile->Get(k->GetName());
		if (!obj->InheritsFrom(CalibItem::Class())) continue;
		projEmpty = false;
		oldItems.push_back((CalibItem*)obj);
		if ( ((CalibItem*)obj)->getDay()<fday_l ) fday_l = ((CalibItem*)obj)->getDay();
		if ( ((CalibItem*)obj)->getDay()>lday_l ) lday_l = ((CalibItem*)obj)->getDay();
	}
	if (fday==367 && lday==0) {
		fday = (projEmpty) ? FIRST_DAY : fday_l;
		lday = (projEmpty) ? LAST_DAY : lday_l;
	}
	numField1->SetIntNumber(fday);
	numField2->SetIntNumber(lday);

	int step = 24*60/itemsPerDay;
	
	for (int d=fday; d<=lday; d++) {
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
	loadItems();
}


void MainFrame::loadItems() {
	if (!rootItem) {
		rootItem = new TGListTreeItemStd(gClient, "run");
		itemsContainer->AddItem(0, rootItem);
		rootItem->SetOpen(1);
	} else {
		itemsContainer->DeleteChildren(rootItem);
	}

	int curDay = 0;
	TGListTreeItem* curItem = NULL;
	for (auto *citem: newItems) {
		printf("Another day CalibItem found!\n");
		int hh = citem->getFirstMinute()/60;
		int mm = citem->getFirstMinute()%60;
		if (curDay!=citem->getDay()) {
			curDay = citem->getDay();
			curItem = new TGListTreeItemStd(gClient, Form("day %d",curDay)); 
			itemsContainer->AddItem(rootItem, curItem);
			itemsContainer->AddItem(curItem, Form("%02d:%02d",hh,mm));
			// days.push_back(item);
		} else {
			itemsContainer->AddItem(curItem, Form("%02d:%02d",hh,mm));
		}
	}
}

void MainFrame::combineTrees() {
	printf("combineTrees(): BEGIN\n");

	TFileMerger merger;
	TSystemDirectory treesDir("treesDir",TREES_DIR);
	TPRegexp reg(".*.root");
	printf("combineTrees(): files in TREES_DIR: %d\n", treesDir.GetListOfFiles()->GetEntries());
	for (auto *l: *treesDir.GetListOfFiles()) {
		if (reg.Match(l->GetName())) merger.AddFile(Form("%s/%s",TREES_DIR,l->GetName()));
	}
	merger.OutputFile("trees.root");
	merger.Merge();
	
	purgeDir(TREES_DIR);

	printf("combineTrees(): END\n");
}

void MainFrame::fillHists() {
	printf("fillHists(): BEGIN\n");

	for (auto *i : newItems) {
		i->fillHists();
	}

	printf("fillHists(): END\n");
}

void MainFrame::fitHists() {
	printf("fitHists(): BEGIN\n");

	for (auto *i : newItems) {
		for (int cellId=0; cellId<CalibItem::NCELLS; cellId++) i->fit(cellId);
	}

	printf("fitHists(): END\n");
}

void MainFrame::DrawTrends(int fcell, int lcell) {

	//form bins and labels
	// int fday = newItems.front()->getDay();
	// int lday = newItems.back()->getDay();
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
	TCanvas *c = canva->GetCanvas();
	st->Draw("nostack");
	// st->Draw();
	st->GetXaxis()->SetAxisColor(0);
	st->GetXaxis()->SetLabelColor(0);
	TLegend *legend = c->BuildLegend(0.5,0.2,1.0,0.45);
/*
	legend->SetX1NDC(0.5);
	legend->SetY1NDC(0.0);
	legend->SetX2NDC(1.0);
	legend->SetY2NDC(0.3);
	legend->Draw();
*/

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
}

void MainFrame::run() {
	purgeDir(LOG_DIR);

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
	// return;

	ssh_session session = ssh_new();
	DayMeta::initConnection(session);
	stringstream *strStream = DayMeta::exec(session, "/bin/bash -c \"cd /u/dborisen/ver4; . send.sh\"");
	// stringstream *strStream = DayMeta::exec(session, "cat /u/dborisen/ver4/send.sh");
	// stringstream *strStream = DayMeta::exec(session, "ls -ltr /lustre");
	string str;
	while (getline(*strStream, str)) {
		printf("%s\n", str.data());
	}
	DayMeta::disconnect(session);
}

void MainFrame::save() {
	Info("save()", "BEGIN");

	CalibItem::projFile->cd();
	for (auto *i : newItems) {
		i->Write("", TObject::kOverwrite);
	}
	for (auto *i : newItems) delete i;
	newItems.clear();
	CalibItem::projFile = NULL;
	
	//GUI CHANGES
	itemsContainer->DeleteChildren(rootItem);
	itemsContainer->ClearViewPort();
	canva->Clear();
	numField1->SetState(0);
	numField2->SetState(0);
	numField3->SetState(0);
	btnRearrange->SetEnabled(0);
	btnFill->SetEnabled(0);
	btnFit->SetEnabled(0);
	btnDrawTrends->SetEnabled(0);
	btnSave->SetEnabled(0);
	//
	btnRun->SetEnabled(0);
	btnCheck->SetEnabled(0);
	btnAggregate->SetEnabled(0);

	Info("save()", "END");
}

void MainFrame::load() {
	initItems();
	// loadItems();
	//GUI CHANGES
	numField1->SetState(1);
	numField2->SetState(1);
	numField3->SetState(1);
	btnRearrange->SetEnabled(1);
	btnFill->SetEnabled(1);
	btnFit->SetEnabled(1);
	btnDrawTrends->SetEnabled(1);
	btnSave->SetEnabled(1);
	//
	btnRun->SetEnabled(1);
	btnCheck->SetEnabled(1);
	btnAggregate->SetEnabled(1);
}


