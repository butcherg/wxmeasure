/*
    wxmeasure - Just views images
    Copyright (C) 2022  Glenn Butcher, glenn.butcher@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "wx/wx.h"
//#include "wxmeasure.xpm"
#include "myConfig.h"
#include "myPropertyDialog.h"
#include "strutil.h"
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/dnd.h>
#include <wx/cmdline.h>
#include <wx/grid.h>
#include <wx/aui/aui.h>
#include <vector>
#include <string>

#include "measurepane.h"

#define MARGIN 80

//struct pt {int x, y; float r;};
struct pt {float x, y, r;};

//#define POLYROUND
#define TEXTCTRLHEIGHT 20

const wxString WXMEASURE_VERSION = "0.1.1";

/*
wxArrayString split(wxString str, wxString delim)
{
	wxArrayString a;
	wxStringTokenizer tokenizer(str, delim);
	while ( tokenizer.HasMoreTokens() ) {
		wxString token = tokenizer.GetNextToken();
		a.Add(token);
	}
	return a;
}
*/

std::string getExeDir(std::string filename)
{
	std::string dir;

#ifdef _WIN32
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH) ;
	std::wstring exepath(exePath);
	dir = std::string(exepath.begin(), exepath.end());
	dir.erase(dir.find_last_of('\\'));
	if (filename != "") dir.append("\\"+filename);
#else
	char exePath[PATH_MAX];
	size_t len = readlink("/proc/self/exe", exePath, sizeof(exePath));
	if (len == -1 || len == sizeof(exePath))
	        len = 0;
	exePath[len] = '\0';
	dir = std::string(exePath);
	dir.erase(dir.find_last_of('/'));
	if (filename != "") dir.append("/"+filename);
#endif

	return dir;
}

class myFrame;

enum {
	Viewer_Quit = wxID_EXIT,
	Viewer_About = wxID_ABOUT,
	Viewer_Open,
	Viewer_LoadImage,
	Viewer_Properties
};

class myFileDropTarget;

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title): wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800,600))
	{
		wxMenu *fileMenu = new wxMenu;\
		fileMenu->Append(Viewer_Open, "O&pen\tCtrl-O", "Open a point file");
		fileMenu->Append(Viewer_Quit, "Q&uit\tCtrl-Q", "Quit wxmeasure");

		wxMenu *helpMenu = new wxMenu;
		helpMenu->Append(Viewer_About, "&About\tF1", "Show about dialog");

		wxMenuBar *menuBar = new wxMenuBar();
		menuBar->Append(fileMenu, "&File");
		menuBar->Append(helpMenu, "&Help");

		SetMenuBar(menuBar);

		image = new measurePane(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

		wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(image, wxSizerFlags(1).Expand());
		SetSizer(sizer);

		CreateStatusBar(3);
		configfile = "(none)";

		//prop filepath: Sets the default path when the program is started.  Default: current working directory
		file.SetPath(wxString(myConfig::getConfig().getValueOrDefault("filepath","")));
		
		SetDropTarget(new myFileDropTarget(this));

		Bind(wxEVT_SIZE, &MyFrame::OnSize, this);
		Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
		Bind(wxEVT_CLOSE_WINDOW, &MyFrame::OnClose, this);
		//Bind(wxEVT_KEY_DOWN, &myPolyPane::OnKey, poly);

	}
	
	~MyFrame()
	{
		m_mgr.UnInit();
		//Quit();
	}
	
	void OnSize(wxSizeEvent& event) 
	{
		image->Resize();
		event.Skip();
	}

	void OnPaint(wxPaintEvent& WXUNUSED(event))
	{
		wxPaintDC dc(this);
		//put dc.DrawWhatever() here...
	}
	
	void OnKey(wxKeyEvent& event)
	{
		printf("MyFrame::OnKey...\n"); fflush(stdout);
		event.Skip();
	}
	
	void OpenFile(wxFileName f)
	{
		if (f.FileExists()) {
			image->setImageFile(f);
			SetLabel(wxString::Format("wxmeasure: %s", f.GetFullName()));
		}
		else wxMessageBox(wxString::Format("File %s not found.", file.GetFullPath()));
	}
	
	void OnOpen(wxCommandEvent& WXUNUSED(event))
	{
		wxFileName f = wxFileSelector("Specify an image file:", file.GetPath(), "", "", wxFileSelectorDefaultWildcardStr, wxFD_OPEN);
		OpenFile(f);
	}
	
	void OnProperties(wxCommandEvent& WXUNUSED(event))
	{
		if (configfile == "(none)") {
			wxMessageBox(_("No configuration file found."));
			return;
		}
		if (propdiag == nullptr) {
			propdiag = new PropertyDialog(this, wxID_ANY, _("Properties"), wxDefaultPosition, wxSize(640,480));
			propdiag->LoadConfig();
			Bind(wxEVT_PG_CHANGED,&MyFrame::UpdateConfig,this);
		}
		if (propdiag != nullptr) {
			propdiag->ClearModifiedStatus();
			propdiag->Show();
		}
		else {
			wxMessageBox(_("Failed to create Properties dialog"));
		}
	}
	
	void UpdateConfig(wxPropertyGridEvent& event)
	{
		wxPGProperty * prop = event.GetProperty();
		wxString propname = event.GetPropertyName();
		wxString propval;

		propval = event.GetPropertyValue().GetString();

		SetStatusText(wxString::Format(_("Changed %s to %s."), propname, propval));
		myConfig::getConfig().setValue((const char  *) propname.mb_str(), (const char  *) propval.mb_str());
		if (!myConfig::getConfig().flush()) SetStatusText(_("Write to configuration file failed."));
		//if (myConfig::getConfig().getValueOrDefault("polyround","0") == "1") 
		//	SetStatusText(_("polyround"),2);
		//else
		//	SetStatusText(_(""),2);

		image->Refresh();
	}
	
	void setConfigFile(wxString conf)
	{
		configfile = conf;
	}
	
	void OnClose(wxCloseEvent& event)
	{
		Destroy();  
	}
	
	void OnQuit(wxCommandEvent& WXUNUSED(event))
	{
		//else Close(true);
	}
	
	
	void OnAbout(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox(wxString::Format
		(
			"wxmeasure %s\n"
			"\n"
			"This is a program that displays an image and lets one \ntake measurements from it.\n\n"
			"%s\n%s.",
			WXMEASURE_VERSION.c_str(),
			wxVERSION_STRING,
			wxGetOsDescription()
                 ),
                 "About wxmeasure",
                 wxOK | wxICON_INFORMATION,
                 this);
	}

private:

	class myFileDropTarget: public wxFileDropTarget
	{
		public:
			myFileDropTarget(MyFrame *frm) {
				frame = frm;
			}

			bool OnDropFiles (wxCoord x, wxCoord y, const wxArrayString &filenames)
			{
				if (frame) {
					wxFileName f(filenames[0]);
					f.MakeAbsolute();
					wxSetWorkingDirectory (f.GetPath());
					frame->OpenFile(f);
					return true;
				}
				return false;
			}

		private:
			MyFrame *frame;
	};

	wxFileName file;
	measurePane* image;
	PropertyDialog *propdiag;
	wxString configfile;
	//bool modified;
	
	wxAuiManager m_mgr;
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Viewer_Quit,  MyFrame::OnQuit)
    EVT_MENU(Viewer_About, MyFrame::OnAbout)
    EVT_MENU(Viewer_Open, MyFrame::OnOpen)
	EVT_MENU(Viewer_Properties, MyFrame::OnProperties)
wxEND_EVENT_TABLE()

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
	{ wxCMD_LINE_PARAM,  "",  "", "file to open", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
	{ wxCMD_LINE_NONE }
};

class MyApp : public wxApp
{
public:
	bool OnInit()
	{
		wxInitAllImageHandlers();

		wxFileName configfile("wxmeasure.conf");
		configfile.SetPath(wxString(getExeDir("")));
		if (configfile.FileExists()) { 
			myConfig::loadConfig(configfile.GetFullPath().ToStdString());
		}
		//else wxMessageBox(wxString::Format("No configuration file: %s",configfile.GetFullPath()));
		
		MyFrame *frame = new MyFrame("wxmeasure");
		
		if (configfile.FileExists()) 
			frame->setConfigFile(configfile.GetFullPath());
		
		wxCmdLineParser cmdline(cmdLineDesc, argc, argv);
		cmdline.Parse();
		
		if (cmdline.GetParamCount() > 0) {
			wxFileName f(cmdline.GetParam());
			frame->OpenFile(f);
		}
		
		//frame->SetIcon(wxmeasure);
		frame->Show(true);
		return true;
	}
	
	int OnRun()
	{
		wxApp::OnRun();
		return 0;
	}
};

wxIMPLEMENT_APP(MyApp);


