/*
 *  FileAccess.h
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#if _WIN32
#include <windows.h>
#endif

class FileAccess {
public:
	FileAccess( const char *path, bool isWriteable, int pathLenMax=PATH_LEN_MAX );
	virtual ~FileAccess();
	
	const char*		GetFilePath() { return mPath; }
	void			SetFilePath( const char *path );
	virtual bool	Load() { return false; };
	virtual bool	Write() { return false; }
	virtual bool	IsLoaded() const { return mIsLoaded; }
	
protected:
	char		*mPath;
	bool		mIsWriteable;
	bool		mIsLoaded;
    int         mPathLenMax;
};
