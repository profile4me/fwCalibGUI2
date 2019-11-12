#ifndef __DayMeta__
#define __DayMeta__

#include <TNamed.h>
#include <vector>
#include <string>
#include <sstream>
#include <libssh/libssh.h>

class DayMeta : public TNamed
{
public:
	static const char *HLD_ROOT;
	static const char *TEMP_TXT;
	static const int FIRST_DAY = 62;
	static const int LAST_DAY = 87;
	int srcID;
private:
	std::vector<std::string> *hldList[LAST_DAY-FIRST_DAY+1];

	static int authenticate_pubkey(ssh_session);
public:
	static void initConnection(ssh_session);
	static std::stringstream *exec(ssh_session, const char*);
	static void disconnect(ssh_session);

	DayMeta();
	DayMeta(const char *);
	~DayMeta();
	int *getSrcID() {return &srcID;}
	std::vector<std::string> *getHldList(int);
	ClassDef(DayMeta,1);
};

#endif