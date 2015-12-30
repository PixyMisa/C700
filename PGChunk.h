/*
 *  PGChunk.h
 *  C700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "C700defines.h"
#include "DataBuffer.h"
#include "FileAccess.h"

class PGChunk : public FileAccess, public DataBuffer {
public:
	PGChunk( int allocMemSize );
	PGChunk( const void *data, int dataSize );
	virtual ~PGChunk();
	
	bool				AppendDataFromVP( const InstParams *vp );
	bool				ReadDataToVP( InstParams *vp );
	
	bool				writeChunk( int type, const void* data, int byte );	//�e�ʕs���őS�����������߂Ȃ��Ƃ���false
	bool				readChunkHead( int *type, long *byte );
	
	static int			getPGChunkSize( const InstParams *vp );

	typedef struct {
		int		type;
		long	size;	//�w�b�_���������f�[�^�T�C�Y
	} MyChunkHead;
	
private:	
	static const int CKID_NULL_DATA	= 0;
	int				mNumPrograms;
};
