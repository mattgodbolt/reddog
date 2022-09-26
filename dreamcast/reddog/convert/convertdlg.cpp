// ConvertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Convert.h"
#include "ConvertDlg.h"
#include "Progress.h"
#include "TexProp.h"
#include "VQ.h"
#include "Animate.h"
#include "TGAload.h"
#include <fstream.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static DWORD lastCount = 400;
static BOOL automated = FALSE;

///////////////////////////////////////////////////////////////////////////////
static bool Execute (char *cmd)
{
	PROCESS_INFORMATION	inf;
	STARTUPINFO			sui;
	SECURITY_ATTRIBUTES sa;
	char				szBuffer[5120];
	DWORD				code, cRead;
	HANDLE              hPipeOutputRead  = NULL;
	HANDLE              hPipeOutputWrite = NULL;
	HANDLE              hPipeInputRead   = NULL;
	HANDLE              hPipeInputWrite  = NULL;

	memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

	// Create pipe for standard output redirection.
	if(CreatePipe(&hPipeOutputRead, &hPipeOutputWrite, &sa,0)){
		if(CreatePipe(&hPipeInputRead, &hPipeInputWrite, &sa, 0)){
	
			memset(&sui, 0, sizeof(sui));
			sui.cb = sizeof(STARTUPINFO);
			sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//			sui.wShowWindow = SW_SHOW;
			sui.wShowWindow = SW_HIDE;
			sui.hStdInput   = hPipeInputRead;
			sui.hStdOutput  = hPipeOutputWrite;
			sui.hStdError   = hPipeOutputWrite;

			if(CreateProcess(
						  NULL,				/* pointer to name of executable module*/
						  cmd,				/* pointer to command line string*/
						  NULL,				/* process security attributes*/
						  NULL,				/* thread security attributes*/
						  TRUE,				/* handle inheritance flag*/
						  0,				/* creation flags*/
						  NULL,				/* pointer to new environment block*/
						  NULL,				/* pointer to current directory name*/
						  &sui,				/* pointer to STARTUPINFO*/
						  &inf)){			/* pointer to PROCESS_INFORMATION*/


				code = STILL_ACTIVE;
				while (code == STILL_ACTIVE){
					if(ReadFile(hPipeOutputRead,&szBuffer,5000, &cRead, NULL)){
						szBuffer[cRead] = 0;  // null terminate
						OutputDebugString (szBuffer);
					}
					WaitForSingleObject (inf.hProcess, 10);
					GetExitCodeProcess(inf.hProcess, &code);
				}				
				WaitForSingleObject (inf.hProcess, INFINITE);
				CloseHandle (inf.hProcess);
			}
			CloseHandle (hPipeInputWrite);	
		}
		CloseHandle (hPipeOutputRead);
	}
	return true;
}
int mgMessageBox (const char *mes, char *cap = NULL, int type = MB_OK)
{
	if (automated)
		return IDYES;
	else
		return MessageBox (NULL, mes, cap, type);
}

static unsigned long Crc32Table[256];
static void initCRCTab(void)
{
	int i, j;
	unsigned long crc;
	for (i=0; i < 256; ++i) {
		crc = (i<<24);
		for (j=0;j<8;++j)
			crc = (crc<<1) ^ ((crc & 0x80000000L) ? 0x04c11db7L : 0);
		Crc32Table[i] = crc;
	}
}

static unsigned long makeGBIX (char *mem, int len)
{
	unsigned long ret = 0;

	while (len) {
		ret = Crc32Table[((ret>>24) ^ *mem++) & 0xff] ^ (ret<<8);
		len--;
	}
	return ret & 0x0fffffff;
}

static int ProcessFile (const char *filename)
{
	int handle, len, read;
	char *mem;

	handle = _open (filename, O_RDWR | O_BINARY, _S_IREAD | _S_IWRITE);

	if (handle == -1) {
		return 0;
	}

	mem = new char [len = _filelength (handle)];
	if (mem == NULL) {
		return 0;
	}

	read = _read (handle, mem, len);
	if (read == len) {
		char *newMem = mem;
		// Check for a GBIX
		if (mem[0] == 'G' && mem[1] == 'B' && mem[2] == 'I' && mem[3] == 'X') {
			newMem += 12;
			len -= 12;
		}
		if (newMem[0] == 'P' && newMem[1] == 'V' && newMem[2] == 'R' && newMem[3] == 'T') {
			struct {
				char				buf[4];
				unsigned long		len;
				unsigned long		gbix;
			} GBIX;
			GBIX.buf[0] = 'G';
			GBIX.buf[1] = 'B';
			GBIX.buf[2] = 'I';
			GBIX.buf[3] = 'X';
			GBIX.len	= 4;
			GBIX.gbix	= makeGBIX (newMem, len);

			_close (handle);
			handle = _open (filename, O_WRONLY | O_TRUNC | O_BINARY, _S_IREAD | _S_IWRITE);
			
			_write (handle, &GBIX, 12);
			_write (handle, newMem, len);

		} else {
		delete [] mem;
		_close (handle);
		return 0;
		}
	} else {
		delete [] mem;
		_close (handle);
		return 0;
	}
	delete [] mem;
	_close (handle);

	// Stamp the file
	CFileStatus stat;
	if (CFile::GetStatus (filename, stat)) {
		stat.m_mtime = CTime::GetCurrentTime();
		CFile::SetStatus (filename, stat);
	}

	return 1;
}

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
// CConvertDlg dialog
extern CConvertApp theApp;

CConvertDlg::CConvertDlg(CAskType &ask, CWnd* pParent /*=NULL*/)
	: CDialog(CConvertDlg::IDD, pParent), parms(ask)
{
	//{{AFX_DATA_INIT(CConvertDlg)
	m_FilterAnimated = FALSE;
	m_FilterCompressed = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Read the last count
	HKEY key;
	DWORD ignored, i2;
	RegCreateKeyEx (HKEY_CURRENT_USER, "SOFTWARE\\Argonaut Software Ltd\\Red Dog\\Convert", 0, REG_NONE, 
		0, KEY_ALL_ACCESS, NULL, &key, &ignored);
	i2 = 4;
	RegQueryValueEx (key, "Number of textures", 0, &ignored, (BYTE *)&lastCount, &i2);
	RegCloseKey (key);

	m_ImageList = NULL;
	initCRCTab();

	if (theApp.m_lpCmdLine[0]) {
		// Automatically update all textures
		if (!stricmp (theApp.m_lpCmdLine, "/all")) {
			automated = TRUE;
		}
	}

}

CConvertDlg::~CConvertDlg()
{
	// Set the last count
	HKEY key;
	DWORD ignored;
	RegCreateKeyEx (HKEY_CURRENT_USER, "SOFTWARE\\Argonaut Software Ltd\\Red Dog\\Convert", 0, REG_NONE, 
		0, KEY_ALL_ACCESS, NULL, &key, &ignored);
	RegSetValueEx (key, "Number of textures", 0, REG_DWORD, (BYTE *)&lastCount, 4);
	RegCloseKey (key);

	delete m_ImageList;
}

void CConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConvertDlg)
	DDX_Control(pDX, IDC_TEXTURELIST, m_TextureList);
	DDX_Check(pDX, IDC_FILTER_ANIMATED, m_FilterAnimated);
	DDX_Check(pDX, IDC_FILTER_COMPRESSED, m_FilterCompressed);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConvertDlg, CDialog)
	//{{AFX_MSG_MAP(CConvertDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_NOTIFY(NM_RCLICK, IDC_TEXTURELIST, OnRclickTexturelist)
	ON_NOTIFY(NM_DBLCLK, IDC_TEXTURELIST, OnDblclkTexturelist)
	ON_BN_CLICKED(IDC_SHOWALL, OnShowall)
	ON_BN_CLICKED(IDC_LIST_OUTDATED, OnShowall)
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConvertDlg message handlers

CString CConvertDlg::getPVRFile (CString fileName)
{
	if (fileName.Left(12).CompareNoCase("P:\\TEXTURES\\") == 0) {
		CString pvr = "P:\\GAME\\TEXMAPS\\" + fileName.Mid(12, fileName.GetLength() - 12 - 3) + "PVR";
		return pvr;
	} else
		return fileName;
}

CString CConvertDlg::getTGAFile (CString fileName)
{
	if (fileName.Left(16).CompareNoCase("P:\\GAME\\TEXMAPS\\") == 0) {
		CString pvr = "P:\\TEXTURES\\" + fileName.Mid(16, fileName.GetLength() - 16 - 3) + "TGA";
		return pvr;
	} else
		return fileName;
}

CString CConvertDlg::getSETFile (CString fileName)
{
	CString set = fileName.Mid(0, fileName.GetLength() - 3) + "SET";
	return set;
}

BOOL CConvertDlg::isUpToDate (CString fileName)
{
	CString pvrFile = getPVRFile (fileName);
	CFileStatus tgaStats, pvrStats;

	if (CFile::GetStatus (fileName, tgaStats) == FALSE)
		return 1024;
	if (CFile::GetStatus (pvrFile, pvrStats) == FALSE)
		return FALSE;

	if (tgaStats.m_mtime > pvrStats.m_mtime)
		return FALSE;
	return TRUE;
}

int CConvertDlg::FillListBox (BOOL all /* = GetListBox */)
{
	static int count, files;
	int nFilesInHere = 0;
	if (all == 100)
		all = IsDlgButtonChecked (IDC_SHOWALL);

	m_TextureList.DeleteAllItems();
	CProgress progress;
	progress.SetText ("Searching for image files");
	count = files = 0;
	UpdateData();

	CTexRec record;
	record.Open(dbOpenTable);
	
	int pref = parms.m_StartDir.GetLength();

	int size = 0;
	while (!record.IsEOF())	{
		if (record.m_Texture_filename.Right (8).CompareNoCase("_off.tga") != 0 &&
			record.m_Texture_filename.Left(pref).CompareNoCase(parms.m_StartDir) == 0) {
			CString displayName = record.m_Texture_filename.Mid (12, record.m_Texture_filename.GetLength()-16);
			int inDate = isUpToDate (record.m_Texture_filename);
			if (inDate == 1024) {
				record.Delete();
			} else {
				if (!((m_FilterAnimated && !record.m_First_frame_of_animation) ||
					(m_FilterCompressed && !record.m_VQ_compress))) {
					if (all || !inDate) {
						m_TextureList.InsertItem (0, displayName, inDate?0:1);
						nFilesInHere++;
					}
					int thisSize = m_TextureList.GetStringWidth(displayName);
					if (thisSize > size)
						size = thisSize;
				}
			}
		}
		record.MoveNext();
		files++;
		if ((files & 15)==0)
			progress.SetProgress((100 * files) / (lastCount+1));
	}
	m_TextureList.SetColumnWidth(0, size + 16);
	
	progress.DestroyWindow();
	lastCount = files;
	if (lastCount == 0)
		lastCount = 1;

	return nFilesInHere;
}
static int nFiles = 0;
void CConvertDlg::RegenerateDatabase (void)
{
	CTexRec record;
	record.Open(dbOpenTable);
	CProgress prog;
	nFiles = 0;

	prog.SetText ("Regenerating texture database...");
	RecurseRegenerate (record, parms.m_StartDir, prog);

	prog.DestroyWindow();
	record.Close();
}

void CConvertDlg::RecurseRegenerate (CTexRec &record, CString startDirectory, CProgress &prog)
{
	CFileFind finder;

	BOOL working = finder.FindFile (startDirectory + "\\*.*");
	while (working) {
		working = finder.FindNextFile();

		if (finder.IsDots())
			continue;

		if (finder.IsDirectory()) {
			RecurseRegenerate (record, finder.GetFilePath(), prog);
		} else if (finder.GetFileName().Right(4) == ".TGA" || finder.GetFileName().Right(4) == ".tga") {
			if (!finder.GetFileName().Right(7).CompareNoCase("_DN.TGA") ||
				!finder.GetFileName().Right(7).CompareNoCase("_LF.TGA") ||
				!finder.GetFileName().Right(7).CompareNoCase("_RT.TGA") ||
				!finder.GetFileName().Right(7).CompareNoCase("_FR.TGA") ||
				!finder.GetFileName().Right(7).CompareNoCase("_BK.TGA"))
				continue;
				
			CString fileName = finder.GetFilePath();
			TextureSettings set (fileName, FALSE);
			record.AddNew();
			record.m_Texture_filename 	= fileName;
			record.m_Don_t_dither		= set.dontDither;
			record.m_Don_t_dither_alpha	= set.dontDitherAlpha;
			record.m_Don_t_mipmap		= set.dontMipmap;
			record.m_First_frame_of_animation = set.firstAnimFrame;
			record.m_GlobalAnim			= set.globalAnim;
			record.m_StratName			= set.animName;
			record.m_wadGBIX			= 0;
			if (set.wasCreated) {
				record.m_VQ_compress	= finder.GetLength() > (64*64*2);
			} else {
				record.m_VQ_compress	= set.vqCompress;
			}
			nFiles++;
			if ((nFiles & 15) == 0) {
				prog.SetProgress((100 * nFiles) / (lastCount+1));
				prog.SetExtra (fileName);
			}
			try {
				record.Update();
			} catch (CException *e) {
				e->Delete();
			}
		}
	}
}

BOOL CConvertDlg::OnInitDialog()
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

	// Get the parameters
	if (parms.DoModal() != IDOK) {
		EndDialog (IDCANCEL);
		return TRUE;
	}
	
	if (GetAsyncKeyState (VK_MENU) & 0x8000 || (parms.m_WhatToDo == 1))
		CheckDlgButton (IDC_SHOWALL, TRUE);
	else
		CheckDlgButton (IDC_LIST_OUTDATED, TRUE);
	// Initialise the list box
	m_ImageList = new CImageList;
	m_ImageList->Create (IDB_IMAGELIST, 16, 0, 0x00ff0000);

	m_TextureList.SetImageList (m_ImageList, LVSIL_SMALL);


	// Automatic stuff
	if (automated) {
		RegenerateDatabase();
		DoFunkyStuff();
		EndDialog(TRUE);
	}

	if (parms.m_RegenerateFirst)
		RegenerateDatabase();

	FillListBox ();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CConvertDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CConvertDlg::OnPaint() 
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
HCURSOR CConvertDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

int CConvertDlg::EnumList (bool onlySelected, CStringArray &array)
{
	LV_ITEM data;
	char buf[MAX_PATH];
	int item = -1;
	int count = 0;
	
	for (;;) {

		item = m_TextureList.GetNextItem (item, onlySelected ? LVNI_SELECTED : 0);
		if (item == -1)
			break;
		
		data.iItem		= item;
		data.iSubItem 	= 0;
		data.mask		= LVIF_TEXT;
		data.pszText	= buf;
		data.cchTextMax	= MAX_PATH;
		if (m_TextureList.GetItem (&data) == 0)
			break;
		count++;	
		array.Add ("P:\\TEXTURES\\" + CString(buf) + ".TGA");
	}
	return count;
}

void CConvertDlg::OnOk() 
{
	DoFunkyStuff ();
	EndDialog (ID_OK);
}

void CConvertDlg::OnRefresh() 
{
	RegenerateDatabase();
	FillListBox ();
}

void CConvertDlg::OnRclickTexturelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint	point, origPoint;
	CRect	rect;

	GetCursorPos (&origPoint);
	m_TextureList.GetWindowRect (&rect);
	point = origPoint - rect.TopLeft();

	int item = m_TextureList.HitTest (point);

	LV_ITEM tvItem;
	char buffer[MAX_PATH];
	tvItem.mask			= LVIF_TEXT;
	tvItem.iItem		= item;
	tvItem.iSubItem		= 0;
	tvItem.pszText		= &buffer[0];
	tvItem.cchTextMax	= MAX_PATH;
	m_TextureList.GetItem (&tvItem);

	if (buffer[0] != '\0') {
		CMenu menu;
		CString totalName = "P:\\TEXTURES\\" + CString(buffer) + ".TGA";
		if (menu.LoadMenu (IDR_TEXTUREMENU)) {
			setName = totalName;
			CMenu *pPopup = menu.GetSubMenu (0);
			pPopup->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, origPoint.x, origPoint.y, AfxGetMainWnd());
		}
	}
	*pResult = 0;
}

static void MakeOffset (CString name)
{
	int startNum = 0;
	CString frameName, baseName;
	CString format = CAnimatedTexture::getFormat (name, startNum);
	format = format.Mid (0, format.GetLength() - 3) + "TGA";
	// Find the offset to 'this' texture
	int i = 0;
	int offset = 0;
	baseName = "";
	startNum = -1;
	do {
		CFileStatus status;
		frameName.Format (format, i);
		if (name.CompareNoCase (frameName)==0)
			break;
		if (baseName == "") {
			if (CFile::GetStatus (frameName, status))
				baseName = frameName;
		}
		if (baseName != "") {
			offset++;
		}
		i++;
	} while (offset != 1000);
	if (offset == 1000) {
		MessageBox (NULL, "Texture isn't part of an animation!", NULL, MB_OK);
		return;
	}
	CString newName = name.Mid(0, name.GetLength() - 4) + CString ("_off.TGA");
	CString PVRfile = CConvertDlg::getPVRFile (newName);
	if (!CopyFile (name, newName, FALSE)) {
		MessageBox (NULL, "Copy failed! Speak to MattG!", NULL, MB_OK);
		return;
	}
	// Get the GBIX of the base anim
//	baseName.Format (format, startNum);
	TextureSettings set(baseName);
	TexAnim anim;
	anim.type = TEXANIM_OFFSET;
	anim.speed = offset;
	CFile output(PVRfile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary);
	output.Write (&anim, sizeof (anim));
	struct {
		unsigned int	type;
		unsigned int	length;
		unsigned int	gbix;
	} global = { *(unsigned int *)&"GBIX", 4, set.GBIX };
	output.Write (&global, sizeof (global));

	output.Close();
	CString message;
	TextureSettings set2(newName);
	set2 = set;
	set2.GBIX = set.GBIX + offset;
	set2.wadGBIX = set.GBIX;
	set2.WriteBack(newName);
	
	message.Format ("A new texture called '%s' has been created as an offset animation from %s (offset %d)",
		newName, baseName, offset);
	MessageBox (NULL, message, NULL, MB_OK);
}

BOOL CConvertDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (LOWORD(wParam)) {
	case IDM_PROPERTIES:
		new CTexProp(setName);
		return TRUE;
	case IDM_MAKEOFFSET:
		MakeOffset (setName);
		return TRUE;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void CConvertDlg::OnDblclkTexturelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint	point;
	CRect	rect;

	GetCursorPos (&point);
	m_TextureList.GetWindowRect (&rect);
	point = point - rect.TopLeft();

	int item = m_TextureList.HitTest (point);

	LV_ITEM tvItem;
	char buffer[MAX_PATH];
	tvItem.mask			= LVIF_TEXT;
	tvItem.iItem		= item;
	tvItem.iSubItem		= 0;
	tvItem.pszText		= &buffer[0];
	tvItem.cchTextMax	= MAX_PATH;
	m_TextureList.GetItem (&tvItem);

	if (buffer[0] != '\0') {
		CString totalName = "P:\\TEXTURES\\" + CString(buffer) + ".TGA";
		setName = totalName;
		new CTexProp (setName);
	}
	*pResult = 0;
}
TextureSettings::TextureSettings (CString fileName, bool fromDB /* = TRUE */)
{
	dontMipmap = dontDither = dontDitherAlpha = vqCompress = 
	firstAnimFrame = FALSE;
	globalAnim = TRUE;
	animName = "NEWANIM";
	animSpeed = 1;
	wasCreated = TRUE;
	
	if (!fromDB) {
		setFile = CConvertDlg::getSETFile(fileName);
		ifstream i(setFile, ios::in | ios::nocreate);
		if (i.is_open()) {
			while (!i.eof()) {
				char option[256];
				i >> option;
				if (!stricmp (option, "-nm"))
					dontMipmap = TRUE;
				else if (!stricmp (option, "-nd"))
					dontDither = TRUE;
				else if (!stricmp (option ,"-na"))
					dontDitherAlpha = TRUE;
				else if (!stricmp (option, "+vq"))
					vqCompress = TRUE;
				else if (!stricmp (option, "+animated"))
					firstAnimFrame = TRUE;
				else if (!stricmp (option, "+stratanim"))
					globalAnim = FALSE;
				else if (!strnicmp (option, "+speed=", strlen("+speed=")))
					animSpeed=atoi(&option[strlen("+speed=")]);
				else if (!strnicmp (option, "+name=", strlen("+name=")))
					animName = &option[strlen("+name=")];
			}
			i.close();
		}
	} else {
		CTexRec findIt;
		findIt.Open	(dbOpenDynaset, 
			"SELECT * "
			"FROM Textures "
			"WHERE TexFilename ='" + CString (fileName) + "'");
		dontMipmap = findIt.m_Don_t_mipmap ? TRUE : FALSE;
		dontDither = findIt.m_Don_t_dither ? TRUE : FALSE;
		dontDitherAlpha = findIt.m_Don_t_dither_alpha ? TRUE : FALSE;
		vqCompress		= findIt.m_VQ_compress ? TRUE : FALSE;
		firstAnimFrame = findIt.m_First_frame_of_animation ? TRUE : FALSE;
		globalAnim		= findIt.m_GlobalAnim ? TRUE : FALSE;
		animSpeed		= findIt.m_GlobalSpeed;
		animName		= findIt.m_StratName;
		GBIX			= findIt.m_GBIX;
		wadGBIX			= findIt.m_wadGBIX;
		if (wadGBIX == 0)
			wadGBIX = GBIX;
		bumpMap			= findIt.m_BumpMap ? true:false;
		punchThru		= findIt.m_Punchthru ? true:false;
		if (findIt.IsEOF() && findIt.IsBOF()) {
			wasCreated = true;
		} else wasCreated = false;
		
	}
}

void TextureSettings::WriteBack (CString fileName)
{
	CTexRec updateIt;
	updateIt.Open	(dbOpenDynaset, 
		"SELECT * "
		"FROM Textures "
		"WHERE TexFilename ='" + CString (fileName) + "'");
	if (updateIt.IsEOF() && updateIt.IsBOF()) {
		updateIt.AddNew();
		updateIt.m_Texture_filename = fileName;
	} else {
		updateIt.Edit();
	}
	
	updateIt.m_Don_t_dither = dontDither;
	updateIt.m_Don_t_mipmap = dontMipmap;
	updateIt.m_Don_t_dither_alpha = dontDitherAlpha;
	updateIt.m_VQ_compress = vqCompress;
	updateIt.m_First_frame_of_animation = firstAnimFrame;
	updateIt.m_GlobalAnim = globalAnim;
	updateIt.m_GlobalSpeed = animSpeed;
	updateIt.m_StratName = animName;
	updateIt.m_GBIX		 = GBIX;
	updateIt.m_wadGBIX	 = (wadGBIX==0)?GBIX:wadGBIX;
	updateIt.m_BumpMap	= bumpMap;
	updateIt.m_Punchthru = punchThru;

	if (updateIt.m_BumpMap)
		updateIt.m_Don_t_mipmap = true;

	updateIt.Update();	
}

void CConvertDlg::OnShowall() 
{
	FillListBox();	
}

void CConvertDlg::DoFunkyStuff() 
{
	char c[MAX_PATH];
	CStringArray	tgaList;
	CStringArray	vqList;
	CStringArray	animList;
	CStringArray	animName;
	CUIntArray		animSpeed;
	CByteArray		animIsStrat;
	int nFiles, count = 0, onlySelected;
	GetWindowsDirectory (&c[0], MAX_PATH);

	UpdateData();

	CProgress progress(this);
	progress.SetText ("Converting TGA to PVR...");

	int numSel = EnumList (TRUE, tgaList);
	if (numSel != 0) {
		onlySelected = TRUE;
		nFiles = numSel;
	} else {
		onlySelected = FALSE;
		nFiles = EnumList (FALSE, tgaList);
	}

	if (nFiles == 0)
		return;

	if (!onlySelected && IsDlgButtonChecked (IDC_SHOWALL) &&
		!m_FilterAnimated && !m_FilterCompressed) {
		int result = mgMessageBox ("This will convert every texture on the network - are you sure you want to continue?", "Message from Convert", MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2);
		if (result != IDYES) {
			return;
		}
	}

	int item;
	for (item = 0; item < nFiles; ++item) {
		CString tgaFile = tgaList[item];
		CString outDir = getPVRFile (tgaFile);
		int slashPos = outDir.GetLength();
		for (int i = 0; i < outDir.GetLength(); ++i) {
			if (outDir[i] == '\\') {
				slashPos = i;
				CreateDirectory (outDir.Mid (0, slashPos), NULL);
			}
		}
		outDir = outDir.Mid (0, slashPos);
		CreateDirectory (outDir, NULL);
		
		TextureSettings set (tgaFile);
/*		if (set.dontDither)
			o << "-nd ";
		if (set.dontDitherAlpha)
			o << "-na ";
		if (set.dontMipmap)
			o << "-nm ";*/
		if (set.vqCompress)
			vqList.Add (getPVRFile (tgaFile));
		if (set.firstAnimFrame) {
			animList.Add (getPVRFile(tgaFile));
			animName.Add (set.animName);
			animSpeed.Add (set.animSpeed);
			animIsStrat.Add (!set.globalAnim);
		}

		progress.SetExtra (tgaFile.Mid(12));
		if ((item & 3) == 0)
			progress.SetProgress((100 * item) / nFiles);

		// Check to see if this is an enviroment map
		if (tgaFile.Right(7).CompareNoCase("_UP.TGA")==0) {
			CString base = tgaFile.Mid(0, tgaFile.GetLength() - 7);
			
			Targa up (base + "_UP.TGA", true);
			Targa dn (base + "_DN.TGA", true);
			Targa lf (base + "_LF.TGA", true);
			Targa rt (base + "_RT.TGA", true);
			Targa fr (base + "_FR.TGA", true);
			Targa bk (base + "_BK.TGA", true);
			
			Targa targa (up, dn, lf, rt, fr, bk);
			
			if (!targa.isOK()) {
				mgMessageBox ("Unable to process environment map '" + tgaFile + "' ensure it is a square, power of 2-sized, 24 or 32 bpp image", "Matt's message box", MB_OK);
				continue;
			}
			PVR pvrFile (targa, set.dontMipmap ? -1 : 0, targa.getAlphaType());
			if (!pvrFile.isOK()) {
				mgMessageBox ("Unable to process file '" + tgaFile + "' go tell Matt!!", "Matt's message box", MB_OK);
				continue;
			}
			CFile outputFile (getPVRFile (tgaFile), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
			pvrFile.Output (outputFile);
			set.GBIX = pvrFile.getGBIX();
			outputFile.Close();

		} else {
			if (set.bumpMap) {
				Targa targa (tgaFile);		
				if (!targa.isOK()) {
					mgMessageBox ("Unable to process file '" + tgaFile + "' ensure it is a square, power of 2-sized, 24 or 32 bpp image", "Matt's message box", MB_OK);
					continue;
				}
				Targa::AlphaType aType = targa.getAlphaType();
				if (set.punchThru)
					aType = Targa::PUNCHTHRU;
				PVR pvrFile (targa, set.dontMipmap ? -1 : 0, aType);
				if (!pvrFile.isOK()) {
					mgMessageBox ("Unable to process file '" + tgaFile + "' go tell Matt!!", "Matt's message box", MB_OK);
					continue;
				}
				if (set.bumpMap)
					pvrFile.MakeBumpMap();
				CFile outputFile (getPVRFile (tgaFile), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
				pvrFile.Output (outputFile);
				set.GBIX = pvrFile.getGBIX();
				outputFile.Close();
			} else {
				Targa targa (tgaFile);		
				if (!targa.isOK()) {
					mgMessageBox ("Unable to process file '" + tgaFile + "' ensure it is a square, power of 2-sized, 24 or 32 bpp image", "Matt's message box", MB_OK);
					continue;
				}
				Targa::AlphaType aType = targa.getAlphaType();
				if (set.punchThru)
					aType = Targa::PUNCHTHRU;
				PVR pvrImage(targa, set.dontMipmap ? -1 : 0, aType);
				if (!pvrImage.isOK()) {
					mgMessageBox ("Unable to process file '" + tgaFile + "' go tell Matt!!", "Matt's message box", MB_OK);
					continue;
				}
				if (set.dontDitherAlpha) { // IE use my vqer
					CFile outputFile (getPVRFile(tgaFile), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
					pvrImage.Output (outputFile);
					outputFile.Close();

					CVqCompressor comp(getPVRFile(tgaFile), progress);
				} else {
					// OK give up and call DOS
					CFile outputFile ("c:\\temp.pvr", CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);
					pvrImage.Output (outputFile);
					outputFile.Close();
					char buf[1024];
					sprintf (buf, "p:\\utils\\pvrconv.exe \"%s\" -out \"%s\" -gi %u -t", 
						tgaFile, getPVRFile (tgaFile),
						pvrImage.getGBIX());
					switch (aType) {
					case Targa::PUNCHTHRU:
						strcat (buf, " -1555");
						break;
					case Targa::FULL:
						strcat (buf, " -4444");
						break;
					case Targa::NONE:
						strcat (buf, " -565");
						break;
					}
					if (set.vqCompress)
						strcat (buf, " -v4a -v4p");
					if (set.dontMipmap)
						strcat (buf, " -nm -nvm");
					Execute (buf);					
					set.GBIX = pvrImage.getGBIX();
				}
			}
		}
		// Write back the new GBIX
		set.WriteBack (tgaFile);
		// Stamp the output file
		CFileStatus stat;
		if (CFile::GetStatus (getPVRFile (tgaFile), stat)) {
			stat.m_mtime = CTime::GetCurrentTime();
			CFile::SetStatus (getPVRFile (tgaFile), stat);
		}
	}
#if 0
	// See if we need to do any VQ compression
	if (vqList.GetSize() > 0) {
		int num = vqList.GetSize() - 1;
		if (num == 0)
			num = 1;
		for (int i = 0; i < vqList.GetSize(); ++i) {
			char buf[256];
			progress.SetText ("VQ compressing " + vqList[i].Mid(16));

			CVqCompressor comp(vqList[i], progress);
			progress.SetProgress ((100 * i) / num);
		}
	}
#endif
	progress.SetExtra ("");
	
	// See if we need to sort out any animations
	if (animList.GetSize() > 0) {
		int num = animList.GetSize() - 1;
		if (num == 0)
			num = 1;
		for (int i = 0; i < animList.GetSize(); ++i) {
			progress.SetText ("Processing animation " + animList[i].Mid(16));
			if (animIsStrat[i]) {
				CAnimatedTexture tex(animList[i], animName[i], animSpeed[i], progress);
			} else {
				CAnimatedTexture tex(animList[i], animSpeed[i], progress);
			}
			progress.SetProgress ((100 * i) / num);
		}
	}

	progress.DestroyWindow();	
}

void CConvertDlg::OnConvert() 
{
	// TODO: Add your control notification handler code here
	DoFunkyStuff();
	FillListBox ();
}

