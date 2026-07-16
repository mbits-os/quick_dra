// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <Windows.h>
#include <quick_dra/version.hpp>

#define STRINGIFY1(x) #x
#define STRINGIFY(x) STRINGIFY1(x)

#define RC_ORGANISATION "midnightBITS"
#ifndef RC_MODULE
#define RC_MODULE "quick-dra"
#endif
#define RC_VERSION_STRING \
	QUICK_DRA_VERSION_STR       \
	QUICK_DRA_VERSION_STABILITY

#define RC_VERSION QUICK_DRA_VERSION_MAJOR, QUICK_DRA_VERSION_MINOR, QUICK_DRA_VERSION_PATCH, 0

LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US

VS_VERSION_INFO VERSIONINFO
	FILEVERSION    RC_VERSION
	PRODUCTVERSION RC_VERSION
	FILETYPE       VFT_APP
{
	BLOCK "StringFileInfo"
	{
		BLOCK "040904b0"
		{
			VALUE "CompanyName", RC_ORGANISATION
			VALUE "FileDescription", L"Quick-DRA for Windows"
			VALUE "FileVersion", RC_VERSION_STRING
			VALUE "InternalName", RC_MODULE
			VALUE "OriginalFilename", RC_MODULE ".exe"
			VALUE "ProductName", L"Quick-DRA"
			VALUE "ProductVersion", RC_VERSION_STRING
			VALUE "LegalCopyright", "Copyright (c) 2026 by " RC_ORGANISATION
		}
	}
	BLOCK "VarFileInfo"
	{
		VALUE "Translation", 0x409, 1200
	}
}

LANGUAGE LANG_POLISH, SUBLANG_NEUTRAL

VS_VERSION_INFO VERSIONINFO
	FILEVERSION    RC_VERSION
	PRODUCTVERSION RC_VERSION
	FILETYPE       VFT_APP
{
	BLOCK "StringFileInfo"
	{
		BLOCK "041504b0"
		{
			VALUE "CompanyName", RC_ORGANISATION
			VALUE "FileDescription", L"Quick-DRA dla Windows"
			VALUE "FileVersion", RC_VERSION_STRING
			VALUE "InternalName", RC_MODULE
			VALUE "OriginalFilename", RC_MODULE ".exe"
			VALUE "ProductName", L"Quick-DRA"
			VALUE "ProductVersion", RC_VERSION_STRING
			VALUE "LegalCopyright", "Copyright (c) 2026 by " RC_ORGANISATION
		}
	}
	BLOCK "VarFileInfo"
	{
		VALUE "Translation", 0x415, 1200
	}
}
