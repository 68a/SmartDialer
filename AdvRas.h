#define MAX_ENTRY_NAME_LEN 255
#define MAX_PHONE_ENTRY 10
#define MAX_LOG_LEN		4096

#include "ras.h"
typedef struct {
	TCHAR entryName[MAX_ENTRY_NAME_LEN+1];
} PHONE_ENTRY;

int GetPhoneBookEntries(PHONE_ENTRY (&phoneEntries)[MAX_PHONE_ENTRY]);
int GetCurrentConnections(BYTE (&RasConnData)[10*sizeof(RASCONN)]);
void ReadDefaultPhonebookEntry();
void WriteDefaultPhonebookEntry();

int Dial(const char * szEntryName);
void Hangup(HRASCONN hRasconn);

void AddRoute();
void DelRoute();

extern TCHAR gszLogData[MAX_LOG_LEN];
extern bool gbRotating;
extern TCHAR gszIniFilePath[MAX_PATH];