#include "MainFrame.h"
#include <TApplication.h>

int main(int argc, char *argv[])
{
	TApplication app("app", &argc, argv);

	MainFrame *mframe = new MainFrame(800,600);
	// mframe.loadItems();

	app.Run();
	return 0;
}