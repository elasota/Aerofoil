//============================================================================
//----------------------------------------------------------------------------
//								 RoomGraphics.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLResources.h"
#include "PLStandardColors.h"
#include "Externs.h"
#include "Environ.h"
#include "MainWindow.h"
#include "RectUtils.h"
#include "ResolveCachingColor.h"
#include "Room.h"
#include "BitmapImage.h"


#define kManholeThruFloor		3957


void LoadGraphicSpecial (DrawSurface *surface, short);
void DrawRoomBackground (short, short, short);
void DrawFloorSupport (void);
void ReadyBackMap (void);
void RestoreWorkMap (void);
void DrawLighting (void);


Rect		suppSrcRect;
DrawSurface	*suppSrcMap;
Rect		localRoomsDest[9];
Rect		houseRect;
short		numNeighbors, numLights, thisTiles[kNumTiles];
short		localNumbers[9], thisBackground;
Boolean		isStructure[9], wardBitSet;

extern	Rect		tempManholes[];
extern	short		numTempManholes, tvWithMovieNumber;
extern	Boolean		shadowVisible, takingTheStairs;


//==============================================================  Functions
//--------------------------------------------------------------  DrawLocale

void ResetLocale (Boolean soft)
{
	char		wasState;

	if (soft)
	{
		RemoveSavedMapsNotInRoom(localNumbers[kCentralRoom]);
		ZeroDinahsNotInRoom(localNumbers[kCentralRoom]);

		// Clear all dinah indexes, they'll be remapped
		for (int i = 0; i < numMasterObjects; i++)
			masterObjects[i].dynaNum = -1;
	}
	else
	{
		ZeroFlamesAndTheLike();
		ZeroDinahs();
		KillAllBands();
		ZeroTriggers();
		numTempManholes = 0;
		FlushAnyTriggerPlaying();
		DumpTriggerSound();
		tvInRoom = false;
		tvWithMovieNumber = -1;

		for (int i = 0; i < 9; i++)
		{
			localNumbers[i] = GetNeighborRoomNumber(i);
			isStructure[i] = IsRoomAStructure(localNumbers[i]);
		}
		ListAllLocalObjects();
	}

	ZeroMirrorRegion();

	takingTheStairs = false;

	const short roomV = (*thisHouse)->rooms[thisRoomNumber].floor;

	PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
	backSrcMap->FillRect(backSrcRect, blackColor);
	
	if (numNeighbors > 3)
	{
		numLights = GetNumberOfLights(localNumbers[kNorthWestRoom]);
		DrawRoomBackground(localNumbers[kNorthWestRoom], kNorthWestRoom, roomV + 1);
		DrawARoomsObjects(kNorthWestRoom, false);
		
		numLights = GetNumberOfLights(localNumbers[kNorthEastRoom]);
		DrawRoomBackground(localNumbers[kNorthEastRoom], kNorthEastRoom, roomV + 1);
		DrawARoomsObjects(kNorthEastRoom, false);
		
		numLights = GetNumberOfLights(localNumbers[kNorthRoom]);
		DrawRoomBackground(localNumbers[kNorthRoom], kNorthRoom, roomV + 1);
		DrawARoomsObjects(kNorthRoom, false);
		
		numLights = GetNumberOfLights(localNumbers[kSouthWestRoom]);
		DrawRoomBackground(localNumbers[kSouthWestRoom], kSouthWestRoom, roomV - 1);
		DrawARoomsObjects(kSouthWestRoom, false);
		
		numLights = GetNumberOfLights(localNumbers[kSouthEastRoom]);
		DrawRoomBackground(localNumbers[kSouthEastRoom], kSouthEastRoom, roomV - 1);
		DrawARoomsObjects(kSouthEastRoom, false);
		
		numLights = GetNumberOfLights(localNumbers[kSouthRoom]);
		DrawRoomBackground(localNumbers[kSouthRoom], kSouthRoom, roomV - 1);
		DrawARoomsObjects(kSouthRoom, false);
	}
	
	if (numNeighbors > 1)
	{
		numLights = GetNumberOfLights(localNumbers[kWestRoom]);
		DrawRoomBackground(localNumbers[kWestRoom], kWestRoom, roomV);
		DrawARoomsObjects(kWestRoom, false);
		DrawLighting();
		
		numLights = GetNumberOfLights(localNumbers[kEastRoom]);
		DrawRoomBackground(localNumbers[kEastRoom], kEastRoom, roomV);
		DrawARoomsObjects(kEastRoom, false);
		DrawLighting();
	}
	
	numLights = GetNumberOfLights(localNumbers[kCentralRoom]);
	DrawRoomBackground(localNumbers[kCentralRoom], kCentralRoom, roomV);
	DrawARoomsObjects(kCentralRoom, soft);
	DrawLighting();

	if (numNeighbors > 3)
		DrawFloorSupport();
	RestoreWorkMap();
	shadowVisible = IsShadowVisible();

	if (soft)
		RedrawAllGrease();
}

//--------------------------------------------------------------  LoadGraphicSpecial

void LoadGraphicSpecial (DrawSurface *surface, short resID)
{
	Rect		bounds;
	THandle<BitmapImage>	thePicture;
	
	thePicture = LoadHouseResource('PICT', resID).StaticCast<BitmapImage>();
	if (thePicture == nil)
	{
		thePicture = LoadHouseResource('Date', resID).StaticCast<BitmapImage>();
		if (thePicture == nil)
		{
			thePicture = LoadHouseResource('PICT', 2000).StaticCast<BitmapImage>();
			if (thePicture == nil)
				RedAlert(kErrFailedGraphicLoad);
		}
	}
	
	bounds = (*thePicture)->GetRect();
	OffsetRect(&bounds, -bounds.left, -bounds.top);
	surface->DrawPicture(thePicture, bounds);

	thePicture.Dispose();
}

//--------------------------------------------------------------  DrawRoomBackground

void DrawRoomBackground (short who, short where, short elevation)
{
	Rect		src, dest;
	short		i, pictID;
	short		tiles[kNumTiles];
	char		wasState;
	
	if (where == kCentralRoom)
	{
		thisBackground = (*thisHouse)->rooms[who].background;
		for (i = 0; i < kNumTiles; i++)
			thisTiles[i] = (*thisHouse)->rooms[who].tiles[i];
	}
	
	if ((numLights == 0) && (who != kRoomIsEmpty))
	{
		PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
		backSrcMap->FillRect(localRoomsDest[where], blackColor);
		
		return;
	}
	
	if (who == kRoomIsEmpty)		// This call should be smarter than this
	{
		if (wardBitSet)
		{
			PortabilityLayer::ResolveCachingColor blackColor = StdColors::Black();
			backSrcMap->FillRect(localRoomsDest[where], blackColor);
			return;
		}
		
		if (elevation > 1)
		{
			pictID = kSky;
			for (i = 0; i < kNumTiles; i++)
				tiles[i] = 2;
		}
		else if (elevation == 1)
		{
			pictID = kMeadow;
			for (i = 0; i < kNumTiles; i++)
				tiles[i] = 0;
		}
		else
		{
			pictID = kDirt;
			for (i = 0; i < kNumTiles; i++)
				tiles[i] = 0;
		}
	}
	else
	{
		pictID = (*thisHouse)->rooms[who].background;
		for (i = 0; i < kNumTiles; i++)
			tiles[i] = (*thisHouse)->rooms[who].tiles[i];
	}
	
	LoadGraphicSpecial(workSrcMap, pictID);
	
	QSetRect(&src, 0, 0, kTileWide, kTileHigh);
	QSetRect(&dest, 0, 0, kTileWide, kTileHigh);
	QOffsetRect(&dest, localRoomsDest[where].left, localRoomsDest[where].top);
	for (i = 0; i < kNumTiles; i++)
	{
		src.left = tiles[i] * kTileWide;
		src.right = src.left + kTileWide;
		CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&src, &dest, srcCopy);
		QOffsetRect(&dest, kTileWide, 0);
	}
}

//--------------------------------------------------------------  DrawFloorSupport

void DrawFloorSupport (void)
{
	Rect		src, dest, whoCares;
	short		i;

	DrawSurface *surface = backSrcMap;

	src = suppSrcRect;
	
	if (isStructure[kNorthWestRoom])
	{
		dest = suppSrcRect;			// left room's ceiling
		QOffsetRect(&dest, localRoomsDest[kWestRoom].left, 
				localRoomsDest[kCentralRoom].top - suppSrcRect.bottom);
		CopyBits((BitMap *)*GetGWorldPixMap(suppSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&src, &dest, srcCopy);
		
		for (i = 0; i < numTempManholes; i++)
			if (SectRect(&dest, &tempManholes[i], &whoCares))
			{
				tempManholes[i].top = dest.top;
				tempManholes[i].bottom = dest.bottom;
				LoadScaledGraphic(surface, kManholeThruFloor, &tempManholes[i]);
			}
	}
	
	if (isStructure[kWestRoom])
	{
		dest = suppSrcRect;			// left room's floor
		QOffsetRect(&dest, localRoomsDest[kWestRoom].left, 
				localRoomsDest[kCentralRoom].bottom);
		CopyBits((BitMap *)*GetGWorldPixMap(suppSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&src, &dest, srcCopy);
		
		for (i = 0; i < numTempManholes; i++)
			if (SectRect(&dest, &tempManholes[i], &whoCares))
			{
				tempManholes[i].top = dest.top;
				tempManholes[i].bottom = dest.bottom;
				LoadScaledGraphic(surface, kManholeThruFloor, &tempManholes[i]);
			}
	}
	
	if (isStructure[kNorthRoom])
	{
		dest = suppSrcRect;			// directly above main room
		QOffsetRect(&dest, localRoomsDest[kCentralRoom].left, 
				localRoomsDest[kCentralRoom].top - suppSrcRect.bottom);
		CopyBits((BitMap *)*GetGWorldPixMap(suppSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&src, &dest, srcCopy);
		for (i = 0; i < numTempManholes; i++)
			if (SectRect(&dest, &tempManholes[i], &whoCares))
			{
				tempManholes[i].top = dest.top;
				tempManholes[i].bottom = dest.bottom;
				LoadScaledGraphic(surface, kManholeThruFloor, &tempManholes[i]);
			}
	}
	
	if (isStructure[kCentralRoom])
	{
		dest = suppSrcRect;			// directly below main room
		QOffsetRect(&dest, localRoomsDest[kCentralRoom].left, 
				localRoomsDest[kCentralRoom].bottom);
		CopyBits((BitMap *)*GetGWorldPixMap(suppSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&src, &dest, srcCopy);
		
		for (i = 0; i < numTempManholes; i++)
			if (SectRect(&dest, &tempManholes[i], &whoCares))
			{
				tempManholes[i].top = dest.top;
				tempManholes[i].bottom = dest.bottom;
				LoadScaledGraphic(surface, kManholeThruFloor, &tempManholes[i]);
			}
	}
	
	if (isStructure[kNorthEastRoom])
	{
		dest = suppSrcRect;
		QOffsetRect(&dest, localRoomsDest[kEastRoom].left, 
				localRoomsDest[kCentralRoom].top - suppSrcRect.bottom);
		CopyBits((BitMap *)*GetGWorldPixMap(suppSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&src, &dest, srcCopy);
		
		for (i = 0; i < numTempManholes; i++)
			if (SectRect(&dest, &tempManholes[i], &whoCares))
			{
				tempManholes[i].top = dest.top;
				tempManholes[i].bottom = dest.bottom;
				LoadScaledGraphic(surface, kManholeThruFloor, &tempManholes[i]);
			}
	}
	
	if (isStructure[kEastRoom])
	{
		dest = suppSrcRect;
		QOffsetRect(&dest, localRoomsDest[kEastRoom].left, 
				localRoomsDest[kCentralRoom].bottom);
		CopyBits((BitMap *)*GetGWorldPixMap(suppSrcMap), 
				(BitMap *)*GetGWorldPixMap(backSrcMap), 
				&src, &dest, srcCopy);
		
		for (i = 0; i < numTempManholes; i++)
			if (SectRect(&dest, &tempManholes[i], &whoCares))
			{
				tempManholes[i].top = dest.top;
				tempManholes[i].bottom = dest.bottom;
				LoadScaledGraphic(surface, kManholeThruFloor, &tempManholes[i]);
			}
	}
}

//--------------------------------------------------------------  ReadyBackMap

void ReadyBackMap (void)
{
	CopyBits((BitMap *)*GetGWorldPixMap(workSrcMap), 
			(BitMap *)*GetGWorldPixMap(backSrcMap), 
			&workSrcRect, &workSrcRect, srcCopy);
}

//--------------------------------------------------------------  RestoreWorkMap

void RestoreWorkMap (void)
{
	Rect		dest;
	
	dest = backSrcRect;
	
	CopyBits((BitMap *)*GetGWorldPixMap(backSrcMap), 
			(BitMap *)*GetGWorldPixMap(workSrcMap), 
			&backSrcRect, &backSrcRect, srcCopy);
}

//--------------------------------------------------------------  ReadyLevel

void ReadyLevel (void)
{
	NilSavedMaps();
	
#ifdef COMPILEQT
	if ((thisMac.hasQT) && (hasMovie) && (tvInRoom))
	{
		tvInRoom = false;
		tvWithMovieNumber = -1;
		theMovie.m_playing = false;
	}
#endif
	
	DetermineRoomOpenings();
	ResetLocale(false);
	InitGarbageRects();
}

//--------------------------------------------------------------  DrawLighting

void DrawLighting (void)
{
	if (numLights == 0)
		return;
	else
	{
		// for future construction
	}
}

//--------------------------------------------------------------  RedrawRoomLighting

void RedrawRoomLighting (void)
{
	short		roomV;
	char		wasState;
	Boolean		wasLit, isLit;
	
	roomV = (*thisHouse)->rooms[thisRoomNumber].floor;
	
	wasLit = numLights > 0;
	numLights = GetNumberOfLights(localNumbers[kCentralRoom]);
	isLit = numLights > 0;
	if (wasLit != isLit)
	{
		DrawRoomBackground(localNumbers[kCentralRoom], kCentralRoom, roomV);
		DrawARoomsObjects(kCentralRoom, true);
		RedrawAllGrease();
		DrawLighting();
		UpdateOutletsLighting(localNumbers[kCentralRoom], numLights);

		if (numNeighbors > 3)
			DrawFloorSupport();
		RestoreWorkMap();
		AddRectToWorkRects(&localRoomsDest[kCentralRoom]);
		shadowVisible = IsShadowVisible();
	}
}

