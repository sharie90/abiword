/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_QNXDIALOG_LISTS_H
#define AP_QNXDIALOG_LISTS_H

#include "ap_Dialog_Lists.h"
class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Lists: public AP_Dialog_Lists
{
public:
	AP_QNXDialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Lists(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			activate();
	virtual void			destroy();
 
	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

    virtual void            notifyActiveFrame(XAP_Frame *pFrame){};

	// CALLBACKS 

	virtual void 		startChanged(void);
	virtual void		stopChanged(void);
	virtual void		applyClicked(void);
	virtual void		startvChanged(void);

	// Just Plain Useful Functions 

	void                            setAllSensitivity(void);
	void                            updateDialog(void);
	static void                     autoupdateLists(UT_Timer * pTimer);

protected:
	PtWidget_t *	_constructWindow(void);
	PtWidget_t *	_constructWindowContents(void);
	void			_populateWindowData(void);
	void			_connectSignals(void);

	UT_Bool     m_bDestroy_says_stopupdating;
	UT_Bool     m_bAutoUpdate_happening_now;
	UT_Timer *  m_pAutoUpdateLists;

	PtWidget_t *m_wMainWindow;

	PtWidget_t * m_wApply;
	PtWidget_t * m_wClose;
	PtWidget_t * m_wContents;
	PtWidget_t * m_wCheckstartlist;
	PtWidget_t * m_wCheckstoplist;
	PtWidget_t * m_wNewlisttypel;
    PtWidget_t * m_wOption_types;
    PtWidget_t * m_wOption_types_menu;
	PtWidget_t * m_wNew_startingvaluel;
	PtWidget_t * m_wNew_startingvaluev;
	PtWidget_t * m_wNew_list_labell;
	PtWidget_t * m_wnew_list_labelv;
	PtWidget_t * m_wCur_listtype;
	PtWidget_t * m_wCur_listtypev;
	PtWidget_t * m_wCur_listlabel;
	PtWidget_t * m_wCur_listlabelv;
    PtWidget_t * m_wCur_changestart_button;
	PtWidget_t * m_wCur_startingvaluel;
	PtWidget_t * m_wCur_startingvaluev;

protected:

};

#endif /* AP_QNXDIALOG_LISTS_H */






