#define _LARGEFILE64_SOURCE
#include "GpFileSystem_X.h"
#include "GpIOStream.h"
#include "IGpDirectoryCursor.h"
#include "IGpSystemServices.h"
#include "IGpMutex.h"
#include "IGpThreadRelay.h"
#include "VirtualDirectory.h"

#include "PLDrivers.h"

#ifdef __MACOS__
#include <SDL.h>
#include <SDL_rwops.h>
#else
#include "SDL2/SDL.h"
#include "SDL2/SDL_rwops.h"
#endif

#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "UTF8.h"

#if defined(__CYGWIN__) || defined(__MACOS__) || defined(__linux)
typedef off_t off64_t;
#define fstat64 fstat
#define fseek64 fseek
#define ftruncate64 ftruncate
#define stat64 stat
#endif

class GpFileStream_X_File final : public GpIOStream
{
public:
	GpFileStream_X_File(FILE *f, bool readOnly, bool writeOnly);
	~GpFileStream_X_File();

	size_t Read(void *bytesOut, size_t size) override;
	size_t Write(const void *bytes, size_t size) override;
	bool IsSeekable() const override;
	bool IsReadOnly() const override;
	bool IsWriteOnly() const override;
	bool SeekStart(GpUFilePos_t loc) override;
	bool SeekCurrent(GpFilePos_t loc) override;
	bool SeekEnd(GpUFilePos_t loc) override;
	GpUFilePos_t Size() const override;
	GpUFilePos_t Tell() const override;
	void Close() override;
	void Flush() override;

private:
	FILE *m_f;
	bool m_seekable;
	bool m_isReadOnly;
	bool m_isWriteOnly;
};


GpFileStream_X_File::GpFileStream_X_File(FILE *f, bool readOnly, bool writeOnly)
	: m_f(f)
	, m_isReadOnly(readOnly)
	, m_isWriteOnly(writeOnly)
{
	m_seekable = (fseek(m_f, 0, SEEK_CUR) == 0);
}

GpFileStream_X_File::~GpFileStream_X_File()
{
	fclose(m_f);
}

size_t GpFileStream_X_File::Read(void *bytesOut, size_t size)
{
	if (m_isWriteOnly)
		return 0;
	return fread(bytesOut, 1, size, m_f);
}

size_t GpFileStream_X_File::Write(const void *bytes, size_t size)
{
	if (m_isReadOnly)
		return 0;
	return fwrite(bytes, 1, size, m_f);
}

bool GpFileStream_X_File::IsSeekable() const
{
	return m_seekable;
}

bool GpFileStream_X_File::IsReadOnly() const
{
	return m_isReadOnly;
}

bool GpFileStream_X_File::IsWriteOnly() const
{
	return m_isWriteOnly;
}

bool GpFileStream_X_File::SeekStart(GpUFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return fseek64(m_f, static_cast<off64_t>(loc), SEEK_SET) >= 0;
}

bool GpFileStream_X_File::SeekCurrent(GpFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return fseek64(m_f, static_cast<off64_t>(loc), SEEK_CUR) >= 0;
}

bool GpFileStream_X_File::SeekEnd(GpUFilePos_t loc)
{
	if (!m_seekable)
		return false;

	fflush(m_f);
	return fseek64(m_f, -static_cast<off64_t>(loc), SEEK_END) >= 0;
}

GpUFilePos_t GpFileStream_X_File::Size() const
{
	fflush(m_f);

	struct stat64 s;
	if (fstat64(fileno(m_f), &s) < 0)
		return 0;

	return static_cast<GpUFilePos_t>(s.st_size);
}

GpUFilePos_t GpFileStream_X_File::Tell() const
{
	return static_cast<GpUFilePos_t>(ftell(m_f));
}

void GpFileStream_X_File::Close()
{
	this->~GpFileStream_X_File();
	free(this);
}

void GpFileStream_X_File::Flush()
{
	fflush(m_f);
}

bool GpFileSystem_X::ResolvePath(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths, std::string &resolution)
{
	const char *prefsAppend = nullptr;

	switch (virtualDirectory)
	{
	case PortabilityLayer::VirtualDirectories::kApplicationData:
		resolution = std::string("Packaged");
		break;
	case PortabilityLayer::VirtualDirectories::kGameData:
		resolution = std::string("Packaged/Houses");
		break;
	case PortabilityLayer::VirtualDirectories::kFonts:
		resolution = std::string("Resources");
		break;
	case PortabilityLayer::VirtualDirectories::kHighScores:
		prefsAppend = "HighScores";
		break;
	case PortabilityLayer::VirtualDirectories::kUserData:
		prefsAppend = "Houses";
		break;
	case PortabilityLayer::VirtualDirectories::kUserSaves:
		prefsAppend = "SavedGames";
		break;
	case PortabilityLayer::VirtualDirectories::kPrefs:
		prefsAppend = "Prefs";
		break;
	case PortabilityLayer::VirtualDirectories::kLogs:
		prefsAppend = "Logs";
		break;
	default:
		return false;
	};

	if (prefsAppend)
		resolution = m_prefsPath + prefsAppend;
	else
		resolution = m_basePath + resolution;

	for (size_t i = 0; i < numPaths; i++)
	{
		resolution += "/";
		resolution += paths[i];
	}
	
	return true;
}

GpFileSystem_X::GpFileSystem_X()
	: m_delayCallback(nullptr)
{
}

GpFileSystem_X::~GpFileSystem_X()
{
}

void GpFileSystem_X::Init()
{
	char *prefsDir = SDL_GetPrefPath("aerofoil", "aerofoil");
	m_prefsPath = prefsDir;

	char *baseDir = SDL_GetBasePath();
	m_basePath = baseDir;
	SDL_free(baseDir);

	char baseDirSeparator = m_basePath[m_basePath.size() - 1];
	if (m_basePath.size() >= 4 && m_basePath.substr(m_basePath.size() - 4, 3) == "bin")
		m_basePath = m_basePath.substr(0, m_basePath.size() - 4) + "lib" + baseDirSeparator + "aerofoil" + baseDirSeparator;

	const char *extensions[] = { "HighScores", "Houses", "SavedGames", "Prefs", "Logs" };
	for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++)
	{
		std::string prefsPath = std::string(prefsDir) + extensions[i];
		int created = mkdir(prefsPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	SDL_free(prefsDir);
}

bool GpFileSystem_X::FileExists(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path)
{
	std::string resolvedPath;
	if (!ResolvePath(virtualDirectory, &path, 1, resolvedPath))
		return false;

	struct stat s;
	return stat(resolvedPath.c_str(), &s) == 0;
}

bool GpFileSystem_X::FileLocked(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &exists)
{
	std::string resolvedPath;
	if (!ResolvePath(virtualDirectory, &path, 1, resolvedPath))
	{
		if (exists)
			exists = false;
		return false;
	}

	int permissions = access(resolvedPath.c_str(), W_OK | F_OK);
	exists = ((permissions & F_OK) != 0);
	return ((permissions & W_OK) != 0);
}

GpIOStream *GpFileSystem_X::OpenFileNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* subPaths, size_t numSubPaths, bool writeAccess, GpFileCreationDisposition_t createDisposition)
{
	const char *mode = nullptr;
	bool canWrite = false;
	bool needResetPosition = false;

	switch (createDisposition)
	{
		case GpFileCreationDispositions::kCreateOrOverwrite:
			mode = "wb";
			break;
		case GpFileCreationDispositions::kCreateNew:
			mode = "x+b";
			break;
		case GpFileCreationDispositions::kCreateOrOpen:
			mode = "a+b";
			needResetPosition = true;
			break;
		case GpFileCreationDispositions::kOpenExisting:
			mode = writeAccess ? "r+b" : "rb";
			break;
		case GpFileCreationDispositions::kOverwriteExisting:
			mode = "r+b";
			break;
		default:
			return nullptr;
	};

	if (virtualDirectory == PortabilityLayer::VirtualDirectories::kSourceExport)
		return nullptr;

	std::string resolvedPath;
	
	if (!ResolvePath(virtualDirectory, subPaths, numSubPaths, resolvedPath))
		return nullptr;

	void *objStorage = malloc(sizeof(GpFileStream_X_File));
	if (!objStorage)
		return nullptr;

	FILE *f = fopen(resolvedPath.c_str(), mode);
	if (!f)
	{
		fprintf(stderr, "GpFileSystem_X: Cannot open \"%s\"\n",
				resolvedPath.c_str());
		free(objStorage);
		return nullptr;
	}

	if (needResetPosition)
		fseek(f, 0, SEEK_SET);

	if (createDisposition == GpFileCreationDispositions::kOverwriteExisting)
	{
		if (ftruncate64(fileno(f), 0) < 0)
		{
			free(objStorage);
			fclose(f);
			return nullptr;
		}
	}

	return new (objStorage) GpFileStream_X_File(f, !writeAccess, false);
}

bool GpFileSystem_X::DeleteFile(PortabilityLayer::VirtualDirectory_t virtualDirectory, const char *path, bool &existed)
{
	std::string resolvedPath;
	if (!ResolvePath(virtualDirectory, &path, 1, resolvedPath))
	{
		existed = false;
		return false;
	}

	if (unlink(resolvedPath.c_str()) < 0)
	{
		existed = (errno != ENOENT);
		return false;
	}
	existed = true;
	return true;
}

bool GpFileSystem_X::ValidateFilePath(const char *path, size_t length) const
{
	for (size_t i = 0; i < length; i++)
	{
		const char c = path[i];
		if (c >= '0' && c <= '9')
			continue;

		if (c == '_' || c == '.' || c == '\'' || c == '!')
			continue;

		if (c == ' ' && i != 0 && i != length - 1)
			continue;

		if (c >= 'a' && c <= 'z')
			continue;

		if (c >= 'A' && c <= 'Z')
			continue;

		return false;
	}

	return true;
}

bool GpFileSystem_X::ValidateFilePathUnicodeChar(uint32_t c) const
{
	if (c >= '0' && c <= '9')
		return true;

	if (c == '_' || c == '\'')
		return true;

	if (c == ' ')
		return true;

	if (c >= 'a' && c <= 'z')
		return true;

	if (c >= 'A' && c <= 'Z')
		return true;

	return false;
}

void GpFileSystem_X::SetDelayCallback(DelayCallback_t delayCallback)
{
	m_delayCallback = delayCallback;
}

GpFileSystem_X *GpFileSystem_X::GetInstance()
{
	return &ms_instance;
}

class GpDirectoryCursor_StringList final : public IGpDirectoryCursor
{
public:
	explicit GpDirectoryCursor_StringList(std::vector<std::string> &paths);
	~GpDirectoryCursor_StringList();

	bool GetNext(const char *&outFileName) override;
	void Destroy() override;

private:
	std::vector<std::string> m_paths;
	size_t m_index;
};

GpDirectoryCursor_StringList::GpDirectoryCursor_StringList(std::vector<std::string> &paths)
	: m_index(0)
{
	std::swap(paths, m_paths);
}

GpDirectoryCursor_StringList::~GpDirectoryCursor_StringList()
{
}

bool GpDirectoryCursor_StringList::GetNext(const char *&outFileName)
{
	if (m_index == m_paths.size())
		return false;
	outFileName = m_paths[m_index].c_str();
	m_index++;
	return true;
}

void GpDirectoryCursor_StringList::Destroy()
{
	delete this;
}

class GpDirectoryCursor_POSIX final : public IGpDirectoryCursor
{
public:
	explicit GpDirectoryCursor_POSIX(DIR *dir);
	~GpDirectoryCursor_POSIX();

	bool GetNext(const char *&outFileName) override;
	void Destroy() override;

private:
	DIR *m_dir;
};

GpDirectoryCursor_POSIX::GpDirectoryCursor_POSIX(DIR *dir)
	: m_dir(dir)
{
}

GpDirectoryCursor_POSIX::~GpDirectoryCursor_POSIX()
{
	closedir(m_dir);
}

bool GpDirectoryCursor_POSIX::GetNext(const char *&outFileName)
{
	struct dirent *dir = readdir(m_dir);
	if (!dir)
		return false;

	outFileName = dir->d_name;
	return true;
}

void GpDirectoryCursor_POSIX::Destroy()
{
	delete this;
}


IGpDirectoryCursor *GpFileSystem_X::ScanDirectoryNested(PortabilityLayer::VirtualDirectory_t virtualDirectory, char const* const* paths, size_t numPaths)
{
	std::string resolvedPath;
	std::vector<std::string> subPaths;
	if (!ResolvePath(virtualDirectory, paths, numPaths, resolvedPath))
		return nullptr;

	DIR *d = opendir(resolvedPath.c_str());
	if (!d)
		return nullptr;

	return new GpDirectoryCursor_POSIX(d);
}


GpFileSystem_X GpFileSystem_X::ms_instance;
