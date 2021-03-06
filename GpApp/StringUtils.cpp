
//============================================================================
//----------------------------------------------------------------------------
//								  StringUtils.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLPasStr.h"
#include "Externs.h"
#include "RenderedFont.h"

#include <string.h>


//==============================================================  Functions
//--------------------------------------------------------------  PasStringCopy
// Given a source string and storage for a second, this function?
// copies from one to the other.  It assumes Pascal style strings.

void PasStringCopy (StringPtr p1, StringPtr p2)
{
	short		stringLength;

	stringLength = *p2++ = *p1++;
	while (--stringLength >= 0)
		*p2++ = *p1++;
}

//--------------------------------------------------------------  WhichStringFirst

// This is a sorting function that handles two Pascal strings.  It?
// will return a 1 to indicate the 1st string is "greater", a 1 to?
// indicate the 2nd was greater and a 0 to indicate that the strings?
// are equal.

short WhichStringFirst (StringPtr p1, StringPtr p2)
{
	short		smallestLength, seek, greater;
	char		char1, char2;
	Boolean		foundIt;

	smallestLength = p1[0];
	if (p2[0] < smallestLength)
		smallestLength = p2[0];

	greater = 0;					// neither are greater, they are equal
	seek = 1;						// start at character #1
	foundIt = false;
	do
	{
		char1 = p1[seek];			// make upper case (if applicable)
		if ((char1 > 0x60) && (char1 < 0x7B))
			char1 -= 0x20;
		char2 = p2[seek];			// make upper case (if applicable)
		if ((char2 > 0x60) && (char2 < 0x7B))
			char2 -= 0x20;

		if (char1 > char2)			// first string is greater
		{
			greater = 1;
			foundIt = true;
		}
		else if (char1 < char2)		// second string is greater
		{
			greater = 2;
			foundIt = true;
		}
		seek++;
		if (seek > smallestLength)	// we've reached the end of the line
		{
			if (!foundIt)
			{
				if (p1[0] > p2[0])	// shortest string wins
					greater = 1;
				else if (p1[0] < p2[0])
					greater = 2;
			}
			foundIt = true;
		}
	}
	while (!foundIt);

	return (greater);
}

//--------------------------------------------------------------  PasStringCopyNum

// This function copies a specified number of characters from one?
// Pascal string to another.

void PasStringCopyNum (StringPtr p1, StringPtr p2, short charsToCopy)
{
	short		i;

	if (charsToCopy > *p1)		// if trying to copy more chars than there are
		charsToCopy = *p1;		// reduce the number of chars to copy to this size

	*p2 = static_cast<unsigned char>(charsToCopy);

	p2++;
	p1++;

	for (i = 0; i < charsToCopy; i++)
		*p2++ = *p1++;
}

//--------------------------------------------------------------  PasStringConcat
// This function concatenates the second Pascal string to the end of?
// the first Pascal string.

void PasStringConcat (StringPtr p1, const PLPasStr &p2)
{
	short		wasLength, addedLength;

	wasLength = *p1;
	if (wasLength > 255)
		wasLength = 255;

	addedLength = p2.Length();
	if ((wasLength + addedLength) > 255)
		addedLength = 255 - wasLength;

	*p1 = wasLength + addedLength;

	p1 += (1 + wasLength);

	memcpy(p1, p2.Chars(), addedLength);
}

//--------------------------------------------------------------  GetLineOfText

// This function walks through a source string and looks for an?
// entire line of text.  A "line" of text is assumed to be bounded?
// by carriage returns.  The index variable indicates which line?
// is sought.

void GetLineOfText (StringPtr srcStr, short index, StringPtr textLine)
{
	short		i, srcLength, count, start, stop;
	Boolean		foundIt;

	PasStringCopy(PSTR(""), textLine);
	srcLength = srcStr[0];

	if (index == 0)						// walk through to "index"
		start = 1;
	else
	{
		start = 0;
		count = 0;
		i = 0;
		foundIt = false;
		do
		{
			i++;
			if (srcStr[i] == '\r')
			{
				count++;
				if (count == index)
				{
					start = i + 1;
					foundIt = true;
				}
			}
		}
		while ((i < srcLength) && (!foundIt));
	}

	if (start != 0)
	{
		i = start;

		foundIt = false;
		do
		{
			if (srcStr[i] == '\r')
			{
				stop = i;
				foundIt = true;
			}
			i++;
		}
		while ((i < srcLength) && (!foundIt));

		if (!foundIt)
		{
			if (start > srcLength)
			{
				start = srcLength;
				stop = srcLength - 1;
			}
			else
				stop = i;
		}

		count = 0;

		for (i = start; i <= stop; i++)
		{
			count++;
			textLine[count] = srcStr[i];
		}
		textLine[0] = static_cast<unsigned char>(count);
	}
}

//--------------------------------------------------------------  WrapText

// Given a string and the maximum number of characters to put on?
// one line, this function goes through and inserts carriage returns?
// in order to ensure that no line of text exceeds maxChars.

void WrapText (StringPtr theText, short maxChars)
{
	short		lastChar, count, chars, spaceIs;
	Boolean		foundEdge, foundSpace;

	lastChar = theText[0];
	count = 0;

	do
	{
		chars = 0;
		foundEdge = false;
		foundSpace = false;
		do
		{
			count++;
			chars++;
			if (theText[count] == '\r')
				foundEdge = true;
			else if (theText[count] == ' ')
			{
				foundSpace = true;
				spaceIs = count;
			}
		}
		while ((count < lastChar) && (chars < maxChars) && (!foundEdge));

		if ((!foundEdge) && (count < lastChar) && (foundSpace))
		{
			theText[spaceIs] = '\r';
			count = spaceIs + 1;
		}
	}
	while (count < lastChar);
}

//--------------------------------------------------------------  GetFirstWordOfString

// Walks a string looking for a space (denoting first word of string).

void GetFirstWordOfString (StringPtr stringIn, StringPtr stringOut)
{
	short		isLong, spaceAt, i;

	isLong = stringIn[0];
	spaceAt = isLong;

	for (i = 1; i < isLong; i++)
	{
		if ((stringIn[i] == ' ') && (spaceAt == isLong))
			spaceAt = i - 1;
	}

	if (spaceAt <= 0)
		PasStringCopy(PSTR(""), stringOut);
	else
		PasStringCopyNum(stringIn, stringOut, spaceAt);
}

//--------------------------------------------------------------  CollapseStringToWidth

// Given a string and a maximum width (in pixels), this function?
// calculates how wide the text would be drawn with the current?
// font.  If the text would exceed our width limit, characters?
// are dropped off the end of the string and "?" appended.

void CollapseStringToWidth (PortabilityLayer::RenderedFont *font, StringPtr theStr, short wide)
{
	short		dotsWide;
	Boolean 	tooWide;

	dotsWide = font->MeasurePStr(PSTR("\xc9"));
	tooWide = font->MeasurePStr(theStr) > wide;
	while (tooWide && theStr[0] > 0)
	{
		theStr[0]--;
		tooWide = ((font->MeasurePStr(theStr) + dotsWide) > wide);
		if (!tooWide)
			PasStringConcat(theStr, PSTR("\xc9"));
	}
}

//--------------------------------------------------------------  GetLocalizedString

StringPtr GetLocalizedString (short index, StringPtr theString)
{
	#define		kLocalizedStringsID		150

	GetIndString(theString, kLocalizedStringsID, index);
	return (theString);
}


