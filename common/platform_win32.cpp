/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <tinker_platform.h>

#include <windows.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <dbghelp.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tstring.h>
#include "stb_divide.h"

size_t GetNumberOfProcessors()
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);
	return system_info.dwNumberOfProcessors;
}

void SleepMS(size_t milliseconds)
{
	Sleep((DWORD)milliseconds);
}

void OpenBrowser(const tstring& url)
{
	ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void OpenExplorer(const tstring& directory)
{
	ShellExecuteA(NULL, "open", directory.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void Alert(const tstring& message)
{
	MessageBoxA(NULL, message.c_str(), "Alert", MB_ICONWARNING|MB_OK);
}

static int g_iMinidumpsWritten = 0;

#ifdef _DEBUG
void CreateMinidump(void* /*pInfo*/)
{
}
#else
void CreateMinidump(void* /*pInfo*/)
{
#if 0
	time_t current_time = ::time(NULL);
	struct tm * time = ::localtime(&current_time);

	char module_name[MAX_PATH];
	::GetModuleFileNameA(NULL, module_name, sizeof(module_name) / sizeof(tchar));
	char *module = strchr(module_name, '.');

	if (module)
		*module = 0;

	module = strchr(module_name, '\\');
	if (module)
		module++;
	else
		module = "unknown";

	char file_name[MAX_PATH];
	_snprintf(file_name, sizeof(file_name) / sizeof(tchar),
			"%s_%d.%.2d.%2d.%.2d.%.2d.%.2d_%d.mdmp",
			module,
			time->tm_year + 1900,
			time->tm_mon + 1,
			time->tm_mday,
			time->tm_hour,
			time->tm_min,
			time->tm_sec,
			g_iMinidumpsWritten++
			);

	HANDLE file_handle = CreateFile((std::string("dumps/") + file_name).c_str(), GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	if ((file_handle != NULL) && (file_handle != INVALID_HANDLE_VALUE))
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId           = GetCurrentThreadId();
		mdei.ExceptionPointers  = (EXCEPTION_POINTERS*)pInfo;
		mdei.ClientPointers     = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine     = NULL;
		mci.CallbackParam       = 0;

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

		BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(),
			file_handle, mdt, (pInfo != 0) ? &mdei : 0, 0, &mci);

		if( rv )
		{
			// Success... message to user?
		}

		CloseHandle(file_handle);
	}
#endif
}
#endif

tstring GetClipboard()
{
	if (!OpenClipboard(NULL))
		return "";

	HANDLE data_handle = GetClipboardData(CF_TEXT);
	char* buffer = (char*)GlobalLock(data_handle);
	GlobalUnlock(data_handle);
	CloseClipboard();

	tstring sClipboard(buffer);

	return sClipboard;
}

void SetClipboard(const tstring& sBuf)
{
	if (!OpenClipboard(NULL))
		return;

	EmptyClipboard();

	HGLOBAL clipboard;
	clipboard = GlobalAlloc(GMEM_MOVEABLE, sBuf.length() + 1);

	char* buffer = (char*)GlobalLock(clipboard);
	strcpy(buffer, LPCSTR(sBuf.c_str()));

	GlobalUnlock(clipboard);

	SetClipboardData(CF_TEXT, clipboard);

	CloseClipboard();
}

tvector<tstring> ListDirectory(const char* directory, bool include_directories)
{
	if (tstrncmp(directory, "$ASSETS/", strlen(directory), 8) == 0)
		directory += 8;

	tvector<tstring> result;

	char path[MAX_PATH];
	sprintf(path, "%s\\*", directory);

	WIN32_FIND_DATA fd;
	HANDLE find = FindFirstFileA(path, &fd);

	if (find != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!include_directories && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// Duh.
			if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
				continue;

			result.push_back(fd.cFileName);
		} while (FindNextFile(find, &fd));

		FindClose(find);
	}

	return result;
}

bool IsFile(const tstring& path)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFileA(path.c_str(), &fd);
	FindClose(hFind);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return false;

	return true;
}

bool IsDirectory(const tstring& path)
{
	tstring path_no_separator = path;

	while (path_no_separator.substr(path_no_separator.length() - 1) == T_DIR_SEP)
		path_no_separator = path_no_separator.substr(0, path_no_separator.length() - 1);

	WIN32_FIND_DATA fd;
	HANDLE find = FindFirstFileA(path_no_separator.c_str(), &fd);
	FindClose(find);

	if (find == INVALID_HANDLE_VALUE)
		return false;

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

void CreateDirectoryNonRecursive(const tstring& path)
{
	CreateDirectory(path.c_str(), NULL);
}

bool CopyFileTo(const tstring& from, const tstring& to, bool override)
{
	if (IsFile(to) && override)
		::DeleteFile(to.c_str());

	return !!CopyFile(from.c_str(), to.c_str(), true);
}

tstring FindAbsolutePath(const tstring& path)
{
	char copy_path[MAX_PATH];
	std::string path_s;

	if (!path.length())
		path_s = ".";
	else
		path_s = path;

	GetFullPathName(path_s.c_str(), MAX_PATH, copy_path, nullptr);

	return copy_path;
}

time_t GetFileModificationTime(const char* file)
{
	struct stat s;
	if (stat(file, &s) != 0)
		return 0;

	return s.st_mtime;
}

void DebugPrint(const char* text)
{
	OutputDebugStringA(text);
}

void Exec(const tstring& line)
{
	system(line.c_str());
}

int GetVKForChar(int c)
{
	switch (c)
	{
	case ';':
		return VK_OEM_1;

	case '/':
		return VK_OEM_2;

	case '`':
		return VK_OEM_3;

	case '[':
		return VK_OEM_4;

	case '\\':
		return VK_OEM_5;

	case ']':
		return VK_OEM_6;

	case '\'':
		return VK_OEM_7;

	case '=':
		return VK_OEM_PLUS;

	case ',':
		return VK_OEM_COMMA;

	case '-':
		return VK_OEM_MINUS;

	case '.':
		return VK_OEM_PERIOD;
	}

	return c;
}

int GetCharForVK(int c)
{
	switch (c)
	{
	case VK_OEM_1:
		return ';';

	case VK_OEM_2:
		return '/';

	case VK_OEM_3:
		return '`';

	case VK_OEM_4:
		return '[';

	case VK_OEM_5:
		return '\\';

	case VK_OEM_6:
		return ']';

	case VK_OEM_7:
		return '\'';

	case VK_OEM_PLUS:
		return '=';

	case VK_OEM_COMMA:
		return ',';

	case VK_OEM_MINUS:
		return '-';

	case VK_OEM_PERIOD:
		return '.';
	}

	return c;
}

static HKL g_iEnglish = LoadKeyboardLayout("00000409", 0);

// If we are using a non-qwerty layout, find the qwerty key that matches this letter's position on the keyboard.
int TranslateKeyToQwerty(int key)
{
	TUnimplemented(); // Move g_iEnglish into tinker memory

	HKL iCurrent = GetKeyboardLayout(0);

	if (iCurrent == g_iEnglish)
		return key;

	UINT i = MapVirtualKeyEx(GetVKForChar(key), MAPVK_VK_TO_VSC, iCurrent);
	return (int)GetCharForVK(MapVirtualKeyEx(i, MAPVK_VSC_TO_VK, g_iEnglish));
}

// If we are using a non-qwerty layout, find the localized key that matches this letter's position in qwerty.
int TranslateKeyFromQwerty(int key)
{
	TUnimplemented(); // Move g_iEnglish into tinker memory

	HKL iCurrent = GetKeyboardLayout(0);

	if (iCurrent == g_iEnglish)
		return key;

	UINT i = MapVirtualKeyEx(GetVKForChar(key), MAPVK_VK_TO_VSC, g_iEnglish);
	return (int)GetCharForVK(MapVirtualKeyEx(i, MAPVK_VSC_TO_VK, iCurrent));
}

void GetScreenDPI(float& xdpi, float& ydpi)
{
	HDC hdc = GetDC(NULL);
	if (hdc)
	{
		xdpi = (float)GetDeviceCaps(hdc, LOGPIXELSX);
		ydpi = (float)GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
	}
	else
	{
		xdpi = 96;
		ydpi = 96;
	}
}

// Don't need to.
void EnableMulticast() {}

void SetLowPeriodScheduler()
{
	timeBeginPeriod(1);
}

void ClearLowPeriodScheduler()
{
	timeEndPeriod(1);
}

void InitializeNetworking()
{
	WSADATA wsadata;
	int result = WSAStartup(MAKEWORD(2, 0), &wsadata);
	TCheck(!result);
}

size_t LoadBinary(const char* binary)
{
	return (size_t)LoadLibraryA(binary);
}

void FreeBinary(size_t binary)
{
	FreeLibrary((HMODULE)binary);
}

void* GetProcedureAddress(size_t binary_handle, char* procedure_name)
{
	return GetProcAddress((HMODULE)binary_handle, procedure_name);
}

void MapFile(char* filename, FileMappingInfo* mapping_info)
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);

	TAssert(sizeof(HFILE) <= sizeof(HANDLE));

	mapping_info->m_created = false;

	OFSTRUCT ofstruct;
	HANDLE file_handle = (HANDLE)OpenFile(filename, &ofstruct, OF_READWRITE);

	if ((HFILE)file_handle == HFILE_ERROR)
	{
		file_handle = (HANDLE)OpenFile(filename, &ofstruct, OF_CREATE|OF_READWRITE);
		mapping_info->m_created = true;
	}

	TAssert((HFILE)file_handle != HFILE_ERROR);

	// The system only gives us allocations at a granularity of 64k, any smaller allocation
	// will include wasted space. So, roll this allocation up to the next nonzero 64k.
	DWORD starting_size = GetFileSize(file_handle, NULL);

	int remainder = stb_mod_eucl(starting_size, system_info.dwAllocationGranularity);
	DWORD aligned_size;
	if (remainder)
		aligned_size = starting_size - stb_mod_eucl(starting_size, system_info.dwAllocationGranularity) + system_info.dwAllocationGranularity;
	else
		aligned_size = std::max(starting_size, system_info.dwAllocationGranularity);

	// We have to resize the file before we make the mapping to be able to use the extra space.
	DWORD set_file_pointer_result = SetFilePointer(file_handle, aligned_size, NULL, FILE_BEGIN);
	if (set_file_pointer_result != INVALID_SET_FILE_POINTER)
	{
		int err = SetEndOfFile(file_handle);
		TAssert(err != 0);
	}

	HANDLE file_mapping_handle = CreateFileMapping(file_handle, NULL, PAGE_READWRITE, 0, 0, NULL);

	TAssert(sizeof(size_t) >= sizeof(HANDLE));
	mapping_info->m_file_handle = (size_t)file_handle;
	mapping_info->m_file_mapping_handle = (size_t)file_mapping_handle;
	mapping_info->m_memory_size = aligned_size;
	mapping_info->m_memory = MapViewOfFile(file_mapping_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
}

void UnmapFile(FileMappingInfo* mapping_info)
{
	UnmapViewOfFile(mapping_info->m_memory);

	TUnimplemented(); // Reduce the file size to what was actually used.

	CloseHandle((HANDLE)mapping_info->m_file_mapping_handle);
	CloseHandle((HANDLE)mapping_info->m_file_handle);
}
