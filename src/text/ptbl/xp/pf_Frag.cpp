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


#include "ut_types.h"
#include "pf_Frag.h"
#include "pt_PieceTable.h"

pf_Frag::pf_Frag(pt_PieceTable * pPT, PFType type, UT_uint32 length)
{
	m_type = type;
	m_length = length;
	m_next = NULL;
	m_prev = NULL;
	m_pPieceTable = pPT;
    m_pField = NULL;
}

pf_Frag::~pf_Frag()
{
}

pf_Frag * pf_Frag::setNext(pf_Frag * pNext)
{
	pf_Frag * pOld = m_next;
	m_next = pNext;
	return pOld;
}

pf_Frag * pf_Frag::setPrev(pf_Frag * pPrev)
{
	pf_Frag * pOld = m_prev;
	m_prev = pPrev;
	return pOld;
}

UT_Bool pf_Frag::createSpecialChangeRecord(PX_ChangeRecord ** /*ppcr*/,
										   PT_DocPosition /*dpos*/) const
{
	// really this should be declared abstract in pf_Frag,
	// but we didn't derrive a sub-class for EOD -- it actually
	// uses the base class as is.
	
	// this function must be overloaded for all sub-classes.
	
	UT_ASSERT(0);
	return UT_TRUE;
}

fd_Field * pf_Frag::getField(void)
{
    return m_pField;
}
