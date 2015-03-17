// ImageEditorView.cpp : implementation of the CImageEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#include "ImageEditorView.h"

#include <algorithm>
#include "ImageEditor/BasicElements.h"
#include <GdiPlus.h>
#include <Gui/GuiTools.h>
#include <Core/Logging.h>
#include <Core/Images/Utils.h>
#include "../MovableElements.h"
#include "resource.h"

#ifndef TR
#define TR(a) L##a
#endif
namespace ImageEditor {

	CImageEditorView::CImageEditorView()  {
	oldPoint.x = -1;
	oldPoint.y = -1;
}

BOOL CImageEditorView::PreTranslateMessage(MSG* /*pMsg*/) {
	return FALSE;
}

LRESULT CImageEditorView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	//return 0;
	CPaintDC dc(m_hWnd);
	CRgn rgn;
	rgn.CreateRectRgn( 0, 0, 0, 0 );
	GetClipRgn( dc, rgn);
	RECT rct;
	rgn.GetRgnBox( &rct );
	Gdiplus::Graphics gr(dc);
	if ( canvas_ ) {
		canvas_->render( &gr, dc.m_ps.rcPaint );
	}
	//horizontalToolbar_.Invalidate(TRUE);

	return 0;
}


LRESULT CImageEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

void CImageEditorView::setCanvas(ImageEditor::Canvas *canvas) {
	canvas_ = canvas;
	if ( canvas ) {
		canvas_->setSize( 1280, 720 );
		canvas_->setCallback( this );
		canvas_->setDrawingToolType(Canvas::dtRectangle);
	}
}

LRESULT CImageEditorView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	POINT pt = {cx, cy};
	LOG(INFO) <<"x=" << cx <<"y=" << cy;
	/*TextElement *textElement = < canvas_->getCurrentlyEditedTextElement();
	if ( textElement ) {
		int elX = textElement->getX();
		int elY = textElement->getY();
		RECT rc  = {elX, elY, elX + textElement->getWidth(), elY + textElement->getHeight()};
		if ( textElement && PtInRect(&rc, pt) ) {
			InputBoxControl* inputBoxControl = dynamic_cast<InputBoxControl*>(textElement->getInputBox());
			inputBoxControl->SendMessage(uMsg, MAKEWPARAM(cx - elX, cy - elY), lParam);
		}
		return 0;
	}*/
	/*RECT toolBarRect;
	horizontalToolbar_.GetClientRect(&toolBarRect);
	horizontalToolbar_.ClientToScreen(&toolBarRect);
	ClientToScreen(&pt);*/
	/*if ( pt.x >= toolBarRect.left && pt.x <= toolBarRect.right && pt.y >= toolBarRect.top && pt.y <= toolBarRect.bottom ) {
		return 0;
	}*/
//	HWND wnd =  WindowFromPoint(pt);
	/*if ( wnd == m_hWnd )*/ {
		canvas_->mouseMove( cx, cy, wParam );
	}
	return 0;
}

LRESULT CImageEditorView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	POINT pt = {cx, cy};
	/*TextElement *textElement = canvas_->getCurrentlyEditedTextElement();
	if ( textElement ) {
		int elX = textElement->getX();
		int elY = textElement->getY();
		RECT rc  = {elX, elY, elX + textElement->getWidth(), elY + textElement->getHeight()};
		if ( textElement && PtInRect(&rc, pt) ) {
			InputBoxControl* inputBoxControl = dynamic_cast<InputBoxControl*>(textElement->getInputBox());
			inputBoxControl->SendMessage(uMsg, MAKEWPARAM(cx - elX, cy - elY), lParam);
		}
		return 0;
	}*/

	SetCapture();
	//horizontalToolbar_.ShowWindow(SW_HIDE);
	canvas_->mouseDown( 0, cx, cy );
	return 0;
}

LRESULT CImageEditorView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	POINT pt = {cx, cy};
	/*TextElement *textElement = canvas_->getCurrentlyEditedTextElement();
	if ( textElement ) {
		int elX = textElement->getX();
		int elY = textElement->getY();
		RECT rc  = {elX, elY, elX + textElement->getWidth(), elY + textElement->getHeight()};
		if ( textElement && PtInRect(&rc, pt) ) {
			InputBoxControl* inputBoxControl = dynamic_cast<InputBoxControl*>(textElement->getInputBox());
			inputBoxControl->SendMessage(uMsg, MAKEWPARAM(cx - elX, cy - elY), lParam);
		}
		return 0;
	}*/
	canvas_->mouseUp( 0, cx, cy );
	ReleaseCapture();
//	horizontalToolbar_.ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CImageEditorView::OnLButtonDblClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	canvas_->mouseDoubleClick( 0, cx, cy );
	return 0;
}

void CImageEditorView::updateView( Canvas* canvas, const CRgn& region ) {
	InvalidateRgn( region );
	RECT rc;
	region.GetRgnBox( &rc );
	//InvalidateRect( &boundingRect );
}

LRESULT CImageEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	return 1;
}

LRESULT CImageEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	HWND 	hwnd = (HWND) wParam;  
	POINT ClientPoint, ScreenPoint;

	if(lParam == -1) 
	{
		ClientPoint.x = 0;
		ClientPoint.y = 0;
		ScreenPoint = ClientPoint;
		::ClientToScreen(hwnd, &ScreenPoint);
	}
	else
	{
		ScreenPoint.x = LOWORD(lParam); 
		ScreenPoint.y = HIWORD(lParam); 
		ClientPoint = ScreenPoint;
		::ScreenToClient(hwnd, &ClientPoint);
	}
	
	/*CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_UNDO, TR("��������"));
	menu.AppendMenu(MF_STRING, ID_PEN, TR("��������"));
	menu.AppendMenu(MF_STRING, ID_BRUSH, TR("�����"));
	menu.AppendMenu(MF_STRING, ID_LINE, TR("�����"));
	menu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("�������������"));
	menu.AppendMenu(MF_STRING, ID_TEXT, TR("�������� �����"));
	menu.AppendMenu(MF_STRING, ID_CROP, TR("�������"));

	//menu.SetMenuDefaultItem(0, true);
	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);*/

	return 0;
}


LRESULT CImageEditorView::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	SetCursor(getCachedCursor(canvas_->getCursor()));
	return 0;
}


HCURSOR CImageEditorView::getCachedCursor(CursorType cursorType)
{
	HCURSOR cur = cursorCache_[cursorType];
	if ( cur ) {
		return cur;
	}
	LPCTSTR lpCursorName = 0;
	switch( cursorType ) {
		case ctEdit:
			lpCursorName = IDC_IBEAM;
			break;
		case ctResizeVertical:
			lpCursorName = IDC_SIZENS;
			break;
		case ctResizeHorizontal:
			lpCursorName = IDC_SIZEWE;
			break;
		case ctResizeDiagonalMain:
			lpCursorName = IDC_SIZENWSE;
			break;
		case ctResizeDiagonalAnti:
			lpCursorName = IDC_SIZENESW;
			break;
		case ctCross:
			lpCursorName = IDC_CROSS;
			break;
		case ctMove:
			lpCursorName = IDC_SIZEALL;
			break;
		default:
			lpCursorName = IDC_ARROW;
	}
	cur = LoadCursor(0, lpCursorName);
	cursorCache_[cursorType] = cur;
	return cur;
}

}
