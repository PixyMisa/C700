/*
 *  SPCFile.h
 *  C700
 *
 *  Created by osoumen on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "SNES_SPC.h"
#include "FileAccess.h"

class SPCFile : public FileAccess {
public:
	SPCFile( const char *path, bool isWriteable );
	virtual ~SPCFile();
	
	unsigned char	*GetRamData() { return mSpcPlay.GetRam(); }
	unsigned char	*GetSampleIndex( int sampleIndex, int *size );
	int				GetLoopSizeIndex( int samleIndex );	//�����Ń��[�v����
	
	virtual bool	Load();
	
private:
	static const int SPC_READ_SIZE = 0x101c0;
	
    SNES_SPC        mSpcPlay;
    
	int				mSrcTableAddr;
	int				mSampleStart[128];
	int				mLoopSize[128];
	int				mSampleBytes[128];
	bool			mIsLoop[128];
};
