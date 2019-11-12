#include "DayMeta.h"
#include <TSystem.h>
#include <cstring>

using namespace std;

ClassImp(DayMeta);

// const char *DayMeta::HLD_ROOT = "/lustre/nyx/hades/raw/mar19/default/tsm";
const char *DayMeta::HLD_ROOT = "/lustre/nyx/hades/raw2/mar19/default/tsm";
// const char *DayMeta::HLD_ROOT = "~/lustre/raw/mar19/default/tsm";
const char *DayMeta::TEMP_TXT = "~/ver4/temp.txt";

DayMeta::DayMeta() {
	srcID = 0;
	for (int d=0; d<=LAST_DAY-FIRST_DAY; d++) hldList[d] = 0;
}

DayMeta::DayMeta(const char *name) : TNamed(name,"") {
	srcID = 0;
	ssh_session session = ssh_new();

	//init connection
	initConnection(session);
	
	for (int d=0; d<=LAST_DAY-FIRST_DAY; d++) {
		hldList[d] = new vector<string>;
		const char *comm = Form("find %s/%03d -name \"be19%03d*\" | sort",HLD_ROOT,FIRST_DAY+d,FIRST_DAY+d);
		printf("Search command: %s\n",comm);
		
		stringstream *strStream=exec(session, comm);
    	std::string str;
		while(std::getline(*strStream, str)) {
			hldList[d]->push_back(str);
		}
	}

	//disconnect
	disconnect(session);
}

void DayMeta::initConnection(ssh_session s) {
	ssh_options_set(s, SSH_OPTIONS_HOST, "kronos.hpc");
	ssh_options_set(s, SSH_OPTIONS_USER, "dborisen");
	ssh_connect(s);
	authenticate_pubkey(s);
}

stringstream *DayMeta::exec(ssh_session s, const char *comm) {
	stringstream *strStream = new stringstream;
	char buffer[1024];

	//establish channel and do request 
	ssh_channel ch = ssh_channel_new(s);
	ssh_channel_set_blocking(ch, 1);
	ssh_channel_open_session(ch);
	// ssh_channel_request_shell(ch);
	// ssh_channel_write(ch, comm, strlen(comm));
	ssh_channel_request_exec(ch, comm);
	
	//read result
	int nbytes = ssh_channel_read(ch, buffer, sizeof(buffer)-1, 0);
    buffer[nbytes]='\0';
    while (nbytes > 0) {
        // fwrite(buffer, 1, nbytes, stdout);
		*strStream << buffer;        
        nbytes = ssh_channel_read(ch, buffer, sizeof(buffer)-1, 0);
	   	buffer[nbytes]='\0';
    }

	//close channel
	ssh_channel_send_eof(ch);
	ssh_channel_close(ch);
	ssh_channel_free(ch);	
	
	return strStream;
}

void DayMeta::disconnect(ssh_session s) {
	ssh_disconnect(s);
	ssh_free(s);
}

int DayMeta::authenticate_pubkey(ssh_session session)
{
  int rc;
  rc = ssh_userauth_publickey_auto(session, NULL, NULL);
  if (rc == SSH_AUTH_ERROR)
  {
     fprintf(stderr, "Authentication failed: %s\n",
       ssh_get_error(session));
     return SSH_AUTH_ERROR;
  }
  return rc;
}

std::vector<std::string> *DayMeta::getHldList(int d) {
	if (d>=FIRST_DAY && d<=LAST_DAY) return hldList[d-FIRST_DAY];
	else return NULL;
}


DayMeta::~DayMeta() {

}
