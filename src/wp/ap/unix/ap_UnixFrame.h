/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_UNIXFRAME_H
#define AP_UNIXFRAME_H

class GR_Graphics;

#ifdef HAVE_GNOME
#include "xap_UnixGnomeFrame.h"
#define XAP_UNIXBASEFRAME XAP_UnixGnomeFrame
#else
#include "xap_UnixFrame.h"
#define XAP_UNIXBASEFRAME XAP_UnixFrame
#endif

#include "ie_types.h"

/*****************************************************************/

class AP_UnixFrame : public XAP_UNIXBASEFRAME
{
public:
	AP_UnixFrame(XAP_UnixApp * app);
	AP_UnixFrame(AP_UnixFrame * f);
	virtual ~AP_UnixFrame(void);

	virtual UT_Bool				initialize(void);
	virtual	XAP_Frame *			cloneFrame(void);
	virtual UT_Error   			loadDocument(const char * szFilename, int ieft);
	virtual UT_Bool				initFrameData(void);
	virtual void				killFrameData(void);

	virtual void				setXScrollRange(void);
	virtual void				setYScrollRange(void);
	virtual void				translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y);
	virtual void				setZoomPercentage(UT_uint32 iZoom);
	virtual UT_uint32			getZoomPercentage(void);
	virtual void				setStatusMessage(const char * szMsg);

	virtual void				toggleRuler(UT_Bool bRulerOn);
	virtual void				toggleBar(UT_uint32 iBarNb, UT_Bool bBarOn);
	virtual void				toggleStatusBar(UT_Bool bStatusBarOn);

protected:
	virtual GtkWidget *			_createDocumentWindow(void);
	virtual GtkWidget *			_createStatusBarWindow(void);
	virtual void				_setWindowIcon(void);
	UT_Error   					_loadDocument(const char * szFilename, IEFileType ieft);
	UT_Error   					_showDocument(UT_uint32 iZoom=100);
	static void					_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 xlimit);
	static void					_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 ylimit);
	UT_Error					_replaceDocument(AD_Document * pDoc);
	virtual void				_showOrHideToolbars(void);
	virtual void				_showOrHideStatusbar(void);
	
	GtkAdjustment *				m_pVadj;
	GtkAdjustment *				m_pHadj;
	GtkWidget *					m_hScroll;
	GtkWidget *					m_vScroll;
	GtkWidget *					m_dArea;
	GtkWidget *					m_table;
	GtkWidget *					m_innertable;
	GtkWidget *					m_topRuler;
	GtkWidget *					m_leftRuler;
	GtkWidget *					m_wSunkenBox;
};

#endif /* AP_UNIXFRAME_H */
