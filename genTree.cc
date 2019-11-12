#include <TTree.h>
#include <TFile.h>
#include <stdio.h>
#include <string>

#include <hloop.h>
#include <hcategorymanager.h>
#include <hwallraw.h>

using namespace std;

const int NCELLS = 304;

int main(int argc, char const *argv[])
{
	if (argc <4) {
		printf("Not enough args!!\n");
		return 1;
	}
	const char *dst=argv[1];
	const char *treesDir=argv[2];
	int srcID = stoi(string(argv[3]));

	TFile ioF(Form("%s/%d.root",treesDir,srcID), "recreate");
	TTree t(Form("%d",srcID), "");
	int nEntries;
	int cellId[NCELLS];
	float rawTime[NCELLS];
	float rawWidth[NCELLS];
	t.Branch("nEntries", &nEntries, "nEntries/I");
	t.Branch("cellId", cellId, "cellId[nEntries]/I");
	t.Branch("rawTime", rawTime, "rawTime[nEntries]/F");
	t.Branch("rawWidth", rawWidth, "rawWidth[nEntries]/F");

	new HLoop(1);
	gLoop->addFile(dst);
	gLoop->setInput("-*,+HWallRaw");
	HCategory *catWallRaw = gLoop->getCategory("HWallRaw");
	HWallRaw *wallRaw;
	for (int e=0; e<gLoop->getChain()->GetEntries(); e++) {
		gLoop->nextEvent(e);
		for (nEntries=0; nEntries<catWallRaw->getEntries(); nEntries++) {
			wallRaw=HCategoryManager::getObject(wallRaw, catWallRaw, nEntries);
			cellId[nEntries]=wallRaw->getCell();
			rawTime[nEntries]=wallRaw->getTime(1);
			rawWidth[nEntries]=wallRaw->getWidth(1);
		}
		t.Fill();
	}

	ioF.cd();
	t.Write();
	ioF.Close();
	return 0;
}