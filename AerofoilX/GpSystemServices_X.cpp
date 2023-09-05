#include "GpSystemServices_X.h"

#include "IGpClipboardContents.h"
#include "IGpThreadEvent.h"

#include <SDL.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdlib.h>

struct GpSystemServices_X_ThreadStartParams
{
	GpSystemServices_X::ThreadFunc_t m_threadFunc;
	void *m_threadContext;
	IGpThreadEvent *m_threadStartEvent;
};

static void *StaticStartThread(void *lpThreadParameter)
{
	const GpSystemServices_X_ThreadStartParams *threadParams = static_cast<const GpSystemServices_X_ThreadStartParams*>(lpThreadParameter);

	GpSystemServices_X::ThreadFunc_t threadFunc = threadParams->m_threadFunc;
	void *threadContext = threadParams->m_threadContext;
	IGpThreadEvent *threadStartEvent = threadParams->m_threadStartEvent;

	threadStartEvent->Signal();

	return reinterpret_cast<void*>(static_cast<intptr_t>(threadFunc(threadContext)));
}

GpSystemServices_X::GpSystemServices_X()
	: m_textInputEnabled(false)
	, m_clipboardContents(nullptr)
{
}

GpSystemServices_X::~GpSystemServices_X()
{
	if (m_clipboardContents)
		m_clipboardContents->Destroy();
}

void *GpSystemServices_X::CreateThread(ThreadFunc_t threadFunc, void *context)
{
	IGpThreadEvent *evt = CreateThreadEvent(true, false);
	if (!evt)
		return nullptr;

	pthread_t *threadPtr = static_cast<pthread_t*>(malloc(sizeof(pthread_t)));
	if (!threadPtr)
	{
		evt->Destroy();
		return nullptr;
	}

	GpSystemServices_X_ThreadStartParams startParams;
	startParams.m_threadContext = context;
	startParams.m_threadFunc = threadFunc;
	startParams.m_threadStartEvent = evt;

	if (pthread_create(threadPtr, nullptr, StaticStartThread, &startParams) != 0)
	{
		evt->Destroy();
		return nullptr;
	}

	evt->Wait();
	evt->Destroy();

	return static_cast<void*>(threadPtr);
}

bool GpSystemServices_X::Beep() const
{
	return false;
}

bool GpSystemServices_X::IsTouchscreen() const
{
	return false;
}

bool GpSystemServices_X::IsUsingMouseAsTouch() const
{
	return false;
}

bool GpSystemServices_X::IsTextInputObstructive() const
{
	return false;
}

bool GpSystemServices_X::IsFullscreenPreferred() const
{	
	return true;
}

bool GpSystemServices_X::IsFullscreenOnStartup() const
{
	return false;
}

bool GpSystemServices_X::HasNativeFileManager() const
{
	return true;
}

GpOperatingSystem_t GpSystemServices_X::GetOperatingSystem() const
{
#ifdef __MACOS__
	return GpOperatingSystems::kMacOS;
#else
	return GpOperatingSystems::kLinux;
#endif
}

GpOperatingSystemFlavor_t GpSystemServices_X::GetOperatingSystemFlavor() const
{
	return GpOperatingSystemFlavors::kGeneric;
}

unsigned int GpSystemServices_X::GetCPUCount() const
{
	return SDL_GetCPUCount();
}

void GpSystemServices_X::SetTextInputEnabled(bool isEnabled)
{
	m_textInputEnabled = isEnabled;
}

bool GpSystemServices_X::IsTextInputEnabled() const
{
	return m_textInputEnabled;
}

bool GpSystemServices_X::AreFontResourcesSeekable() const
{
	return true;
}

IGpClipboardContents *GpSystemServices_X::GetClipboardContents() const
{
	return m_clipboardContents;
}

void GpSystemServices_X::SetClipboardContents(IGpClipboardContents *contents)
{
	if (contents != m_clipboardContents)
	{
		if (m_clipboardContents)
			m_clipboardContents->Destroy();

		m_clipboardContents = contents;
	}
}

GpSystemServices_X *GpSystemServices_X::GetInstance()
{
	return &ms_instance;
}

GpSystemServices_X GpSystemServices_X::ms_instance;
