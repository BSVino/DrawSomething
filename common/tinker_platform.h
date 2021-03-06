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

#ifndef _LW_PLATFORM
#define _LW_PLATFORM

#include <tstring.h>
#include <tvector.h>

void GetMACAddresses(unsigned char*& paiAddresses, size_t& iAddresses);
size_t GetNumberOfProcessors();
void SleepMS(size_t iMS);
void OpenBrowser(const tstring& sURL);
void OpenExplorer(const tstring& sDirectory);
void Alert(const tstring& sMessage);
void CreateMinidump(void* pInfo);
tstring GetClipboard();
void SetClipboard(const tstring& sBuf);
tvector<tstring> ListDirectory(const char* directory, bool include_directories = true);
bool IsFile(const tstring& sPath);
bool IsDirectory(const tstring& sPath);
void CreateDirectoryNonRecursive(const tstring& sPath);
bool CopyFileTo(const tstring& sFrom, const tstring& sTo, bool bOverride = true);
tstring FindAbsolutePath(const tstring& sPath);
time_t GetFileModificationTime(const char* pszFile);
void DebugPrint(const char* pszText);
void Exec(const tstring& sLine);
int TranslateKeyToQwerty(int iKey);
int TranslateKeyFromQwerty(int iKey);
void GetScreenDPI(float& xdpi, float& ydpi); // The number of physical pixels per inch.
void EnableMulticast();
void SetLowPeriodScheduler();
void ClearLowPeriodScheduler();
void InitializeNetworking();
size_t LoadBinary(const char* binary);
void FreeBinary(size_t binary);
void* GetProcedureAddress(size_t binary_handle, const char* procedure_name);
void SetCurrentDirectory(const char* dir);

struct FileMappingInfo
{
	size_t m_file_handle;
#ifdef _WIN32
	size_t m_file_mapping_handle;
#endif
	size_t m_memory_size;
	void*  m_memory;
	uint8  m_created : 1;
};

void MapFile(char* filename, FileMappingInfo* /*OUT*/ mapping_info);
void ResizeMap(FileMappingInfo* mapping_info, size_t minimum_size);
void UnmapFile(FileMappingInfo* mapping_info);

#ifdef _WIN32
#define T_DIR_SEP "\\"
#else
#define T_DIR_SEP "/"
#endif

#ifdef __ANDROID__
#define T_ASSETS_PREFIX "$ASSETS/"
#else
#define T_ASSETS_PREFIX ""
#endif

#ifdef _WIN32
#define TDLLEXPORT __declspec(dllexport)
#else
#define TDLLEXPORT
#endif

#endif
