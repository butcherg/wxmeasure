#ifndef __MEASUREPANE_H
#define __MEASUREPANE_H

#include "wx/wx.h"
#include <wx/filename.h>


class measurePane: public wxPanel
{
public:

	measurePane(wxWindow *parent, int id, wxPoint pos, wxSize size);
	void OnPaint(wxPaintEvent& event);
	void Resize();
	void setStatus();
	void setStatus(wxPoint p);
	void mouseLeftDown(wxMouseEvent& event);
	void mouseLeftDClick(wxMouseEvent& event);
	
	void mouseMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void OnMouseWheel(wxMouseEvent& event);
	void render(wxDC &dc);
	void setImage(wxImage image);
	void setImageFile(wxFileName f);
	void setCalibration(long cal);
	void refreshImage();
	void OnTimer(wxTimerEvent& event);

	wxPoint getImageCoord(wxPoint p)  ;
	wxPoint getScreenCoord(wxPoint p);
	float toRadians(float deg);
	
	void DrawMarkedLine(wxDC &dc, wxPoint tip, wxPoint tail);
	
protected:

	bool smallerThan();
	
private:

	wxFileName filename;
	wxImage img, scaledimg;
	float scale;
	wxPoint pos;
	int ifw, ifh, imgx, imgy, imgw, imgh, prevx, prevy;
	bool dragging, fit;
	wxTimer t;
	wxDateTime modtime;
	int lastline;
	
	bool measuring, calibrating;
	//unsigned beginx, beginy, endx, endy, lastline;
	wxPoint begin, end;
	
	int calibration, pxmeasurement;
	float measurement;
};

#endif