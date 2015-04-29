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

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <dlfcn.h>

#include <strutils.h>
#include "stb_divide.h"

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses)
{
	TUnimplemented();
}

size_t GetNumberOfProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void SleepMS(size_t iMS)
{
	usleep(iMS);
}

void OpenBrowser(const tstring& sURL)
{
	int iSystem = system((tstring("firefox ") + sURL).c_str());
}

void OpenExplorer(const tstring& sDirectory)
{
	int iSystem = system((tstring("gnome-open ") + sDirectory).c_str());
}

void Alert(const tstring& sMessage)
{
	fputs(sMessage.c_str(), stderr);
}

void CreateMinidump(void* pInfo)
{
}

tstring GetClipboard()
{
	return tstring();
}

void SetClipboard(const tstring& sBuf)
{
}

tvector<tstring> ListDirectory(const char* full_directory, bool include_directories)
{
	tstring directory = full_directory;
	if (directory.startswith("$ASSETS/"))
		directory = directory.substr(8);

	tvector<tstring> result;

	struct dirent *dp;

	DIR *dir = opendir((directory).c_str());
	while ((dp=readdir(dir)) != NULL)
	{
		if (!include_directories && (dp->d_type == DT_DIR))
			continue;

		tstring name = dp->d_name;
		if (name == ".")
			continue;

		if (name == "..")
			continue;

		result.push_back(name);
	}
	closedir(dir);

	return result;
}

bool IsFile(const tstring& sPath)
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(sPath.c_str(), &stFileInfo);
	if(intStat == 0 && S_ISREG(stFileInfo.st_mode))
		return true;
	else
		return false;
}

bool IsDirectory(const tstring& sPath)
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(sPath.c_str(), &stFileInfo);
	if(intStat == 0 && S_ISDIR(stFileInfo.st_mode))
		return true;
	else
		return false;
}

void CreateDirectoryNonRecursive(const tstring& sPath)
{
	mkdir(sPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool CopyFileTo(const tstring& sFrom, const tstring& sTo, bool bOverride)
{
	TUnimplemented();
	return 0;
}

tstring FindAbsolutePath(const tstring& sPath)
{
	TUnimplemented();

	char* pszFullPath = realpath(sPath.c_str(), nullptr);
	tstring sFullPath = pszFullPath;
	free(pszFullPath);

	return sFullPath;
}

time_t GetFileModificationTime(const char* pszFile)
{
	struct stat s;
	if (stat(pszFile, &s) != 0)
		return 0;

	return s.st_mtime;
}

void DebugPrint(const char* pszText)
{
	puts(pszText);
}

void Exec(const tstring& sLine)
{
	int iSystem = system((tstring("./") + sLine).c_str());
}

// Not worried about supporting these on Linux right now.
int TranslateKeyToQwerty(int iKey)
{
	return iKey;
}

int TranslateKeyFromQwerty(int iKey)
{
	return iKey;
}

void SetLowPeriodScheduler()
{
	// No op on OSX
}

void ClearLowPeriodScheduler()
{
	// No op on OSX
}

void SetCurrentDirectory(const char* dir)
{
	chdir(dir);
}

size_t LoadBinary(const char* binary)
{
	void* r = dlopen(binary, 0);

	TAssert(r);
	if (!r)
		printf("Couldn't dlopen: %s\n", dlerror());

	return (size_t)r;
}

void FreeBinary(size_t binary)
{
	dlclose((void*)binary);
}

void* GetProcedureAddress(size_t binary_handle, const char* procedure_name)
{
	return dlsym((void*)binary_handle, procedure_name);
}

void MapFile(char* filename, FileMappingInfo* /*OUT*/ mapping_info)
{
	mapping_info->m_created = false;

	int fd = open(filename, O_RDWR);

	if (fd < 0)
	{
		fd = open(filename, O_CREAT | O_TRUNC | O_RDWR);
		mapping_info->m_created = true;
	}

	TAssert(fd >= 0);

	struct stat file_statbuf;

	fstat(fd, &file_statbuf);
	int starting_size = file_statbuf.st_size;

	// The system only gives us allocations at a granularity of 4k, any smaller allocation
	// will include wasted space. So, roll this allocation up to the next nonzero 4k.
	int page_size = sysconf(_SC_PAGESIZE);
	int remainder = stb_mod_eucl(starting_size, page_size);
	int aligned_size;
	if (remainder)
		aligned_size = starting_size - stb_mod_eucl(starting_size, page_size) + page_size;
	else
		aligned_size = std::max(starting_size, page_size);

	int truncated = ftruncate(fd, aligned_size);
	TAssert(truncated == 0);

	mapping_info->m_file_handle = (size_t)fd;
	mapping_info->m_memory_size = aligned_size;
	mapping_info->m_memory = mmap(nullptr, aligned_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);

	TAssert(mapping_info->m_memory != MAP_FAILED);
}

void ResizeMap(FileMappingInfo* mapping_info, size_t minimum_size)
{
	munmap(mapping_info->m_memory, mapping_info->m_memory_size);

	struct stat file_statbuf;

	fstat(mapping_info->m_file_handle, &file_statbuf);
	int file_size = file_statbuf.st_size;

	// I'm not sure this will ever happen. If years go by and this assert is
	// never hit then the fstat and the next tmax below can be removed.
	TAssert(file_size <= minimum_size);

	minimum_size = tmax(minimum_size, file_size);

	int page_size = sysconf(_SC_PAGESIZE);
	int remainder = stb_mod_eucl(minimum_size, page_size);
	int aligned_size;
	if (remainder)
		aligned_size = minimum_size - stb_mod_eucl(minimum_size, page_size) + page_size;
	else
		aligned_size = tmax(minimum_size, page_size);

	int truncated = ftruncate(mapping_info->m_file_handle, aligned_size);
	TAssert(truncated == 0);

	mapping_info->m_memory_size = aligned_size;
	mapping_info->m_memory = mmap(nullptr, aligned_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, mapping_info->m_file_handle, 0);
}

void UnmapFile(FileMappingInfo* mapping_info)
{
	munmap(mapping_info->m_memory, mapping_info->m_memory_size);
	close(mapping_info->m_file_handle);
}

void InitializeNetworking()
{
	// Nothing to do on OSX.
}

