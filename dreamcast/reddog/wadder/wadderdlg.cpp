// WadderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Wadder.h"
#include "WadderDlg.h"
#include "WadFile.h"
#include "TexLib.h"
#include "TexRec.h"
#include "RedDog.h"
#include <ctype.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool LoadNewCP = false;
static bool FullWad = false;

#ifdef _DEBUG
#define LogV Log
#else
#define LogV(message) (void)(0)
#endif

#define TEXRAM_AMOUNT		(4.4f) // paranoia and PAL rolled into one

ObjectHeader objH;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWadderDlg dialog

CWadderDlg::CWadderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWadderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWadderDlg)
	m_Log = _T("");
	m_WentOK = _T("");
	m_TexRamAmt = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWadderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWadderDlg)
	DDX_Control(pDX, IDC_TEXPROG, m_TexProgress);
	DDX_Control(pDX, IDC_LOG, m_LogCtrl);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDCANCEL, m_Cancel);
	DDX_Text(pDX, IDC_LOG, m_Log);
	DDX_Text(pDX, IDC_WENTOK, m_WentOK);
	DDX_Text(pDX, IDC_TEXRAMAMT, m_TexRamAmt);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWadderDlg, CDialog)
	//{{AFX_MSG_MAP(CWadderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_COPYDATA,	OnLogMessage)
    ON_MESSAGE(WM_USER+1,	OnTexRAMUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWadderDlg message handlers
#if ARTIST
#define WADDIR "D:\\DREAMCAST\\WADS\\"
#else
#if GODMODEMACHINE
#define WADDIR "D:\\DREAMCAST\\WADS\\"
#else
#define WADDIR "C:\\DREAMCAST\\WADS\\"
#endif
#endif
#define COMMAND_FILE WADDIR "WAD.TXT"

BOOL CWadderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_OK.EnableWindow (FALSE);

#if 0
	CWorker::Worker ((void *)m_hWnd);
#else
	AfxBeginThread (CWorker::Worker, (void *)m_hWnd);
#endif
	m_TexProgress.SetRange (0, (short)(TEXRAM_AMOUNT * 1024.f));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWadderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWadderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWadderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// Log a message
void CWadderDlg::Log (CString message)
{
	OutputDebugString (message);
	OutputDebugString ("\r\n");
	m_Log += message + "\r\n";
	if (message == "Done.") {
		m_OK.EnableWindow();
		if (SetTimer (0, 3500, NULL)) {
			m_WentOK = "WAD created OK";
		}
	}
	UpdateData(FALSE);
}

LRESULT CWadderDlg::OnLogMessage (WPARAM wParam, LPARAM lParam)
{
	COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;
	Log ((char *)cds->lpData);
	return 0;
}

LRESULT CWadderDlg::OnTexRAMUpdate (WPARAM wParam, LPARAM lParam)
{
	int totBytes = (int) wParam;
	m_TexRamAmt.Format ("%.2fMb (%d%%)", (float)totBytes / (1024.f*1024.f), (int)(100.f * totBytes / (TEXRAM_AMOUNT * 1024 * 1024)));
	m_TexProgress.SetPos (totBytes/1024);
	UpdateData (FALSE);
	return 0;
}
struct LibSort
{
	CString name;
	int size;
};
static int sorter(const void *v0, const void *v1)
{
	return (((LibSort*)v1)->size - ((LibSort*)v0)->size);
}
void CWorker::OutputTexLib (WadFile &output,
							TexLib &library)
{
	int nTextures = library.Count();
	int nBytes, totBytes, thisTex;
	char buffer[256*1024];
	output.Write (&nTextures, sizeof (nTextures));
	output.EndChunk(); // End the texture count chunk
	static int baseLib = 0;

	// Read the size of each texture in turn
	CArray<LibSort, LibSort> array;
	library.Reset();
	while (library.More()) {
		LibSort s;
		CString texName = "P:\\GAME\\TEXMAPS\\" + library.GetElement() + ".PVR";
		CFileStatus status;
		CFile::GetStatus (texName, status);

		s.name = texName;
		s.size = status.m_size;
		array.Add(s);
	}

	// Sort based on file size
	totBytes = 0;
	if (array.GetSize()) {
		qsort (&array[0], array.GetSize(), sizeof (LibSort), sorter);
		
		// Now output each texture in turn
		//	library.Reset();
		//	while (library.More())
		for (int texture = 0; texture < array.GetSize(); ++texture)
		{
			bool vq, anim;
			int animFrames;
			//		CString texName = library.GetElement();
			//		CFile texfile ("P:\\GAME\\TEXMAPS\\" + texName + ".PVR", CFile::modeRead | CFile::shareDenyNone);
			CFile texfile (array[texture].name, CFile::modeRead | CFile::shareDenyNone);
			thisTex = 0;
			anim = false;
			do {
				nBytes = texfile.Read (&buffer, sizeof (buffer));
				if (thisTex == 0) {
					if (!memcmp (buffer, "ANIM", 4)) {
						vq = (buffer[0x15+0x24] & 0x40)?true:false;
						anim = ((*(char *)(buffer + 0xc)) == 1) ? true : false;
						animFrames = *((int *)&buffer[8]);
					} else {
						vq = (buffer[0x15] & 0x40)?true:false;
					}
				}
				thisTex += nBytes;
				if (nBytes)
					output.Write (&buffer, nBytes);
			} while (nBytes);
			if (anim && animFrames /* offset */) {
				totBytes += ((thisTex/animFrames)+255) & ~255;
			} else
				totBytes += (thisTex+255) & ~255;
			CString string;
			string.Format("* Outputting texture '" + array[texture].name + "'" + " is %d bytes (%sVQ)", thisTex, vq? "non-" : "");
			LogV (string);
			output.EndChunk();
			texfile.Close();
			::PostMessage (window, WM_USER+1, (WPARAM) (totBytes + baseLib), NULL);
		}
	}
	CString logMes;
	logMes.Format ("* Texture RAM needed: %.2fMb", (float)totBytes / (1024.f*1024.f));
	Log (logMes);

	int residentSize = totBytes;

	if (baseLib == 0 && FullWad)
		baseLib = totBytes;
	else
		residentSize += baseLib;

	// Write the size out
	output.Write (&totBytes, 4);

	if (residentSize > (TEXRAM_AMOUNT*1024*1024)) {
		DWORD d = GetTickCount();
		MessageBox (NULL, "Texture RAM overflowed", "Ooh an annoying warning!", MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_SYSTEMMODAL);
		while ((GetTickCount() - d) < 4*1000) {
			d = GetTickCount();
			MessageBox (window, "No, I mean it.  The texture RAM overflowed and you can't ignore it!  Oh impatient one, it takes about five seconds to read the message, right?", "Ooh an even more annoying warning!", 
				MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_SYSTEMMODAL);
		}
	}
}

UINT CWorker::Worker (void *w)
{
	CWorker worker ((HWND)w);
	return worker.Go();
}

CWorker::CWorker (HWND w)
{
	window = w;
}

CString CWorker::Tokenise (CString &ref)
{
	CString com;
	int idx = ref.FindOneOf ("\n\r\t ");
	if (idx == -1)
		idx = ref.GetLength();
	com = ref.Left (idx);
	ref = ref.Mid (idx, ref.GetLength()-idx);
	ref.TrimLeft();
	return com;
}

int CWorker::AddRDO (CString string, WadFile &wad, TexLib &wadTexList)
{
	CString palString;
	CFile file, pal;
	bool hasPal;
	LogV ("Reading object " + string);
	int amount;
	char buffer[256*1024];

	if (!file.Open (string, CFile::modeRead | CFile::shareDenyNone)) {
		CString logger;
		logger.Format("Unable to open input file '%s'!", string);
		Log (logger);
		return 1;
	}
	
	palString = string.Left (string.GetLength()-4) + "_PAL.RDO";
	if (pal.Open (palString, CFile::modeRead | CFile::shareDenyNone)) {
		hasPal = true;
		Log ("Found PAL animation");
	} else
		hasPal = false;
	
	// If we don't have a pal fork, then store a zero length skip
	amount = 0;
	wad.Mark();	// Mark here as the place to store the data
	wad.Write (&amount, 4);

	try {
		if (file.Read (&objH, sizeof (objH)) != sizeof (objH)) {
			Log ("Unable to read model header!");
			return 1;
		}
	} catch (CFileException *p)
	{
		p->Delete();
		Log ("Unable to read model!");
		return 1;
	}
	if (objH.tag != OBJECTHEADER_TAG) {
		Log ("Invalid object");
		return 1;
	}
	// Convert old CP data to new
	LoadNewCP = (objH.version >= 0x190);

	if (!LoadNewCP)
		objH.version = 0x190;
	// Copy out the header
	wad.Write (&objH, sizeof (objH));
	
	// Recursively load the objects, copying them to the wad as we go
	// and adding textures to the library
	if (!RecurseReadObj (file, wad, wadTexList))
		return 1;
	
	// Copy any remaining data
	int nBytes;
	do {
		nBytes = file.Read (&buffer, sizeof (buffer));
		if (nBytes)
			wad.Write (&buffer, nBytes);
	} while (nBytes);

	if (hasPal) {
		wad.WriteMark(); // mark that madness in
	}
	
	// End the map chunk
	wad.EndChunk();
	
	file.Close();

	// Deal with any pal fork
	if (hasPal) {
		wad.Mark();
		wad.Write (&amount, 4);
		try {
			if (pal.Read (&objH, sizeof (objH)) != sizeof (objH)) {
				Log ("Unable to read model header!");
				return 1;
			}
		} catch (CFileException *p)
		{
			p->Delete();
			Log ("Unable to read model!");
			return 1;
		}
		if (objH.tag != OBJECTHEADER_TAG) {
			Log ("Invalid object");
			return 1;
		}
		// Convert old CP data to new
		LoadNewCP = (objH.version >= 0x190);
		
		if (!LoadNewCP)
			objH.version = 0x190;
		// Copy out the header
		wad.Write (&objH, sizeof (objH));
		
		// Recursively load the objects, copying them to the wad as we go
		// and adding textures to the library
		if (!RecurseReadObj (pal, wad, wadTexList))
			return 1;
		
		// Copy any remaining data
		int nBytes;
		do {
			nBytes = pal.Read (&buffer, sizeof (buffer));
			if (nBytes)
				wad.Write (&buffer, nBytes);
		} while (nBytes);
	
		wad.WriteMark();
		
		// End the map chunk
		wad.EndChunk();
		
		pal.Close();
	}

	return 0;
}

UINT CWorker::Go (void)
{
	bool doneSystem = false;
	CString string;
	CStdioFile command;
	WadFile wad, texWad;
	TexLib wadTexList;

	// Are we doing a full WAD, or just a single file?
	extern CWadderApp theApp;
	if (theApp.m_lpCmdLine[0]) {
		Log ("*** Full WAD ***");
		FullWad = true;
	} else {
		TexLib::DoneSystem = true;
	}
	if (!command.Open(COMMAND_FILE, CFile::modeRead | CFile::shareDenyNone)) {
		Log ("! Unable to open command file!");
		return 1;
	}
	bool SkipCommands = false;
	do {
		if (!command.ReadString (string))
			break;
		string.TrimLeft();
		if (string.Left(1) == '#' || string == "")
			continue;
		// Find the command word
		CString com;
		com = Tokenise (string);
		com.MakeUpper();
		if (SkipCommands) {
			if (com == "ENDIFFULL")
				SkipCommands = false;
			continue;
		}
		if (com == "CREATEWAD") {
			CString filename, name = Tokenise(string);
			filename = WADDIR + name + ".LVL";
			wad.Open (filename);
			filename = WADDIR + name + ".TXP";
			texWad.Open (filename);
			wadTexList.Clear();
			Log ("* Creating " + string);
		} else if (com == "IFFULL") {
			if (!FullWad) {
				SkipCommands = true;
			}
		} else if (com == "ENDIFFULL") {
			SkipCommands = false;
		} else if (com == "ADDFILE") {
			LogV ("* Adding file " + string);
			CFile mapFile;
			if (!mapFile.Open (string, CFile::modeRead | CFile::shareDenyNone)) {
				Log ("Unable to open '" + string + "'");
				return 1;
			}
			char buffer[256*1024];
			int nBytes;
			do {
				nBytes = mapFile.Read (&buffer, sizeof (buffer));
				if (nBytes)
					wad.Write (&buffer, nBytes);
			} while (nBytes);
			wad.EndChunk();
			mapFile.Close();			
		} else if (com == "ADDPVR") {
			LogV ("* Adding texture " + string);
			Uint32 gbix;
			getGBIX(string, gbix);
			wadTexList.Add (gbix, string);
		} else if (com == "CLOSEWAD") {
			CString logMes;
			logMes.Format ("* Wad has %d unique texture%s", wadTexList.Count(), wadTexList.Count()==1 ? "" : "s");
			Log (logMes);
			if (!doneSystem) {
				doneSystem = true;
				wadTexList.SetAsSystem();
			}
			if (wadTexList.invalidTextures) {
				CString string;
				string.Format("Unable to create WAD - %d textures need converting");
				Log (string);
				return 1;
			}
			try {
				OutputTexLib (texWad, wadTexList);
			} catch (CFileException *e) {
				Log("* Missing file '" + CString(e->m_strFileName) +"' !!!!");
				e->Delete();
				return 1;
			}
			wad.Close();
			texWad.Close();
			Log ("* Closing WAD");
			Log ("");
		} else if (com == "ADDMAP") {
			// Map data
			CString mapName = "C:\\STRATS\\MAP\\" + string + ".MAP";
			CFile mapFile;
			if (!mapFile.Open (mapName, CFile::modeRead | CFile::shareDenyNone)) {
				Log ("Unable to open strat map");
				return 1;
			}
			
			// Now to copy the stratmapfile verbatim
			char buffer[256*1024];
			int nBytes;
			do {
				nBytes = mapFile.Read (&buffer, sizeof (buffer));
				if (nBytes)
					wad.Write (&buffer, nBytes);
			} while (nBytes);
			wad.EndChunk();
			mapFile.Close();			
			
			// Strat object list
			CString modList;
			modList = "C:\\STRATS\\MAP\\" + string + ".MOD";
			LogV ("Processing model list " + modList);
			CStdioFile modListFile;
			if (!modListFile.Open (modList, CFile::modeRead | CFile::shareDenyNone)) {
				Log ("Unable to open model list");
				return 1;
			}
			CString line;
			modListFile.ReadString (line);
			int numModels = atoi (line);
			wad.Write (&numModels, 4);
			while (modListFile.ReadString(line)) {
				if (AddRDO (line, wad, wadTexList))
					return 1;
				modListFile.ReadString (line);
				int foo = atoi (line);
				wad.Write (&foo, 4);
			}
			modListFile.Close();

			// Check for the NFO file
			CString nfoName = "C:\\STRATS\\MAP\\" + string + ".NFO";
			LogV ("Processing NFO file " + nfoName);
			CStdioFile nfoFile;
			if (!nfoFile.Open (nfoName, CFile::modeRead | CFile::shareDenyNone)) {
				Log ("Unable to open NFO file");
				return 1;
			}
			int nFile = 0;
			while (nfoFile.ReadString(line)) {
				nFile++;
				CFile xtraFile;
				if (!xtraFile.Open (line, CFile::modeRead | CFile::shareDenyNone)) {
					CString logMes;
					logMes.Format ("Skipping file '%s'", line);
					LogV(logMes);
					int fileSize = 0;
					wad.Write (&fileSize, 4);
					wad.EndChunk();
					continue;
				}

				// Write the file size
				int fileSize = xtraFile.GetLength();
				wad.Write (&fileSize, 4);

				CString logMes;

				switch (nFile) {
				case 1: /* Dome! */
					logMes.Format ("Adding file '%s' which is %d bytes long - treating as a DOME", line, fileSize);
					LogV(logMes);
					xtraFile.Close();
					if (AddRDO (line, wad, wadTexList))
						return 1;
					break;
				case 5:
				case 6:
				case 7:
				case 8:
					logMes.Format ("Adding file '%s' which is %d bytes long - treating as a DOME", line, fileSize);
					LogV(logMes);
					xtraFile.Close();
					if (AddRDO (line, wad, wadTexList))
						return 1;
					break;
				
				
				default: /* Anything else: copy it byte for byte */
					logMes.Format ("Adding file '%s' which is %d bytes long", line, fileSize);
					LogV(logMes);
					do {
						nBytes = xtraFile.Read (&buffer, sizeof (buffer));
						if (nBytes)
							wad.Write (&buffer, nBytes);
					} while (nBytes);
					xtraFile.Close();
					break;
				}
				wad.EndChunk();
			}
			nfoFile.Close();
			
			// Landscape processing
			string = "P:\\GAME\\NEWSCAPES\\" + string + ".RDL";
			Log ("Preparing to process landscape '" + string + "' ...");
			CFile file;
			
			if (!file.Open (string, CFile::modeRead | CFile::shareDenyNone)) {
				CString logger;
				logger.Format("Unable to open input file '%s'!", string);
				Log (logger);
				return 1;
			}
			
			Map map;
			try {
				if (file.Read (&map, sizeof (map)) != sizeof (map)) {
					Log ("Truncated map");
					return 1;
				}
			} catch (CFileException *p)
			{
				p->Delete();
				Log ("Unable to read map!");
				return 1;
			}
			
			if (map.header.tag != MAPHEADER_TAG) {
				Log ("That's not a map!");
				return 1;
			}
			
			CString logMes;
			logMes.Format ("Map has %d boxes and %d materials", 
				map.numBoxes, 
				map.numMaterials);
			Log (logMes);
			
			// Write out the map to the wadded stream
			wad.Write (&map, sizeof (map));
						
			Uint32 mat;
			for (mat = 0; mat < map.numMaterials; ++mat) {
				char texName[256], pasteName[256];
				Material material;
				file.Read (&material, sizeof (material));
				/* Patch the flags */
				if ((material.flags & MF_MIPMAP_ADJUST_MASK) == 0)
					material.flags |= MF_MIPMAP_SET(4);
				if (material.surf.surfaceDesc) {
					file.Read (texName, (int)material.surf.surfaceDesc);
					Uint32 wadGBIX;
					Uint32 GBIX = getGBIX (texName, wadGBIX);
					material.surf.surfaceDesc = (void *)GBIX;
					CString str;
					str.Format ("* Texture '%s' has GBIX 0x%x", texName, GBIX);
					LogV (str);
//					wadTexList.Add (wadGBIX, CString (texName));
					wadTexList.Add (GBIX, CString (texName));
				}
				if ((material.pasted.flags & MF_MIPMAP_ADJUST_MASK) == 0)
					material.pasted.flags |= MF_MIPMAP_SET(4);
				if (material.pasted.surf.surfaceDesc) {
					file.Read (pasteName, (int)material.pasted.surf.surfaceDesc);
					Uint32 wadGBIX;
					Uint32 GBIX = getGBIX (pasteName, wadGBIX);
					CString str;
					material.pasted.surf.surfaceDesc = (void *)GBIX;
					str.Format ("* Pasted texture '%s' has GBIX 0x%x", pasteName, GBIX);
					LogV (str);
//					wadTexList.Add (wadGBIX, CString (pasteName));
					wadTexList.Add (GBIX, CString (pasteName));
				}
				// wad the compacted material
				wad.Write (&material, sizeof (material));
			}
			
			// Now to copy the rest of the mapfile verbatim
			do {
				nBytes = file.Read (&buffer, sizeof (buffer));
				if (nBytes)
					wad.Write (&buffer, nBytes);
			} while (nBytes);
			
			// End the map chunk
			wad.EndChunk();

			file.Close();
		} else if (com == "ADDOBJECT") {
			string = "P:\\GAME\\NEWOBJECTS\\" + string + ".RDO";
			if (AddRDO (string, wad, wadTexList))
				return 1;
		} else {
			Log ("! Unknown command '" + com + "' (" + string + ")");
		}
	} while (1);

	Log ("Done.");

	AfxDaoTerm();

	return 0;
}

// Recursively snarf an object
bool CWorker::RecurseReadObj (CFile &file, WadFile &wad, TexLib &wadTexList)
{
	Object obj;
	if (file.Read (&obj, sizeof (obj)) != sizeof (obj)) {
		Log ("Truncated object");
		return false;
	}
	// Copy to the wad
	wad.Write (&obj, sizeof (obj));
	// Deal with any cps
	if (obj.ncp != 0) {
		if (LoadNewCP) {
			CP1 *temp = (CP1 *)malloc (sizeof(CP1) * obj.ncp);
			file.Read (temp, sizeof(CP1) * obj.ncp);
			wad.Write (temp, sizeof(CP1) * obj.ncp);
			free (temp);
		} else {
			// Old format - convert to new
			int i;
			CP1 temp;
			temp.radius = 0.f;
			for (i = 0; i < obj.ncp; ++i) {
				file.Read (&temp, sizeof(Point3));
				wad.Write (&temp, sizeof(CP1));
			}
		}
	}
	// Deal with the model
	if (obj.model) {
		ModelHeader modelH;
		Model model;
		file.Read (&modelH, sizeof (modelH));
		if (modelH.tag != MODELHEADER_TAG) {
			Log ("Corrupt model");
			return false;
		}
		wad.Write (&modelH, sizeof (modelH));

		// Hacking for bump map
		if (modelH.version < 0x140) {
			file.Read (&model, sizeof (model) - 8);
			model.bump = NULL;
			model.modelFlags = 0;
		} else if (modelH.version < 0x150) {
			file.Read (&model, sizeof (model) - 4);
			model.modelFlags = 0;
		} else {
			file.Read (&model, sizeof (model));
		}
		if (objH.version < 0x192)
			model.bump = NULL;
		wad.Write (&model, sizeof (model));

		// Materials next
		for (int mat = 0; mat < model.nMats; ++mat) {
			char texName[256], pasteName[256];
			Material material;
			file.Read (&material, sizeof (material));
			if ((material.flags & MF_MIPMAP_ADJUST_MASK) == 0)
				material.flags |= MF_MIPMAP_SET(4);
			if (material.surf.surfaceDesc) {
				file.Read (texName, (int)material.surf.surfaceDesc);
				Uint32 wadGBIX;
				Uint32 GBIX = getGBIX (texName, wadGBIX);
				material.surf.surfaceDesc = (void *)GBIX;
				CString str;
				str.Format ("* Texture '%s' has GBIX 0x%x", texName, GBIX);
				LogV (str);
//				wadTexList.Add (wadGBIX, CString (texName));
				wadTexList.Add (GBIX, CString (texName));
			}
			if ((material.pasted.flags & MF_MIPMAP_ADJUST_MASK) == 0)
				material.pasted.flags |= MF_MIPMAP_SET(4);
			if (material.pasted.surf.surfaceDesc) {
				file.Read (pasteName, (int)material.pasted.surf.surfaceDesc);
				Uint32 wadGBIX;
				Uint32 GBIX = getGBIX (pasteName, wadGBIX);
				CString str;
				material.pasted.surf.surfaceDesc = (void *)GBIX;
				str.Format ("* Pasted texture '%s' has GBIX 0x%x", pasteName, GBIX);
				LogV (str);
//				wadTexList.Add (wadGBIX, CString (pasteName));
				wadTexList.Add (GBIX, CString (pasteName));
			}
			// wad the compacted material
			wad.Write (&material, sizeof (material));
		}

		// Vertices next
		Point3 *p = (Point3 *)malloc (sizeof (Point3) * model.nVerts);
		file.Read (p, sizeof (Point3) * model.nVerts);
		wad.Write (p, sizeof (Point3) * model.nVerts);
		free (p);

		// Vertex normals
		Vector3 *v = (Vector3 *)malloc (sizeof (Vector3) * model.nVerts);
		file.Read (v, sizeof (Vector3) * model.nVerts);
		wad.Write (v, sizeof (Vector3) * model.nVerts);
		free (v);

		int nPolys = (model.nQuads + model.nTris + model.nStripPolys);
		int nEdges = (model.nQuads * 4 + model.nTris * 3 + model.nStrips * 2 + model.nStripPolys);

		// Plane equations
		if (model.plane) {
			Plane *pl = (Plane *)malloc (sizeof (Plane) * nPolys);
			file.Read (pl, sizeof (Plane) * nPolys);
			wad.Write (pl, sizeof (Plane) * nPolys);
			free (pl);
		}
		// UV Values
		UV *uv = (UV *)malloc (sizeof (UV) * nEdges);
		file.Read (uv, sizeof (UV) * nEdges);
		wad.Write (uv, sizeof (UV) * nEdges);
		free (uv);

		// Bump map warez
		if (model.bump) {
			BumpMapInfo *bump = (BumpMapInfo *)malloc (sizeof (BumpMapInfo) * model.nVerts);
			file.Read (bump, sizeof (BumpMapInfo) * model.nVerts);
			wad.Write (bump, sizeof (BumpMapInfo) * model.nVerts);
			free (bump);
		}

		// Check for 'old' models
		if (model.nTris || model.nQuads) {
			Log (CString ("* Old format model '") + file.GetFileName() + CString ("' - could do with re-exporting"));
		}
		// Now to deal with the strip info
		int temp = (int)model.strip;
#if 0
		int tri = model.nTris;
		while (tri) {
			// Read this 'strip' info
			Uint16 material, nInStrip;
			file.Read (&material, 2);
			file.Read (&nInStrip, 2);
			wad.Write (&material, 2);
			wad.Write (&nInStrip, 2);
			// Deal with that many triangles
			Uint16 *foo = (Uint16 *)malloc (nInStrip * 3 * 2);
			file.Read (foo, nInStrip * 3 * 2);
			wad.Write (foo, nInStrip * 3 * 2);
			tri -= nInStrip;

			//MATT ADD
			free(foo);
		}
		int quad = model.nQuads;
		while (quad) {
			// Read this 'strip' info
			Uint16 material, nInStrip;
			file.Read (&material, 2);
			file.Read (&nInStrip, 2);
			wad.Write (&material, 2);
			wad.Write (&nInStrip, 2);
			// Deal with that many quads
			Uint16 *foo = (Uint16 *)malloc (nInStrip * 4 * 2);
			file.Read (foo, nInStrip * 4 * 2);
			wad.Write (foo, nInStrip * 4 * 2);
			//MATT ADD
			free (foo);
			quad -= nInStrip;
		}
		int strip = model.nStripPolys;
		while (strip) {
			// Read this 'strip' info
			Uint16 material, nInStrip;
			file.Read (&material, 2);
			file.Read (&nInStrip, 2);
			wad.Write (&material, 2);
			wad.Write (&nInStrip, 2);
			// Deal with that many strip entries
			Uint16 *foo = (Uint16 *)malloc (nInStrip * 2);
			file.Read (foo, nInStrip * 2);
			wad.Write (foo, nInStrip * 2);
			//MATT ADD
			free (foo);
			strip -= (nInStrip-2);
		}
		// Deal with the terminator!
		if (model.nStripPolys) {
			Uint16 foo[2];
			file.Read (foo, 4);
			wad.Write (foo, 4);
		}
#else
		VertRef *strip = new VertRef[temp];
		file.Read (strip, sizeof (VertRef) * temp);
		wad.Write (strip, sizeof (VertRef) * temp);
		delete [] strip;
#endif

	}
	// Deal with any children
	for (int child = 0; child < obj.no_child; ++child) {
		if (!RecurseReadObj (file, wad, wadTexList))
			return false;
	}
	return true;
}


// Get the GBIX of a texture
Uint32 CWorker::getGBIX (CString texture, Uint32 &wadGBIX)
{
	Uint32 retVal;
	record.Open	(dbOpenDynaset, 
		"SELECT * "
		"FROM Textures "
		"WHERE TexFilename ='P:\\TEXTURES\\" + texture + ".TGA'");
	if (record.IsEOF()) {
		Log ("Missing texture '"+texture+"' - has it been converted?");
		record.Close();
		return 0xffffffff;
	}
	retVal = record.m_GBIX;
	wadGBIX = record.m_wadGBIX ? record.m_wadGBIX : record.m_GBIX;
	record.Close();
	return retVal;
}

void CWorker::Log (CString s)
{
	COPYDATASTRUCT cds;
	cds.dwData = 0;
	cds.lpData = (void *) ((const char *)s);
	cds.cbData = s.GetLength() + 1;
	SendMessage(window,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);
}

void CWadderDlg::OnTimer(UINT nIDEvent) 
{
	KillTimer (0);
	EndDialog (TRUE);
	CDialog::OnTimer(nIDEvent);
}
