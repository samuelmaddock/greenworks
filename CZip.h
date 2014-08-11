/*
	ported from:

	minizip.c
	Version 1.1, February 14h, 2010
	sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

	Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

	Modifications of Unzip for Zip64
	Copyright (C) 2007-2008 Even Rouault

	Modifications for Zip64 support on both zip and unzip
	Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

	Porting for Greenworks integration of both zip and unzip by Francesco Abbattista
	Copyright (C) 2014 Greenheart Games ( http://greenheartgames.com )

*/
#ifndef ZIP_H
#define ZIP_H

#ifndef USE_FILE32API
#define USE_FILE32API
#endif

#ifndef _WIN32
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
#define _FILE_OFFSET_BIT 64
#endif
#endif

#include "Includes.h"

#ifdef _WIN32
	#include <direct.h>
	#include <io.h>
#else
	#include <unistd.h>
	#include <utime.h>
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

#include "minizip/zip.h"
#include "CUtils.h"

#ifdef _WIN32
	#define USEWIN32IOAPI
	#include "minizip/iowin32.h"
#endif

#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

class CZip
{
	private:
		uLong filetime(char *f, tm_zip *tmzip, uLong *dt);
		int check_exist_file(const char* filename);
		int getFileCrc(const char* filenameinzip, void* buf, unsigned long size_buf, unsigned long* result_crc);
		int isLargeFile(const char* filename);
	public:
		int zip(const char* targetFile, const char* sourceDir, int compressionLevel, const char* password);
};
#endif 