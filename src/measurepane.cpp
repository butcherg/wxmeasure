

#include "measurepane.h"
#include "myConfig.h"

enum {
	NOLINE,
	CALIBRATE,
	MEASURE,
};

int sqr(int a)
{
	return a*a;
}

int distance(wxPoint a, wxPoint b)
{
	return sqrt(sqr(a.x - b.x) + sqr(a.y - b.y));
}

wxString feetinches(float m)
{
	int feet = (int) m; //whole feet
	float ftfrac = m - feet; //fraction of a foot
	
	float n = 12*ftfrac; //inches
	int inches = (int) n; //whole inches
	float infrac = n - inches; //fraction of an inch
	
	int denom = 1/infrac;

	if (feet <= 0)
		return wxString::Format("%02.2f\"", 12*ftfrac);
		//return wxString::Format("%d 1/%d in", inches, denom);
	else
		return wxString::Format("%d' %02.2f\"", feet, 12*ftfrac);
}

	measurePane::measurePane(wxWindow *parent, int id, wxPoint pos, wxSize size): wxPanel(parent, id, pos, size)
	{
		SetDoubleBuffered(true);
		t.SetOwner(this);
		scale = 1.0;
		imgx = imgy = 0;
		dragging = measuring = calibrating = false;
		fit = true;
		calibration = measurement = 0;
		//prop backgroundcolor  - sets background color.  Default: BLACK
		wxString bgcolor = myConfig::getConfig().getValueOrDefault("backgroundcolor","BLACK"); 
		SetBackgroundColour(wxColour(bgcolor));
		
		Bind(wxEVT_PAINT, &measurePane::OnPaint, this);
		Bind(wxEVT_MOUSEWHEEL, &measurePane::OnMouseWheel,  this);
		Bind(wxEVT_LEFT_DOWN, &measurePane::mouseLeftDown, this);
		Bind(wxEVT_LEFT_DCLICK, &measurePane::mouseLeftDClick, this);
		Bind(wxEVT_MOTION, &measurePane::mouseMoved, this);
		Bind(wxEVT_LEFT_UP, &measurePane::mouseReleased, this);
		Bind(wxEVT_TIMER, &measurePane::OnTimer,  this);
	}
	
	wxPoint measurePane::getImageCoord(wxPoint p)  //from screen coord
	{
		wxPoint i;
		i.x = (p.x - imgx) / scale;
		i.y = (p.y - imgy) / scale;
		return i;
	}

	wxPoint measurePane::getScreenCoord(wxPoint p) //from image coord
	{
		wxPoint s;
		s.x = p.x * scale + imgx;
		s.y = p.y * scale + imgy;
		return s;
	}

	float measurePane::toRadians(float deg)
	{
		return deg*(3.1416/180.0);
	}


	//void measurePane::DrawMarkedLine(wxDC &dc, int tipx, int tipy, int tailx, int taily)
	void measurePane::DrawMarkedLine(wxDC &dc, wxPoint tip, wxPoint tail)
	{
		dc.DrawLine(tip.x,tip.y,tail.x,tail.y);
		
		int alen = 15;
		int dx = tip.x - tail.x;
		int dy = tip.y - tail.y;
		
		double theta = atan2(dy, dx);
		
		float angle = 90;
		if (abs(tip.x - tail.x) > abs(tip.y - tail.y))
			angle = 89.5;
		else
			angle = 90.5;
		
		double rad = toRadians(angle); 
		double x = tip.x - alen * cos(theta + rad);
		double y = tip.y - alen * sin(theta + rad);
		dc.DrawLine(tip.x, tip.y, x, y); //else dc.DrawLine(tailx, taily, x, y);
			
		x = tail.x - alen * cos(theta + rad);
		y = tail.y - alen * sin(theta + rad);
		dc.DrawLine(tail.x, tail.y, x, y);
		
		rad = toRadians(-angle);
		x = tip.x - alen * cos(theta + rad);
		y = tip.y - alen * sin(theta + rad);
		dc.DrawLine(tip.x, tip.y, x, y); //else dc.DrawLine(tailx, taily, x, y);

		x = tail.x - alen * cos(theta + rad);
		y = tail.y - alen * sin(theta + rad);
		dc.DrawLine(tail.x, tail.y, x, y);
	}
	
	void measurePane::render(wxDC &dc) 
	{
		if (!img.IsOk()) return;
		dc.DrawBitmap(wxBitmap(scaledimg), imgx, imgy);
		
		if (measuring) {
			dc.SetPen(wxPen(wxColour(0,0,255),2));
			DrawMarkedLine(dc, getScreenCoord(begin), getScreenCoord(end));
		}
		else if (calibrating) {
			dc.SetPen(wxPen(wxColour(255,0,0),2));
			DrawMarkedLine(dc, getScreenCoord(begin), getScreenCoord(end));
		}
		else if (lastline == MEASURE) {
			dc.SetPen(wxPen(wxColour(0,0,255),2));
			DrawMarkedLine(dc, getScreenCoord(begin), getScreenCoord(end));
		}
		else if (lastline == CALIBRATE) {
			dc.SetPen(wxPen(wxColour(255,0,0),2));
			DrawMarkedLine(dc, getScreenCoord(begin), getScreenCoord(end));
		}
	}
	
	
	void measurePane::OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);
		render(dc);
	}
	
	void measurePane::Resize()
	{
		if (!img.IsOk()) return;
		if (!fit) return;
		int ww, wh;
		GetSize(&ww, &wh);
		int iw = img.GetWidth();
		int ih = img.GetHeight();
		if (fit) {
			if (iw > ih)
				scale = (double) ww/ (double) iw;
			else
				scale = (double) wh/ (double) wh;
			imgx = 0;
			imgy = 0;
		}
		scaledimg = img.Scale(iw*scale, ih*scale);
		Refresh();
		setStatus();
	}
	
	void measurePane::setStatus()
	{
		if (!fit)
			((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %d\%",int(scale*100)),1);
			//((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %d\%,  dim: %d,%d",int(scale*100), imgw, imgh),1);
		else
			((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %d\% (fit)",int(scale*100)),1);
			//((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %d\% (fit),  dim: %d,%d",int(scale*100), imgw, imgh),1);
			
		if (calibration > 0) 
			((wxFrame *) GetParent())->SetStatusText(wxString::Format("calibration: %d pixels", calibration),2);
		
		if (pxmeasurement > 0)
			((wxFrame *) GetParent())->SetStatusText(wxString::Format("measurement: %s", feetinches(measurement)),0);
		
		Refresh();
			
	}
	
	void measurePane::setStatus(wxPoint p)
	{
		if (!img.IsOk()) return;
		int ww, wh;
		GetSize(&ww, &wh);
		int iw = img.GetWidth();
		int ih = img.GetHeight();

		wxPoint i = getImageCoord(p);
		//wxPoint tp = getScreenCoord(i);

		//if (i.x >= 0 & i.x <= ifw & i.y >=0 & i.y <= ifh)
		//	((wxFrame *) GetParent())->SetStatusText(wxString::Format("%d,%d",i.x, i.y));
		//else
		//	((wxFrame *) GetParent())->SetStatusText("");
		
		//((wxFrame *) GetParent())->SetStatusText(wxString::Format("mousepos: %d,%d  imgpos: %d,%d  roundtrip: %d,%d", p.x, p.y, imgx, imgy, tp.x, tp.y),2);

		setStatus();
	}
	
	
	
	void measurePane::mouseLeftDClick(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();
		int ww, wh;
		GetSize(&ww, &wh);
		int iw = img.GetWidth();
		int ih = img.GetHeight();
		
		//mouse position in image coordinates
		int px = (p.x - imgx) / scale;
		int py = (p.y - imgy) / scale;

		if (!fit) {
			if (iw > ih)
				scale = (double) ww/ (double) iw;
			else
				scale = (double) wh/ (double) wh;
			imgx = 0;
			imgy = 0;
			fit = true;
		}
		else {
			scale = 1.0;
			imgx = -px + ww/2;
			imgy = -py + wh/2;
			fit = false;
		}
		scaledimg = img.Scale(iw*scale, ih*scale);
		setStatus(p);
		Refresh();
	}
	
	bool measurePane::smallerThan()
	{
		int ww, wh;
		GetSize(&ww, &wh);
		int iw = img.GetWidth();
		int ih = img.GetHeight();
		if ( iw < ww | ih < wh) return true;
		return false;
	}
	
	void measurePane::mouseLeftDown(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();
		if (event.AltDown()) {
			calibrating = true;
			pxmeasurement = 0; measurement = 0.0;
			begin = end = getImageCoord(p);
			((wxFrame *)GetParent())->SetStatusText("calibrating...");
			lastline = NOLINE;
		}
		else if (event.ControlDown()) {
			if (calibration > 0) {
				measuring = true;
				begin = end = getImageCoord(p);
				((wxFrame *)GetParent())->SetStatusText("measuring...");
				lastline = NOLINE;
			}
		}
		else {
			((wxFrame *)GetParent())->SetStatusText("");
			prevx=pos.x; prevy=pos.y;
			prevx=p.x; prevy=p.y;
			dragging = true;
		}
		Refresh();
	}
	
	void measurePane::mouseMoved(wxMouseEvent& event)
	{
		wxPoint p = event.GetPosition();
		if (calibrating) {
			end = getImageCoord(p);
		}
		else if (measuring) {
			end = getImageCoord(p);
		}
		else {
			if (dragging) {
				fit = false;
				//pos = p;
				imgx -= prevx - p.x;
				imgy -= prevy - p.y;
				Refresh();
			}
			setStatus(p);
			prevx = p.x;
			prevy = p.y;
			
		}
		setStatus();
		Refresh();
	}
	
	void measurePane::mouseReleased(wxMouseEvent& event)
	{
		if (calibrating) {
			lastline = CALIBRATE;
			calibration = distance(begin, end);
			pxmeasurement = 0; measurement = 0.0;
		}
		else if (measuring) {
			lastline = MEASURE;
			pxmeasurement = distance(begin, end);
			measurement = (float) pxmeasurement / (float) calibration;
		}
		((wxFrame *)GetParent())->SetStatusText("");
		calibrating = measuring = dragging = false;
		setStatus();
		Refresh();
	}
	
	void measurePane::OnMouseWheel(wxMouseEvent& event)
	{
		fit = false;
		wxPoint p = event.GetPosition();
		
		int ww, wh;
		GetSize(&ww, &wh);
		int iw = img.GetWidth();
		int ih = img.GetHeight();

		//mouse position in image coordinates
		int px = (p.x - imgx) / scale;
		int py = (p.y - imgy) / scale;

		float increment = 0.01;
		if (event.ShiftDown())
			increment = 0.1;
		//if (event.ControlDown())
		//	increment = 1.0;

		if (event.GetWheelRotation() > 0)
			scale += increment;
		else
			scale -= increment;
		if (scale <= 0.01) scale = 0.01;
		
		if (ww > (iw*scale) & wh > (ih*scale)) {  //image smaller than window
			imgx = ww/2-(iw*scale)/2;
			imgy = wh/2-(ih*scale)/2;
		}
		else {  //image larger than window, dx/dy computed to keep pointed-to place in image positioned in the window
			int dx = px-(px * (1.0+increment));
			int dy = py-(py * (1.0+increment));
		
			if (event.GetWheelRotation() < 0) {
				imgx -= dx;
				imgy -= dy;
			}
			else {
				imgx += dx;
				imgy += dy;
			}
		}
		
		scaledimg = img.Scale(iw*scale, ih*scale);

		setStatus();
		Refresh();
		
	}
	

	void measurePane::setImage(wxImage image)
	{
		scale = 1.0;
		imgx = imgy = 0;
		int ww, wh;
		GetSize(&ww, &wh);
		imgw = ifw = image.GetWidth();
		imgh = ifh = image.GetHeight();
		img = image;
		if (ifw > ifh)
			scale = (double) ww/ (double) ifw;
		else
			scale = (double) wh/ (double) ifh;
		scaledimg = img.Scale(ifw*scale, ifh*scale);
		fit = true;
		setStatus();
		Refresh();
	}
	
	void measurePane::setImageFile(wxFileName f)
	{
		filename = f;

		wxImage img(filename.GetFullPath());
		setImage(img);
		////prop autoupdate 0|1 - enables auto image update if the file changes.  Default: 0
		//if (myConfig::getConfig().getValueOrDefault("autoupdate","0") == "1") {
		//	modtime = filename.GetModificationTime();
		//	//((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %d\%  dim: %d,%d",int(scale*100), imgw, imgh),1);
		//	t.Start(500,wxTIMER_ONE_SHOT);
		//}
	}
	
	void measurePane::refreshImage()
	{
		wxImage image(filename.GetFullPath());
		img = image;
		imgw = img.GetWidth();
		imgh = img.GetHeight();
		scaledimg = img.Scale(imgw*scale, imgh*scale);
		Refresh();
	}
	
	void measurePane::OnTimer(wxTimerEvent& event)
		{
			wxDateTime m = filename.GetModificationTime();
			if (!m.IsEqualTo(modtime)) {
				refreshImage();
				modtime = m;
			}
			t.Start(500,wxTIMER_ONE_SHOT);
			event.Skip();
		}
	
