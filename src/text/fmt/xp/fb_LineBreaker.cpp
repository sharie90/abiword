/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include <stdlib.h>

#include "fb_LineBreaker.h"
#include "fl_BlockLayout.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_TextRun.h"
#include "fp_Column.h"

#include "ut_assert.h"

fb_LineBreaker::fb_LineBreaker()
{
}

// LineBreaker shouldn't break a line until it finds a non-blank
// item passed the end of the line.
// All trailing spaces should remain on the end of the line.

UT_sint32 fb_LineBreaker::breakParagraph(fl_BlockLayout* pBlock)
{
	fp_Line* pLine = pBlock->getFirstLine();
	UT_ASSERT(pLine);

	while (pLine)
	{
		if (pLine->countRuns() > 0)
		{
			m_pFirstRunToKeep = pLine->getFirstRun();
			m_pLastRunToKeep = NULL;
			
			m_iMaxLineWidth = pLine->getMaxWidth();
			m_iWorkingLineWidth = 0;
			
//			UT_Bool bFoundBreakAfter = UT_FALSE;
//			UT_Bool bFoundSplit = UT_FALSE;
			
//			fp_TextRun* pRunToSplit = NULL;
//			fp_TextRun* pOtherHalfOfSplitRun = NULL;
			
			fp_Run* pOffendingRun = NULL;
			
			fp_Run* pCurrentRun = m_pFirstRunToKeep;
			fp_Run* pPreviousRun;

			while (UT_TRUE)
			{
				// if this run is past the end of the line... 

				UT_Bool bRunIsNonBlank = UT_TRUE;
				if(pCurrentRun)
				{
					if(!pCurrentRun->doesContainNonBlankData())
					{
						bRunIsNonBlank = UT_FALSE;
					}
				}
				
				if (bRunIsNonBlank && (m_iWorkingLineWidth  > m_iMaxLineWidth))
				{
					// This is the first run which will start past the end of the line

					UT_ASSERT(pPreviousRun);
					UT_sint32 iTrailingSpace = _moveBackToFirstNonBlankData(pPreviousRun, &pOffendingRun);

					m_iWorkingLineWidth -= iTrailingSpace;
					if (m_iWorkingLineWidth > m_iMaxLineWidth)
					{
						// This run needs splitting.

						UT_ASSERT(pOffendingRun);

						_splitAtOrBeforeThisRun(pOffendingRun);
						goto done_with_run_loop;
					}
					else
					{
						if(pCurrentRun)
						{
							_splitAtNextNonBlank(pCurrentRun);
						}
						goto done_with_run_loop;
					}
				}
				if(!pCurrentRun)
					break;

				m_iWorkingLineWidth += pCurrentRun->getWidth();
			
				unsigned char iCurRunType = pCurrentRun->getType();

				switch (iCurRunType)
				{
				case FPRUN_FORCEDCOLUMNBREAK:
				{
					m_pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}
				case FPRUN_FORCEDPAGEBREAK:
				{
					m_pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}
				case FPRUN_FORCEDLINEBREAK:
				{
					m_pLastRunToKeep = pCurrentRun;
					goto done_with_run_loop;
				}
				case FPRUN_TAB:
				{
					/*
					  find the position of this tab and its type.
					  if it's a left tab, then add its width to the m_iWorkingLineWidth
					*/

					UT_sint32 iPos;
					unsigned char iType;

					UT_Bool bRes = pLine->findNextTabStop(m_iWorkingLineWidth, iPos, iType);
					if (bRes)
					{
						UT_ASSERT(iType == FL_TAB_LEFT);	// TODO right and center tabs

						UT_ASSERT(iPos > m_iWorkingLineWidth);
					
						m_iWorkingLineWidth = iPos;
					}
					else
					{
						// tab won't fit.  bump it to the next line
						UT_ASSERT(pCurrentRun->getPrev());
						UT_ASSERT(pCurrentRun != m_pFirstRunToKeep);
						
						m_pLastRunToKeep = pCurrentRun->getPrev();
						goto done_with_run_loop;
					}
					break;
				}
				case FPRUN_FMTMARK:
					break;
					
				case FPRUN_FIELD:
				case FPRUN_IMAGE:
				case FPRUN_TEXT:
				{

					break;
				}
				default:
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				} /* switch */
			
				pPreviousRun = pCurrentRun;
				pCurrentRun = pCurrentRun->getNext();
			} /* the run loop */

		done_with_run_loop:
			
			/*
			  OK, we've gone through the run loop.  If a run was to
			  be split, it has already been split.  m_pLastRunToKeep
			  should now be set to the last run which should be on
			  this line.  We need to make sure that all runs from
			  the first one on the line up until m_pLastRunToKeep are
			  actually on this line.  Furthermore, we need to make
			  sure that no other runs are on this line.
			*/

			_breakTheLineAtLastRunToKeep(pLine, pBlock);

			
			/*
			  Now we know all the runs which belong on this line.
			  However, those runs are not properly positioned.  We
			  call the line to do the actual layout.
			*/

			pLine->layout();

		} /* if countruns > 0 */
		
		pLine = pLine->getNext();
	}

	return 0; // TODO return code
}

UT_sint32 fb_LineBreaker::_moveBackToFirstNonBlankData(fp_Run *pCurrentRun, fp_Run **pOffendingRun)
{
	// need to move back untill we find the first non blank character and
	// return the distance back to this character.

	UT_sint32 iTrailingBlank = 0;

	do
	{
		if(!pCurrentRun->doesContainNonBlankData())
		{
			iTrailingBlank += pCurrentRun->getWidth();
		}
		else
		{
			iTrailingBlank += pCurrentRun->findTrailingSpaceDistance();
			break;
		}
		
		pCurrentRun = pCurrentRun->getPrev();
	}
	while(pCurrentRun);


	*pOffendingRun = pCurrentRun;

	return iTrailingBlank;
}


UT_Bool fb_LineBreaker::_splitAtOrBeforeThisRun(fp_Run *pCurrentRun)
{

	// This run is past the end of the line.

	// Reminder: m_iWorkingLineWidth = Length including this run minus any trailing spaces in this run.

	// Set m_iWorkingLineWidth to length including this run.

	m_iWorkingLineWidth += pCurrentRun->findTrailingSpaceDistance();
	m_iWorkingLineWidth -= pCurrentRun->getWidth();


	fp_Run *pOffendingRun = pCurrentRun;
	fp_RunSplitInfo splitInfo;
	fp_TextRun *pRunToSplit = NULL;

	UT_Bool bFoundBreakAfter = UT_FALSE;

	UT_Bool bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(m_iMaxLineWidth - m_iWorkingLineWidth, splitInfo);
	if (bFoundSplit)
	{
		UT_ASSERT(pOffendingRun->getType() == FPRUN_TEXT);
		pRunToSplit = (fp_TextRun*) pOffendingRun;
	}
	else
	{
			
		/*
		  The run we wanted to split (the one which pushes
		  this line over the limit) cannot be split.  We need
		  to work backwards along the line to find a split
		  point.  As we stop at each run along the way, we'll
		  first check to see if we can break the line after
		  that run.  If not, we'll try to split that run.
		*/

		fp_Run* pRunLookingBackwards = pCurrentRun;
		while (pRunLookingBackwards != m_pFirstRunToKeep)
		{
			pRunLookingBackwards = pRunLookingBackwards->getPrev();

			if (pRunLookingBackwards->canBreakAfter())
			{
				/*
				  OK, we can break after this
				  run.  Move all the runs after this one
				  onto the next line.
				*/

				bFoundBreakAfter = UT_TRUE;
				m_pLastRunToKeep = pRunLookingBackwards;

				break;
			}
			else
			{
				/*
				  Can't break after this run.  Let's
				  see if we can split this run to get
				  something which will fit.
				*/
				bFoundSplit = pRunLookingBackwards->findMaxLeftFitSplitPoint(pRunLookingBackwards->getWidth(), splitInfo);

				if (bFoundSplit)
				{
					// a suitable split was found.

					UT_ASSERT(pRunLookingBackwards->getType() == FPRUN_TEXT);
					
					pRunToSplit = (fp_TextRun*) pRunLookingBackwards;
					break;
				}
			}
		}
	}

	if (!(bFoundSplit || bFoundBreakAfter))
	{
		/*
		  OK.  There are no valid break points on this line,
		  anywhere.  We can't break after any of the runs, nor
		  can we split any of the runs.  We're going to need
		  to force a split of the Offending Run.
		*/

		bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(m_iMaxLineWidth - m_iWorkingLineWidth, splitInfo, UT_TRUE);
		if (bFoundSplit)
		{
			UT_ASSERT(pOffendingRun->getType() == FPRUN_TEXT);
			pRunToSplit = (fp_TextRun*) pOffendingRun;
		}
		else
		{
			/*
			  Wow!  This is a very resilient run.  It is the
			  run which no longer fits, and yet it cannot be
			  split.  It might be a single-character run.
			  Perhaps it's an image.  Anyway, we still have to
			  try as hard as we can to find a line break.
			*/

			if (pOffendingRun != m_pFirstRunToKeep)
			{
				/*
				  Force a break right before the offending run.
				*/
				m_pLastRunToKeep = pOffendingRun->getPrev();

				bFoundBreakAfter = UT_TRUE;
			}
			else
			{
				// nothing else we can do but this.
				m_pLastRunToKeep = pOffendingRun;
				bFoundBreakAfter = UT_TRUE;
			}
		}
	}

	if (bFoundSplit)
	{
		UT_ASSERT(!bFoundBreakAfter);

		_splitRunAt(pRunToSplit, splitInfo);

	}
	

/*
		// Previous line may be at fault.

		pOffendingRun = pCurrentRun->getPrev();
		UT_ASSERT(pOffendingRun);

		iTrailingSpace = pOffendingRun->findTrailingSpaceDistance();

		if (m_iWorkingLineWidth - iTrailingSpace > m_iMaxLineWidth)
		{
			// This run needs splitting.

			do
			{
				UT_sint32 iOffending_run_length = pOffendingRun->getWidth();

				if(pOffendingRun->splitRunToShortenBy(m_iMaxLineWidth - m_iWorkingLineWidth))
				{
					// run was split ok.

					m_pLastRunToKeep = pOffendingRun;
					goto done_with_run_loop;
				}
				else
				{
					// run could not be split.

					// Can we break before this run.

					if(pOffendingRun->canBreakBefore())
					{
						m_pLastRunToKeep = pOffendingRun->getPrev();

						UT_ASSERT(m_pLastRunToKeep);
						goto done_with_run_loop;
					}
					else
					{
						pOffendingRun = pOffendingRun->getPrev();
					}
				}
			}
			while(pOffendingRun != m_pFirstRunToKeep);
		
			m_pLastRunToKeep = m_pFirstRunToKeep;	// Cannot shorten run at all.
			goto done_with_run_loop;
		}
	}
*/

	return UT_TRUE;
}

UT_Bool fb_LineBreaker::_splitAtNextNonBlank(fp_Run *pCurrentRun)
{
	fp_RunSplitInfo splitInfo;
	UT_Bool	bCanSplit = pCurrentRun->findFirstNonBlankSplitPoint(splitInfo);

	if(bCanSplit)
	{
		_splitRunAt(pCurrentRun, splitInfo);
	}
	else
	{
		// cannot split run so split before.

		m_pLastRunToKeep = pCurrentRun->getPrev();
	}
	
	return UT_TRUE;
}

void fb_LineBreaker::_splitRunAt(fp_Run *pCurrentRun, fp_RunSplitInfo &splitInfo)
{
	UT_ASSERT(pCurrentRun->getType() == FPRUN_TEXT);
	fp_TextRun *pRunToSplit = (fp_TextRun*) pCurrentRun;

	pRunToSplit->split(splitInfo.iOffset + 1);	// TODO err check this
	UT_ASSERT(pRunToSplit->getNext());
	UT_ASSERT(pRunToSplit->getNext()->getType() == FPRUN_TEXT);
	
	fp_TextRun *pOtherHalfOfSplitRun = (fp_TextRun*) pRunToSplit->getNext();

// todo decide if we need to call recalcWidth() now on the 2 pieces.
//							pRunToSplit->recalcWidth();
//							pOtherHalfOfSplitRun->recalcWidth();
	
	UT_ASSERT((UT_sint32)pRunToSplit->getWidth() == splitInfo.iLeftWidth);

	UT_ASSERT(pOtherHalfOfSplitRun);
	UT_ASSERT(pOtherHalfOfSplitRun->getLine() == pRunToSplit->getLine());
	m_pLastRunToKeep = pRunToSplit;
}

void fb_LineBreaker::_breakTheLineAtLastRunToKeep(fp_Line *pLine, 
													fl_BlockLayout *pBlock)
{

	/*
	  If m_pLastRunToKeep is NULL here, that means that
	  all remaining runs in this block will fit on this
	  line.
	*/

	
	fp_Run *pCurrentRun = m_pFirstRunToKeep;
	while (pCurrentRun)
	{
		if (pCurrentRun->getLine() != pLine)
		{
			fp_Line* pOtherLine = pCurrentRun->getLine();
			UT_ASSERT(pOtherLine);

			pOtherLine->removeRun(pCurrentRun);
			pLine->addRun(pCurrentRun);
		}

		if (pCurrentRun == m_pLastRunToKeep)
		{
			break;
		}
		else
		{
			pCurrentRun = pCurrentRun->getNext();
		}
	}

	fp_Line* pNextLine = NULL;

	if (
		m_pLastRunToKeep
		&& (pLine->getLastRun() != m_pLastRunToKeep)
		)
	{
		// make sure there is a next line
		pNextLine = pLine->getNext();
		if (!pNextLine)
		{
			fp_Line* pNewLine  = pBlock->getNewLine();
			UT_ASSERT(pNewLine);	// TODO check for outofmem
					
			pNextLine = pNewLine;
		}

		fp_Run* pRunToBump = pLine->getLastRun();
		while (pLine->getLastRun() != m_pLastRunToKeep)
		{
			UT_ASSERT(pRunToBump->getLine() == pLine);

			pLine->removeRun(pRunToBump);
			pNextLine->insertRun(pRunToBump);

			pRunToBump = pRunToBump->getPrev();
		}
	}

	UT_ASSERT((!m_pLastRunToKeep) || (pLine->getLastRun() == m_pLastRunToKeep));
}
















#if 0

					/* if this run is past the end of the line... */
					if (
						((m_iWorkingLineWidth + pCurrentRun->getWidth()) > m_iMaxLineWidth)
						)
					{
						// This is the first run which doesn't fit on the line
						pOffendingRun = pCurrentRun;
					
						bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(m_iMaxLineWidth - m_iWorkingLineWidth, si);
						if (bFoundSplit)
						{
							UT_ASSERT(pOffendingRun->getType() == FPRUN_TEXT);
							pRunToSplit = (fp_TextRun*) pOffendingRun;
						}
						else
						{
							/*
							  The run we wanted to split (the one which pushes
							  this line over the limit) cannot be split.  We need
							  to work backwards along the line to find a split
							  point.  As we stop at each run along the way, we'll
							  first check to see if we can break the line after
							  that run.  If not, we'll try to split that run.
							*/

							fp_Run* pRunLookingBackwards = pCurrentRun;
							while (pRunLookingBackwards != m_pFirstRunToKeep)
							{
								pRunLookingBackwards = pRunLookingBackwards->getPrev();

								if (pRunLookingBackwards->canBreakAfter())
								{
									/*
									  OK, we can break after this
									  run.  Move all the runs after this one
									  onto the next line.
									*/

									bFoundBreakAfter = UT_TRUE;
									m_pLastRunToKeep = pRunLookingBackwards;

									break;
								}
								else
								{
									/*
									  Can't break after this run.  Let's
									  see if we can split this run to get
									  something which will fit.
									*/
									bFoundSplit = pRunLookingBackwards->findMaxLeftFitSplitPoint(pRunLookingBackwards->getWidth(), si);

									if (bFoundSplit)
									{
										// a suitable split was found.

										UT_ASSERT(pRunLookingBackwards->getType() == FPRUN_TEXT);
										
										pRunToSplit = (fp_TextRun*) pRunLookingBackwards;
										break;
									}
								}
							}
						}

						if (!(bFoundSplit || bFoundBreakAfter))
						{
							/*
							  OK.  There are no valid break points on this line,
							  anywhere.  We can't break after any of the runs, nor
							  can we split any of the runs.  We're going to need
							  to force a split of the Offending Run.
							*/

							bFoundSplit = pOffendingRun->findMaxLeftFitSplitPoint(m_iMaxLineWidth - m_iWorkingLineWidth, si, UT_TRUE);
							if (bFoundSplit)
							{
								UT_ASSERT(pOffendingRun->getType() == FPRUN_TEXT);
								pRunToSplit = (fp_TextRun*) pOffendingRun;
							}
							else
							{
								/*
								  Wow!  This is a very resilient run.  It is the
								  run which no longer fits, and yet it cannot be
								  split.  It might be a single-character run.
								  Perhaps it's an image.  Anyway, we still have to
								  try as hard as we can to find a line break.
								*/

								if (pOffendingRun != m_pFirstRunToKeep)
								{
									/*
									  Force a break right before the offending run.
									*/
									m_pLastRunToKeep = pOffendingRun->getPrev();

									bFoundBreakAfter = UT_TRUE;
								}
								else
								{
									// nothing else we can do but this.
									m_pLastRunToKeep = pOffendingRun;
									bFoundBreakAfter = UT_TRUE;
								}
							}
						}

						if (bFoundSplit)
						{
							UT_ASSERT(!bFoundBreakAfter);
				
							pRunToSplit->split(si.iOffset + 1);	// TODO err check this
							UT_ASSERT(pRunToSplit->getNext());
							UT_ASSERT(pRunToSplit->getNext()->getType() == FPRUN_TEXT);
							
							pOtherHalfOfSplitRun = (fp_TextRun*) pRunToSplit->getNext();

// todo decide if we need to call recalcWidth() now on the 2 pieces.
//							pRunToSplit->recalcWidth();
//							pOtherHalfOfSplitRun->recalcWidth();
							
							UT_ASSERT((UT_sint32)pRunToSplit->getWidth() == si.iLeftWidth);

							UT_ASSERT(pOtherHalfOfSplitRun);
							UT_ASSERT(pOtherHalfOfSplitRun->getLine() == pRunToSplit->getLine());
							m_pLastRunToKeep = pRunToSplit;
						}
						
						goto done_with_run_loop;
						
					} /* if this run doesn't fit on the line */

#endif