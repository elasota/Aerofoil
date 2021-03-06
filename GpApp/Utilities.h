//============================================================================
//----------------------------------------------------------------------------
//								  Utilities.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLQDOffscreen.h"
#include "GpPixelFormat.h"


PLError_t CreateOffScreenGWorld (DrawSurface **surface, Rect *bounds);
PLError_t CreateOffScreenGWorldCustomDepth (DrawSurface **surface, Rect *bounds, GpPixelFormat_t pixelFormat);
