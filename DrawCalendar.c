/*	File:		DrawCalendar.c	Contains:	Module for displaying the calendar (Gregorian).	Written by:	Martin Minow	Copyright:	� 1994-1997 by Apple Computer, Inc., all rights reserved.	Change History (most recent first):	You may incorporate this sample code into your applications without	restriction, though the sample code has been provided "AS IS" and the	responsibility for its operation is 100% yours.  However, what you are	not permitted to do is to redistribute the source as "DSC Sample Code"	after having made changes. If you're going to re-distribute the source,	we require that you make it clear in the source that the code was	descended from Apple Sample Code, but that you've made changes.	DrawCalendar displays the calendar (Gregorian) for the selected date. The	algorithm has been simplified and consequently will only work for dates within	the Macintosh epoch.	Note that DrawCalendar is common to the Control Strip and Setup application.	If you change it, you must rebuild both modules.*//////////////////////////////////////////////////////////////////////////// Pick up some common stuff, specifically the height and width macros.#include "MacCalendarCommon.h"/////////////////////////////////////////////////////////////////////////// Pick up prototypes for our exported routines.#include "DrawCalendar.h"/////////////////////////////////////////////////////////////////////////// Pick up system types.#include <Fonts.h>#include <IntlResources.h>#include <Memory.h>#include <OSUtils.h>#include <Packages.h>#include <QuickDraw.h>#include <Script.h>#include <TextUtils.h>/////////////////////////////////////////////////////////////////////////// Some global constants.#define	kFebruary		2						/* The magic month					*//* * This character vector contains the number of days in a month. Because compilers * do not necessarily allow pc-relative addressing of generic vectors, we define * it as a character string. *  034		28 *	035		29 *	036		30 *	037		31 */					/*    Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec			*/#define kDayInMonth "\000\037\034\037\036\037\036\037\037\036\037\036\037"/////////////////////////////////////////////////////////////////////////static short GetFontNumber(		SavedSettingsHandle		settings		/* Current font etc			*/	){		Str255					fontName;		short					fontNumber;		pstrcpy(fontName, (**settings).fontName);		GetFNum(fontName, &fontNumber);		return (fontNumber);}/////////////////////////////////////////////////////////////////////////// GetCalendarDisplaySize// Return the width and height of a rectangle needed to display the calendar in the// specified font and font size. This is needed to draw the calendar and to position// the calendar display with respect to the Control Strip.Point						GetCalendarDisplaySize(		SavedSettingsHandle		settings		/* Current font etc			*/	){		FontInfo				fontInfo;		short					dateWidth;		short					lineHeight;		Point					result;		short					saveTextFont;		short					saveTextSize;		short					saveTextFace;		GrafPtr					currentPort;				GetPort(&currentPort);		saveTextFont = currentPort->txFont;		saveTextSize = currentPort->txSize;		saveTextFace = currentPort->txFace;		TextFont(GetFontNumber(settings));		TextSize((**settings).fontSize);		TextFace(normal);		GetFontInfo(&fontInfo);		lineHeight = fontInfo.ascent + fontInfo.descent + fontInfo.leading;		dateWidth = StringWidth("\p 00");		result.h = (dateWidth * 7) + 4;		/* 7 == days in the week				*/		result.v = (lineHeight * 8) + 4;	/* 8 == lines of text in the calendar	*/		TextFont(saveTextFont);		TextSize(saveTextSize);		TextFace(saveTextFace);		return (result);}/////////////////////////////////////////////////////////////////////////// GetCalendarMonthRect//// Given a display rectangle, font, and font size, create the actual display area.void						GetCalendarMonthRect(		SavedSettingsHandle		settings,		/* Current font etc			*/		const Rect				*displayRect,	/* Where to draw the text	*/		Rect					*monthRect		/* Returns drawing rect		*/	){		Point					monthSize;				monthSize = GetCalendarDisplaySize(settings);		/*		 * Center the month rectangle within the drawing rectangle.		 * >> 1 is used to divide by two without loading a library routine.		 */		monthRect->left = displayRect->left					+ ((width(*displayRect) - monthSize.h) >> 1);		monthRect->top = displayRect->top					+ ((height(*displayRect) - monthSize.v) >> 1);		monthRect->right = monthRect->left + monthSize.h;		monthRect->bottom = monthRect->top + monthSize.v;}/////////////////////////////////////////////////////////////////////////// DrawCalendar//// Draw the month - the port is set to the drawing port. Text font, size, and style// are not preserved. dayName is a Pascal string with the following format, repeated// seven times, once for each day://	{ nByte, Byte1, Byte2, etc }. For example, if dates are represented// by "S M Tu W Th F S", dayName would be specified in a pascal string as//		"\p\001S\001M\002Tu\001W\002Th\001F\001S\000"// Note: the day names must correspond to the firstDayOfWeek parameter. I.e. if the// first day is Monday, the first word in the string is "M".void						DrawCalendar(		SavedSettingsHandle		settings,		/* Current font etc			*/		short					year,			/* 1904 ..					*/		short					month,			/* January == 1				*/		const Rect				*displayRect	/* Where to draw the text	*/	){		DateTimeRec				now;		unsigned long			nowSeconds;		short					weekday;		short					daysInMonth;		short					today;		short					lineHeight;		short					dateWidth;		short					spaceWidth;		short					digitWidth;		FontInfo				fontInfo;		Rect					monthRect;		short					hPos;		short					vPos;		register unsigned char	*dayNamePtr;		short					dayWidth;		Intl1Hndl				intlHdl;		Str255					work;		Boolean					isThisMonth;		short					thisDate;		Rect					dayRect;		PenState				penState;		short					hOffset;		short					vOffset;		short					penSize;				/*		 * Get the drawing parameters for this month. This duplicates the logic of		 * GetCalendarDisplaySize above.		 */		GetCalendarMonthRect(settings, displayRect, &monthRect);		TextFont(GetFontNumber(settings));		TextSize((**settings).fontSize);		TextFace(normal);		GetFontInfo(&fontInfo);		lineHeight = fontInfo.ascent + fontInfo.descent + fontInfo.leading;		spaceWidth = CharWidth(' ');		digitWidth = CharWidth('0');		dateWidth = spaceWidth + (digitWidth * 2);		/*		 * If we're displaying the current month, we want to hilite today's date.		 * 1.0d3		 */		GetDateTime(&nowSeconds);		SecondsToDate(nowSeconds, &now);		isThisMonth = (year == now.year && month == now.month);		thisDate = now.day;		/*		 * Get the parameters for this particular month. We convert day 1 to seconds,		 * then back to the date in order to locate the weekday corresponding to the		 * first day of the month.		 */		now.year = year;		now.month = month;		now.day = 1;		now.hour = 0;		now.minute = 0;		now.second = 0;		DateToSeconds(&now, &nowSeconds);		SecondsToDate(nowSeconds, &now);		vPos = monthRect.top + 2 + fontInfo.ascent;		/*		 * Draw the month and year names.		 */		intlHdl = (Intl1Hndl) GetIntlResource(1);		if (intlHdl != NULL) {			/*			 * 1.0d4: Don't modify the actual data.			 */			if (HandToHand((Handle *) &intlHdl) == noErr) {				/*				 * Convert the date to "month, year" (myd + supress day)				 */				(**intlHdl).suppressDay = 3;				IUDatePString(nowSeconds, myd, work, (Handle) intlHdl);				hPos = monthRect.left + 2					+ ((width(monthRect) - StringWidth(work)) >> 1);				MoveTo(hPos, vPos);				DrawString(work);				vPos += lineHeight;			}			DisposeHandle((Handle) intlHdl);		}			/*		 * Draw the days in the week. dayName is a vector of Pascal strings hiding		 * inside a Pascal string.		 */		pstrcpy(work, (**settings).dayNameString);		dayNamePtr = (unsigned char *) &work[1];		hPos = monthRect.left + 2;		TextFace(bold);										/* 1.0d3				*/		for (weekday = 0; weekday < 7; weekday++) {			dayWidth = StringWidth((StringPtr) dayNamePtr);			MoveTo(hPos + dateWidth - dayWidth, vPos);			DrawString((StringPtr) dayNamePtr);			dayNamePtr += dayNamePtr[0] + 1;			hPos += dateWidth;		}		TextFace(normal);									/* 1.0d3				*/		vPos += lineHeight;		/*		 * How far do we go in this month, with a leap year hack.		 */		daysInMonth = kDayInMonth[month];		if ((year & 0x3) == 0 && month == kFebruary)			++daysInMonth;		/*		 * now.dayOfWeek is the weekday corresponding to the first day of the month.		 * For example, if the first day of the month is on a Sunday, now.dayOfWeek		 * will equal one. firstDayOfWeek will equal one for Sunday, two for Monday.		 */		weekday = now.dayOfWeek - (**settings).firstDayOfWeek;		if (weekday < 0)			weekday = 6;		hPos = monthRect.left + 2 + (weekday * dateWidth);		for (today = 1; today <= daysInMonth; today++, weekday++) {			if (weekday >= 7) {		/* Wrap around to a new week.					*/				hPos = monthRect.left + 2;				vPos += lineHeight;				weekday = 0;			}			/*			 * hOffset locates the left edge of the date -- this will cover one			 * digit for dates 1 to 9, and two digits for 10 to 31.			 */			hOffset = hPos + spaceWidth;			if (today < 10) {				hOffset += digitWidth;		/* Space over the leftmost digit space	*/				MoveTo(hOffset, vPos);				DrawChar(today + '0');			}			else {				MoveTo(hOffset, vPos);				DrawChar((today / 10) + '0');				DrawChar((today % 10) + '0');			}			if (isThisMonth && today == thisDate) {			/* 1.0d3				*/				/*				 * We are drawing an oval around this date. There is quite a bit of				 * eyeball adjustment that could be re-adjusted by someone with				 * more (or less) visual taste.				 */				GetPenState(&penState);				penSize = (fontInfo.ascent >= 12) ? fontInfo.ascent / 6 : 1;				vOffset = vPos - fontInfo.ascent - penSize;				PenSize(penSize, penSize);				SetRect(					&dayRect,					hOffset - penSize,					vOffset,					hPos + dateWidth + penSize,					vOffset + lineHeight + penSize				);				FrameRoundRect(					&dayRect,					(dateWidth * 3) / 4,					(lineHeight * 3) / 4				);				SetPenState(&penState);			}			hPos += dateWidth;		}}