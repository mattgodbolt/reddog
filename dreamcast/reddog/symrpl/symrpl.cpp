// SymRpl.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SymRpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		if (argc != 5) {
			cerr << _T("Usage: symrpl mapfile elffile sourcesymbol destsymbol") << endl;
			nRetCode = 1;
		} else {
			CStdioFile mapFile, mapOutFile;
			CFile elfFile;
			CString SourceSymbol = argv[3];
			CString DestSymbol = argv[4];
			int SLen = strlen (argv[3]), DLen = strlen(argv[4]);
			unsigned long SourceAddress = 0, DestAddress = 0;

			// Find the symbol addressess
			mapFile.Open (argv[1], CFile::modeRead | CFile::shareDenyNone);
			do {
				CString string;
				if (!mapFile.ReadString (string))
					break;
				if (SourceSymbol == string.Left (SLen)) {
					// The next word is the hex address
					int find = string.Find ("H'");
					sscanf (string.Mid (find, string.GetLength()), "H'%x", &SourceAddress);
					// Rewrite the output
					string.Format("%sH'%x%s", string.Left(find), DestAddress, string.Mid (find + 10));
				} else if (DestSymbol == string.Left (DLen)) {
					// The next word is the hex address
					int find = string.Find ("H'");
					sscanf (string.Mid (find, string.GetLength()), "H'%x", &DestAddress);
					// Deliberately don't output the destination
				}
			} while (1);

			mapFile.Close();

			if (SourceAddress == 0 || DestAddress == 0) {
				cerr << "Unable to find symbols" << endl;
				nRetCode = 1;
			} else {
				
				// Now to go back and rewrite the source
				mapFile.Open (argv[1], CFile::modeRead | CFile::shareDenyNone);
				mapOutFile.Open ("C:\\TEMP.LST", CFile::modeCreate | CFile::modeWrite);
				do {
					CString string;
					if (!mapFile.ReadString (string))
						break;
					if (SourceSymbol == string.Left (SLen)) {
						// The next word is the hex address
						int find = string.Find ("H'");
						// Rewrite the output
						string.Format("%sH'%x%s", string.Left(find), DestAddress, string.Mid (find + 10));
						mapOutFile.WriteString (string);
						mapOutFile.WriteString ("\n");
					} else if (DestSymbol == string.Left (DLen)) {
						// The next word is the hex address
						int find = string.Find ("H'");
						sscanf (string.Mid (find, string.GetLength()), "H'%x", &DestAddress);
						// Deliberately don't output the destination
					} else {
						mapOutFile.WriteString (string);
						mapOutFile.WriteString ("\n");
					}
				} while (1);
				mapFile.Close();
				
				mapOutFile.Close();
				
				// Replace the .lst file
				unlink (argv[1]);
				rename ("C:\\TEMP.LST", argv[1]);

				// Now replace and suchlike
				elfFile.Open (argv[2], CFile::modeRead);
				int length = elfFile.GetLength();
				char *buffer = new char[length];
				elfFile.Read (buffer, length);
				elfFile.Close();
				
				// Scan for the lower byte
				char *ptr = buffer;
				int bytesLeft = length;
				do {
					if (*(unsigned long *)ptr == SourceAddress) {
						*(unsigned long *)ptr = DestAddress;
					}
					ptr++;
				} while (bytesLeft--);
				
				elfFile.Open (argv[2], CFile::modeCreate | CFile::modeWrite);
				elfFile.Write (buffer, length);
				
				delete[]buffer;
			}
		}
	}

	return nRetCode;
}


