#ifndef __MainFrame__
#define __MainFrame__

#include <TGFrame.h>
#include <TGListTree.h>
#include <TGListBox.h>
#include <TGCanvas.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TFile.h>
#include <vector>
#include <list>
#include "CalibItem.h"

const int COLORS[] = {1,   920,  632,  416, 600, 400, 616, 432, 800, 820, 840,  860, 880, 900};
const int FIRST_DAY = 62;
const int LAST_DAY = 87;
class MainFrame : public TGMainFrame
{
	static const char *TASK_FILE;
	static const char *TREES_DIR;
	static const char *LOG_DIR;

	int fday;
	int lday;
	int itemsPerDay;

	TGVerticalFrame *leftSide;
	TGCanvas *itemsView;
	TGListTree *itemsContainer;
	TGListTreeItem *rootItem;
	TGListBox *taskBox;
	//CONTROLS 2
	TGHorizontalFrame *controls2;
	TGTextButton *btnRun;
	TGTextButton *btnCheck;
	TGTextButton *btnAggregate;
	///

	TGVerticalFrame *rightSide;
	//CONTROLS 1
	TGHorizontalFrame *controls;
	TGTextButton *btnLoad;
	TGNumberEntry *numField1;
	TGNumberEntry *numField2;
	TGNumberEntry *numField3;
	TGTextButton *btnRearrange;
	TGTextButton *btnFill;
	TGTextButton *btnFit;
	TGTextButton *btnDrawTrends;
	TGTextButton *btnSave;
	///
	TRootEmbeddedCanvas *canva;

	std::list<CalibItem*> oldItems;
	std::list<CalibItem*> newItems;
	std::vector<float> dbins, hbins;
	std::vector<const char*> dlabels, hlabels;

public:
	MainFrame(int w, int h);
	void CloseWindow();
	void checkFields();
	int getColor(int);
	void purgeProjFile();
	void initItemByOlds(int day, int firstMinute, int duration);
	void loadItems();
	void purgeDir(const char*);
	//SLOTS
	void combineTrees();
	void fillHists();
	void fitHists();
	void initItems(int itemsPerDay=1);
	void DrawTrends(int=0, int=10);
	void run();
	void save();
	void load();

	~MainFrame() {
	}
	ClassDef(MainFrame,0);
};

#endif
