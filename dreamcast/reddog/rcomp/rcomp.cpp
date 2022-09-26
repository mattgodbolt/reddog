// RComp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RComp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

static int nWritten = 0;
static void Write (CStdioFile &out, int i)
{
	CString str;
	str.Format ("0x%02x, 0x%02x, 0x%02x, 0x%0x2, ", i&0xff, (i>>8)&0xff, (i>>16)&0xff,(i>>24)&0xff);
	out.WriteString (str);
	nWritten+=4;
	if (nWritten >= 16) {
		nWritten-= 16;
		out.WriteString ("\n");
	}
}

static void Write (CStdioFile &out, const char *c, int nBytes = -1)
{
	CString str;
	const char *s;
	if (nBytes == -1)
		nBytes = strlen (c) + 1;
	for (s = c; nBytes; ++s, --nBytes) {
		str.Format ("0x%02x, ", *c);
		out.WriteString (str);
		nWritten++;
		if (nWritten >= 16) {
			nWritten-= 16;
			out.WriteString ("\n");
		}
	}
}

static CString Shift(CString &str)
{
	CString retVal;
	int i;
	i = str.Find(' ');
	if (i == -1) {
		retVal = str;
		str = "";
		return retVal;
	}
	retVal = str.Left(i);
	str = str.Mid (i+1);
	return retVal;
}

struct MemChunk
{
	MemChunk *next;
	int nBytes;
	char *data;
};

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
		if (argc != 4) {
			cerr << "Shit!" << endl;
			return 1;
		}
		CStdioFile file;
		CStdioFile out, out2;
		if (!file.Open (argv[1], CFile::modeRead | CFile::shareDenyNone))
		{
			cerr << "Arse!" << endl;
			return 1;
		}
		if (!out.Open (argv[2], CFile::modeWrite | CFile::modeCreate))
		{
			cerr << "Arse!" << endl;
			return 1;
		}
		if (!out2.Open (argv[3], CFile::modeWrite | CFile::modeCreate))
		{
			cerr << "Arse!" << endl;
			return 1;
		}

		out.WriteString ("#include \"RedDog.h\"\n");
		out.WriteString ("#include \"");
		out.WriteString (argv[3]);
		out.WriteString ("\"\n\n");
		out2.WriteString ("#ifndef _RCOMP_CREATED\n");
		out2.WriteString ("#define _RCOMP_CREATED\n");
		int dialog = -1;
		MemChunk *controls = NULL;
		
		do {
			CString command;
			if (!file.ReadString (command))
				break;
			command.TrimLeft();
			if (command.GetLength() == 0 || command[0] == '#')
				continue;
			if (isdigit(command[0])) {
				// Beginning of a new dialog
				ASSERT (dialog == -1);
				CString tok = Shift (command);
				dialog = atoi(tok);
				CString newDialog;
				newDialog.Format ("char FEDialog_%d[] = {\n", dialog);
				out.WriteString (newDialog);
				newDialog.Format ("extern char FEDialog_%d[];\n", dialog);
				out2.WriteString (newDialog);
				Shift(command)
			} else {
				CString word = Shift(command);
				if (!word.CompareNoCase ("STYLE") ||
					!word.CompareNoCase ("CAPTION") ||
					!word.CompareNoCase ("FONT") ||
					!word.CompareNoCase ("BEGIN"))
					continue;
				if (!word.CompareNoCase("END")) {
					MemChunk *c, *cNext;
					Write (out, dialog);
					int nCtrls = 0;
					for (c = controls; c ; c = c->next)
						nCtrls++;
					Write (out, nCtrls);
					for (c = controls; c ; c = cNext) {
						cNext = c->next;
						Write (out, c->data, c->nBytes);
						delete c->data;
						delete c;
					}
					dialog = -1;
					out.WriteString ("};\n\n");
				}
				if (!word.CompareNoCase("PUSHBUTTON")) {
				}
			}
		} while (1);
		file.Close();

		ASSERT (dialog == -1);
		out.Close();
		out2.WriteString ("#endif\n\n");
		out2.Close();
	}

	return nRetCode;
}


