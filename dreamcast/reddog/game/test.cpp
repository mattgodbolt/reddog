#if 0
extern "C" {
#include "RedDog.h"
#include "GameState.h"
#include "View.h"
}

#define MAX_PAGES 16

/*
 * The virtual Front End Page handler
 * Constructors and destructors should handle data preparation/finalisation
 * Prepare/Finalise handle UI
 */
class FEPage
{
private:
	// Front end variables
	static int StackPtr;
	static FEPage *Stack[MAX_PAGES];
	
public:
	// Virtual destructor to allow proper inheritance
	virtual ~FEPage() {}

	// Initialises the FrontEnd at a particular page
	static void Initialise (FEPage *page);

	// Run a frame of the Front End, supplied by FEPage, non-overridable
	static void RunOneFrame (Uint32 padPress);

	// Methods supplied by FEPage, overridable:
	// Go back to the previous screen
	virtual void Back();
	// Go to a new state
	virtual void NextPage (FEPage *page);

	// User overridden methods:
	// Method used to draw a page, called once a frame
	virtual void Draw() = 0;
	// Method used to process input, if any
	virtual void ProcessInput (Uint32 padPress) = 0;
	// Method used to initialise a page prior to being drawn for the first time
	virtual void Prepare(void) {}
	// Method used to finalise a page just after being drawn for the last time
	virtual void Finalise(void) {}
};

#define MAX_WIDGET_TYPES 16

class Widget
{
protected:
	friend class WidgetFactory;
	virtual Widget *Clone() = 0;
public:
	virtual ~Widget() {}
	virtual void Load (Stream *s) = 0;
	virtual void Display () = 0;
};

class TextWidget : public Widget
{
private:
	virtual Widget *Clone() { return new TextWidget; }

	char *text;
	float pos[4];
	Colour col;
public:
	TextWidget() : text(NULL) {
		pos[0] = 100;
		pos[1] = 100;
		pos[2] = 300;
		pos[3] = 200;
	}
	~TextWidget()
	{
		if (text)
			delete[] text;
	}
	void Load (Stream *s)
	{
		int length;
		sRead (&length, 4, s);
		text = new char[length];
		sRead (text, length, s);
//		sRead (pos,. 16, s);
	}
	void Display ()
	{
		rSetMaterial (&fadeMat);
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);
		kmxxSetVertex_0 (0xe0000000, pos[0], pos[1], 10, col);
		kmxxSetVertex_0 (0xe0000000, pos[0], pos[3], 10, col);
		kmxxSetVertex_0 (0xe0000000, pos[2], pos[1], 10, col);
		kmxxSetVertex_0 (0xf0000000, pos[2], pos[3], 10, col);
		kmxxReleaseCurrentPtr (&vertBuf);
	}
};

class WidgetFactory
{
private:
	int				wId[MAX_WIDGET_TYPES]; // The widget ID for each kind of widget
	Widget			*widget[MAX_WIDGET_TYPES];// The actual creators for each kind of widget
	int				n;						// Current widget number
public:
	WidgetFactory() : n(0) { }

	void RegisterWidget (int id, Widget *example)
	{
		dAssert (id < MAX_WIDGET_TYPES, "Too many widget types");
		wId[n] = id;
		widget[n] = example;
		n++;
	}

	Widget *Create (int id)
	{
		int i;
		for (i = 0; i < n; ++i) {
			if (wId[i] == id)
				return widget[i]->Clone();
		}
		dAssert (FALSE, "Unable to find widget");
		return NULL;
	}

};
WidgetFactory *theWidgetFactory;


class StdAppearance
{
private:
	int			nWidgets;		// Number of widgets
	Widget		**widget;		// Widget array
public:
};

class WindowsPage : public FEPage
{
private:
	int foo;
public:
	WindowsPage() { foo = 123; }
	void Draw() { }
	void ProcessInput(Uint32 padPress) { }
};

WindowsPage *testPage;

////////////////////////////////////////////////////////////////

int FEPage::StackPtr;
FEPage *FEPage::Stack[MAX_PAGES];
// Initialises the FrontEnd at a particular page
void FEPage::Initialise (FEPage *page)
{
	StackPtr = 0;
	Stack[StackPtr] = page;
	page->Prepare();
}
// Run a frame of the Front End, supplied by FEPage, non-overridable
void FEPage::RunOneFrame (Uint32 padPress)
{
	FEPage *page = Stack[StackPtr];
	if (padPress)
		page->ProcessInput (padPress);
	page->Draw();
}
// Go back to the previous screen
void FEPage::Back()
{
	dAssert (StackPtr > 0, "Stack underflow");
	Finalise();
	StackPtr--;
	Stack[StackPtr]->Prepare();
}

// Go to a new state
void FEPage::NextPage (FEPage *page)
{
	dAssert (StackPtr < (MAX_PAGES-1), "Stack overflow");
	Finalise();
	StackPtr++;
	Stack[StackPtr] = page;
	page->Prepare();
}

extern "C" void testHandler (GameState state, GameStateReason reason)
{
	static Camera *fEndCam;
	static FEPage *curPage;

	switch (reason) {
	case GSR_INITIALISE:

		fEndCam = CameraCreate();
		fEndCam->type = tagCamera::CAMERA_LOOKAROUND;
		fEndCam->farZ = GlobalFogFar = 5000.f;
		fEndCam->fogNearZ = 0.99f * fEndCam->farZ;
		fEndCam->u.lookDir.rotX = fEndCam->u.lookDir.rotY = fEndCam->u.lookDir.rotZ = 0;
		fEndCam->pos.y = -5.f;
		fEndCam->pos.z = 1.f;
		fEndCam->fogColour.colour = 0;
		CameraSet (fEndCam, 0 );
		testPage = new WindowsPage();
		FEPage::Initialise (testPage);
		curPage = testPage;
		break;

	case GSR_RUN:
		CameraSet (fEndCam, 0);
		FEPage::RunOneFrame(0);
		break;

	case GSR_FINALISE:
		delete testPage;
		break;
	}
}

extern "C" void TestCPlusPlus(void)
{
	// Initialise all the once-only variables
	theWidgetFactory = new WidgetFactory();
	theWidgetFactory->RegisterWidget (0, new TextWidget);

	Widget *w = theWidgetFactory->Create(0);
	testHandler (GS_GAME, GSR_INITIALISE);
	testHandler (GS_GAME, GSR_RUN);
	testHandler (GS_GAME, GSR_FINALISE);
}
#else
static void moo(void)
{
}
#endif
