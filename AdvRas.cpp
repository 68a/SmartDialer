#include "StdAfx.h"
#include <windows.h>
#include <stdio.h>
#include "ras.h"
#include "raserror.h"
#include "advras.h"

#pragma comment(lib, "rasapi32.lib")


TCHAR gszLogData[MAX_LOG_LEN];
extern TCHAR			gszDefaultEntryName[MAX_ENTRY_NAME_LEN];

#define INI_FILE_NAME "smartdialer.ini"
TCHAR gszIniFilePath[MAX_PATH];

int GetPhoneBookEntries(PHONE_ENTRY (&phoneEntries)[MAX_PHONE_ENTRY])
{
	DWORD dwCb = 0;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwEntries = 0;
    LPRASENTRYNAME lpRasEntryName = NULL;
    
    // Call RasEnumEntries with lpRasEntryName = NULL. dwCb is returned with the required buffer size and 
    // a return code of ERROR_BUFFER_TOO_SMALL
    dwRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &dwCb, &dwEntries);

    if (dwRet == ERROR_BUFFER_TOO_SMALL){
        // Allocate the memory needed for the array of RAS entry names.
        lpRasEntryName = (LPRASENTRYNAME) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
        if (lpRasEntryName == NULL){
            wprintf(L"HeapAlloc failed!\r\n");
            return 0;
        }
		// The first RASENTRYNAME structure in the array must contain the structure size
        lpRasEntryName[0].dwSize = sizeof(RASENTRYNAME);
        
        // Call RasEnumEntries to enumerate all RAS entry names
        dwRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &dwCb, &dwEntries);

        // If successful, print the RAS entry names 
        if (ERROR_SUCCESS == dwRet){
            wprintf(L"The following RAS entry names were found:\n");
            for (DWORD i = 0; i < min(dwEntries,MAX_PHONE_ENTRY) ; i++){
                //wsprintf(phoneEntries[i].entryName, L"%s\n", lpRasEntryName[i].szEntryName);
				memset(phoneEntries[i].entryName, 0, sizeof(phoneEntries[i].entryName));
				strcpy_s( phoneEntries[i].entryName, sizeof(phoneEntries[i].entryName), lpRasEntryName[i].szEntryName);
            }
        }
        //Deallocate memory for the connection buffer
        HeapFree(GetProcessHeap(), 0, lpRasEntryName);
        lpRasEntryName = NULL;
        
	}
	return min(dwEntries,MAX_PHONE_ENTRY);
}
int GetCurrentConnections(BYTE (&RasConnData)[10*sizeof(RASCONN)])
{
    // Get the current list of connections 
	
	//BYTE        RasConnData[10*sizeof(RASCONN)];
    LPRASCONN pRasConn = (LPRASCONN) RasConnData;   

    pRasConn->dwSize = sizeof(RASCONN);   
    DWORD Connections = 0;   
    DWORD cb = sizeof(RasConnData);   
    if ( RasEnumConnections(pRasConn, &cb, &Connections)) {   
 
        return 0;      
    }  
	return Connections;
}
void Hangup(HRASCONN hRasconn)
{
	RasHangUp(hRasconn);
}
void AddRoute()
{
	static char runStr[256] = "rundll32.exe cmroute.dll SetRoutes /STATIC_FILE_NAME \
addchnroutes.txt  /DONT_REQUIRE_URL   /IPHLPAPI_ACCESS_DENIED_OK";
	int ret = WinExec(runStr, SW_SHOW);
}
void DelRoute()
{
	static char runStr[256] = "rundll32.exe cmroute.dll SetRoutes /STATIC_FILE_NAME \
delchnroutes.txt  /DONT_REQUIRE_URL   /IPHLPAPI_ACCESS_DENIED_OK";
	int ret = WinExec(runStr, SW_HIDE);
}

void WriteLog(TCHAR * szMessage)
{
	int msgLen = strlen(szMessage);
	int logLen = strlen(gszLogData);
	if (msgLen + logLen > MAX_LOG_LEN -1 ) {
		if (msgLen < MAX_LOG_LEN) {
			strcpy_s(gszLogData, szMessage);
		}
	}
	else {
		strcat_s(gszLogData, szMessage);
	}
}
// Callback function RasDialFunc()
void WINAPI RasDialFunc(UINT unMsg, 
                        RASCONNSTATE rasconnstate, 
                        DWORD dwError )
{
    char szRasString[256] = {0}; // Buffer for storing the error string
    TCHAR szTempBuf[256] = {0};  // Buffer used for printing out the text

    if (dwError)  // Error occurred
    {

        return;
    }

   // Map each of the states of RasDial() and display on the screen
   // the next state that RasDial() is entering
   switch (rasconnstate)
   {
      case RASCS_OpenPort:
			gbRotating = TRUE;
            sprintf (szTempBuf,"RASCS_OpenPort = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Opening port...\r\n");
			WriteLog(szTempBuf);
			break;
        case RASCS_PortOpened:
            sprintf (szTempBuf,"RASCS_PortOpened = %d\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Port opened.\r\n");
			WriteLog(szTempBuf);
        	break;
        case RASCS_ConnectDevice: 
            sprintf (szTempBuf,"RASCS_ConnectDevice = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Connecting device...\r\n");
			WriteLog(szTempBuf);
           break;
        case RASCS_DeviceConnected: 
            sprintf (szTempBuf,"RASCS_DeviceConnected = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Device connected.\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AllDevicesConnected:
            sprintf (szTempBuf,"RASCS_AllDevicesConnected = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"All devices connected.\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_Authenticate: 
            sprintf (szTempBuf,"RASCS_Authenticate = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Authenticating...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AuthNotify:
            sprintf (szTempBuf,"RASCS_AuthNotify = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Authentication notify.\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AuthRetry: 
            sprintf (szTempBuf,"RASCS_AuthRetry = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Retrying authentication...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AuthCallback:
            sprintf (szTempBuf,"RASCS_AuthCallback = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Authentication callback...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AuthChangePassword: 
            sprintf (szTempBuf,"RASCS_AuthChangePassword = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Change password...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AuthProject: 
            sprintf (szTempBuf,"RASCS_AuthProject = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Projection phase started...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AuthLinkSpeed: 
            sprintf (szTempBuf,"RASCS_AuthLinkSpeed = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Negoting speed...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_AuthAck: 
            sprintf (szTempBuf,"RASCS_AuthAck = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Authentication acknowledge...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_ReAuthenticate: 
            sprintf (szTempBuf,"RASCS_ReAuthenticate = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Retrying Authentication...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_Authenticated: 
            sprintf (szTempBuf,"RASCS_Authenticated = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Authentication complete.\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_PrepareForCallback: 
            sprintf (szTempBuf,"RASCS_PrepareForCallback = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Preparing for callback...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_WaitForModemReset: 
            sprintf (szTempBuf,"RASCS_WaitForModemReset = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Waiting for modem reset...\r\n");
			WriteLog(szTempBuf);

            break;
        case RASCS_WaitForCallback:
            sprintf (szTempBuf,"RASCS_WaitForCallback = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Waiting for callback...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_Projected:  
            sprintf (szTempBuf,"RASCS_Projected = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Projection completed.\r\n");
			WriteLog(szTempBuf);
            break;
    #if (WINVER >= 0x400) 
        case RASCS_StartAuthentication:    // Windows 95 only 
            sprintf (szTempBuf,"RASCS_StartAuthentication = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Starting authentication...\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_CallbackComplete:       // Windows 95 only 
            sprintf (szTempBuf,"RASCS_CallbackComplete = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Callback complete.\r\n");
			WriteLog(szTempBuf);
            break;
        case RASCS_LogonNetwork:           // Windows 95 only 
            sprintf (szTempBuf,"RASCS_LogonNetwork = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Login to the network.\r\n");
			WriteLog(szTempBuf);
            break;
    #endif 
        case RASCS_SubEntryConnected:
		
            sprintf (szTempBuf,"RASCS_SubEntryConnected = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Subentry connected.\r\n");
			WriteLog(szTempBuf);

            break;
        case RASCS_SubEntryDisconnected:
		
            sprintf (szTempBuf,"RASCS_SubEntryDisconnected = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Subentry disconnected.\r\n");
			WriteLog(szTempBuf);
            break;
		//PAUSED STATES:
		case RASCS_Interactive:
            sprintf (szTempBuf,"RASCS_Interactive = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"In Paused state: Interactive mode.\r\n");
			WriteLog(szTempBuf);
            break;
		case RASCS_RetryAuthentication:
            sprintf (szTempBuf,"RASCS_RetryAuthentication = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"In Paused state: Retry Authentication...\r\n");
			WriteLog(szTempBuf);
            break;
		case RASCS_CallbackSetByCaller:
            sprintf (szTempBuf,"RASCS_CallbackSetByCaller = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"In Paused state: Callback set by Caller.\r\n");
			WriteLog(szTempBuf);
            break;
		case RASCS_PasswordExpired:
			gbRotating = FALSE;
            sprintf (szTempBuf,"RASCS_PasswordExpired = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"In Paused state: Password has expired...\r\n");
			WriteLog(szTempBuf);
            break;
 
        case RASCS_Connected: // = RASCS_DONE: 
			gbRotating = FALSE;
            sprintf (szTempBuf,"RASCS_Connected = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Connection completed.\r\n");
			WriteLog(szTempBuf);
//			SetEvent(gEvent_handle);
            break;
        case RASCS_Disconnected: 
			gbRotating = FALSE;
            sprintf (szTempBuf,"RASCS_Disconnected = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
            sprintf (szTempBuf,"Disconnecting...\r\n");
			WriteLog(szTempBuf);
            break;
		default:
			sprintf (szTempBuf,"Unknown Status = %d\r\n", rasconnstate);
			WriteLog(szTempBuf);
			sprintf (szTempBuf,"What are you going to do about it?\r\n");
			WriteLog(szTempBuf);
			break;
   }
} 
int Dial(const char * szEntryName)
{
    LPRASDIALPARAMS lpRasDialParams = NULL;   // Structure to store the RasDial parameters
    HRASCONN        hRasConn = NULL;          // Handle to RAS connection
    DWORD           nRet = 0;                 // Return value from a function
	BOOL			bPassword;
	 // Initialize the RASDIALPARAMS structure
    lpRasDialParams = (LPRASDIALPARAMS) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASDIALPARAMS));
    if (NULL == lpRasDialParams)
    {
	    printf("HeapAlloc failed\r\n");
		return -1;
    }
	gszLogData[0] = 0;
	strcpy_s(lpRasDialParams->szEntryName, szEntryName);
    lpRasDialParams->dwSize =sizeof(RASDIALPARAMS);
	RasGetEntryDialParams(NULL, lpRasDialParams, &bPassword);
	nRet = RasDial(NULL, NULL, lpRasDialParams, 0, &RasDialFunc, &hRasConn);
	return nRet;
}
#define MAX_BUFFER 256
void ReadDefaultPhonebookEntry()
{
	TCHAR path[MAX_PATH];



	GetModuleFileName(NULL,path,sizeof(path));
	if (strlen(path) == 0)
		return;
	
	TCHAR * p = path + strlen(path) -1;
	while( *p != '\\' && p > path) {
		p--;
	}
	memset(gszIniFilePath, 0, sizeof(gszIniFilePath));
	strncpy(gszIniFilePath, path, p-path+1);

	strcat_s(gszIniFilePath, INI_FILE_NAME);
	FILE * fp = fopen(gszIniFilePath, "r");
	TCHAR buf[MAX_BUFFER];
	if (fp == NULL) {
		fp = fopen(gszIniFilePath, "w");
		PHONE_ENTRY phoneEntries[MAX_PHONE_ENTRY];
		int count = GetPhoneBookEntries(phoneEntries);
		if (count > 0) {
			strcpy_s(gszDefaultEntryName, phoneEntries[0].entryName);
			fputs(gszDefaultEntryName,fp);
		}
		fclose(fp);
	}
	else {
		char * pRet = fgets(buf, MAX_BUFFER, fp);
		if (pRet != NULL) {
			strcpy_s(gszDefaultEntryName, buf);
		}
	}
}
void WriteDefaultPhonebookEntry()
{
	FILE * fp = fopen(gszIniFilePath, "w");
	if (fp != NULL) {
		fputs(gszDefaultEntryName,fp);
		fclose(fp);
	}
}