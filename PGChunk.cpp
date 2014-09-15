/*
 *  PGChunk.cpp
 *  C700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include <string.h>
#include "PGChunk.h"

//-----------------------------------------------------------------------------
PGChunk::PGChunk(int allocMemSize)
: FileAccess(NULL, true)
, DataBuffer(allocMemSize)
, mNumPrograms( 0 )
{
}

//-----------------------------------------------------------------------------
PGChunk::PGChunk( const void *data, int dataSize )
: FileAccess(NULL, true)
, DataBuffer( data, dataSize )
, mNumPrograms( 0 )
{
}

//-----------------------------------------------------------------------------
PGChunk::~PGChunk()
{
}

//-----------------------------------------------------------------------------
bool PGChunk::AppendDataFromVP( InstParams *vp )
{
	if ( mReadOnly ) {
		return false;
	}
	
	int		intValue;
	double	doubleValue;
	
	//�v���O������
	if (vp->pgname[0] != 0) {
		writeChunk(kAudioUnitCustomProperty_ProgramName, vp->pgname, PROGRAMNAME_MAX_LEN);
	}

	int	brrSize = vp->brr.size;
	//�ŏI�u���b�N�����[�v�t���O�ɂ���
	if (vp->loop) {
		vp->brr.data[brrSize - 9] |= 2;
	}
	else {
		vp->brr.data[brrSize - 9] &= ~2;
	}
	writeChunk(kAudioUnitCustomProperty_BRRData, vp->brr.data, brrSize);
	doubleValue = vp->rate;
	writeChunk(kAudioUnitCustomProperty_Rate, &doubleValue, sizeof(double));
	intValue = vp->basekey;
	writeChunk(kAudioUnitCustomProperty_BaseKey, &intValue, sizeof(int));
	intValue = vp->lowkey;
	writeChunk(kAudioUnitCustomProperty_LowKey, &intValue, sizeof(int));
	intValue = vp->highkey;
	writeChunk(kAudioUnitCustomProperty_HighKey, &intValue, sizeof(int));
	intValue = vp->lp;
	writeChunk(kAudioUnitCustomProperty_LoopPoint, &intValue, sizeof(int));

	intValue = vp->ar;
	writeChunk(kAudioUnitCustomProperty_AR, &intValue, sizeof(int));
	intValue = vp->dr;
	writeChunk(kAudioUnitCustomProperty_DR, &intValue, sizeof(int));
	intValue = vp->sl;
	writeChunk(kAudioUnitCustomProperty_SL, &intValue, sizeof(int));
	intValue = vp->sr;
	writeChunk(kAudioUnitCustomProperty_SR, &intValue, sizeof(int));

	intValue = vp->volL;
	writeChunk(kAudioUnitCustomProperty_VolL, &intValue, sizeof(int));
	intValue = vp->volR;
	writeChunk(kAudioUnitCustomProperty_VolR, &intValue, sizeof(int));

	intValue = vp->echo ? 1:0;
	writeChunk(kAudioUnitCustomProperty_Echo, &intValue, sizeof(int));
	intValue = vp->bank;
	writeChunk(kAudioUnitCustomProperty_Bank, &intValue, sizeof(int));
	
    intValue = vp->sustainMode ? 1:0;
    writeChunk(kAudioUnitCustomProperty_SustainMode, &intValue, sizeof(int));
	
    intValue = vp->monoMode ? 1:0;
    writeChunk(kAudioUnitCustomProperty_MonoMode, &intValue, sizeof(int));

    intValue = vp->portamentoOn ? 1:0;
    writeChunk(kAudioUnitCustomProperty_PortamentoOn, &intValue, sizeof(int));

    intValue = vp->portamentoRate;
    writeChunk(kAudioUnitCustomProperty_PortamentoRate, &intValue, sizeof(int));

    intValue = vp->noteOnPriority;
    writeChunk(kAudioUnitCustomProperty_NoteOnPriority, &intValue, sizeof(int));
    
    intValue = vp->releasePriority;
    writeChunk(kAudioUnitCustomProperty_ReleasePriority, &intValue, sizeof(int));

	//���g�`���
	intValue = vp->isEmphasized ? 1:0;
	writeChunk(kAudioUnitCustomProperty_IsEmaphasized, &intValue, sizeof(int));
	if ( vp->sourceFile[0] ) {
		writeChunk(kAudioUnitCustomProperty_SourceFileRef, vp->sourceFile, PATH_LEN_MAX);
	}
    
	mNumPrograms++;
	
	return true;
}

//-----------------------------------------------------------------------------
int PGChunk::getPGChunkSize( const InstParams *vp )
{
	int cksize = 0;
	if ( vp->brr.data ) {
		cksize += sizeof( MyChunkHead ) * 23;
		cksize += sizeof( int ) * 19;	//int�^�f�[�^�~14
		cksize += sizeof(double);		//double�^�f�[�^�P��
		cksize += PROGRAMNAME_MAX_LEN;
		cksize += PATH_LEN_MAX;
		cksize += vp->brr.size;
	}
	return cksize;
}

//-----------------------------------------------------------------------------
bool PGChunk::ReadDataToVP( InstParams *vp )
{
	while ( (mDataSize - mDataPos) > (int)sizeof( MyChunkHead ) ) {
		int		ckType;
		long	ckSize;
		readChunkHead(&ckType, &ckSize);
		switch (ckType) {
			case kAudioUnitCustomProperty_ProgramName:
				readData(vp->pgname, ckSize, &ckSize);
				break;
			case kAudioUnitCustomProperty_BRRData:
			{
				if ( vp->brr.data ) {
					delete [] vp->brr.data;
				}
				vp->brr.data = new unsigned char[ckSize];
				long	actSize;
				readData(vp->brr.data, ckSize, &actSize);
				vp->brr.size = actSize;
				vp->loop = vp->brr.data[actSize-9]&2?true:false;
				break;
			}
			case kAudioUnitCustomProperty_Rate:
				readData(&vp->rate, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_BaseKey:
				readData(&vp->basekey, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_LowKey:
				readData(&vp->lowkey, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_HighKey:
				readData(&vp->highkey, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_LoopPoint:
				readData(&vp->lp, ckSize, NULL);
				break;

			case kAudioUnitCustomProperty_AR:
				readData(&vp->ar, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_DR:
				readData(&vp->dr, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_SL:
				readData(&vp->sl, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_SR:
				readData(&vp->sr, ckSize, NULL);
				break;

            case kAudioUnitCustomProperty_SustainMode:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->sustainMode = value ? true:false;
				break;
			}
            
			case kAudioUnitCustomProperty_VolL:
				readData(&vp->volL, ckSize, NULL);
				break;
			case kAudioUnitCustomProperty_VolR:
				readData(&vp->volR, ckSize, NULL);
				break;
				
			case kAudioUnitCustomProperty_Echo:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->echo = value ? true:false;
				break;
			}
			case kAudioUnitCustomProperty_Bank:
				readData(&vp->bank, ckSize, NULL);
				break;
				
            case kAudioUnitCustomProperty_MonoMode:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->monoMode = value ? true:false;
				break;
			}
            
            case kAudioUnitCustomProperty_PortamentoOn:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->portamentoOn = value ? true:false;
				break;
			}
                
            case kAudioUnitCustomProperty_PortamentoRate:
				readData(&vp->portamentoRate, ckSize, NULL);
				break;
                
            case kAudioUnitCustomProperty_NoteOnPriority:
				readData(&vp->noteOnPriority, ckSize, NULL);
				break;
                
            case kAudioUnitCustomProperty_ReleasePriority:
				readData(&vp->releasePriority, ckSize, NULL);
				break;
                
			case kAudioUnitCustomProperty_IsEmaphasized:
			{
				int value;
				readData(&value, ckSize, NULL);
				vp->isEmphasized = value ? true:false;
				break;
			}
			case kAudioUnitCustomProperty_SourceFileRef:
				readData(vp->sourceFile, ckSize, &ckSize);
				break;
			default:
				//�s���`�����N�̏ꍇ�͔�΂�
				AdvDataPos(ckSize);
				break;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool PGChunk::writeChunk( int type, const void *data, int byte )
{
	if ( mReadOnly ) {
		return false;
	}
	
	MyChunkHead	ckHead = {type, byte};
	
	//�󂫗e�ʃ`�F�b�N
	if ( mDataSize < ( mDataPos + byte + (int)sizeof(MyChunkHead) ) ) {
		return false;
	}
	
	long	writeSize;
	if ( writeData(&ckHead, sizeof(MyChunkHead), &writeSize) == false ) {
		return false;
	}
	
	if ( writeData(data, byte, &writeSize) == false ) {
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool PGChunk::readChunkHead( int *type, long *byte )
{
	long		toRead = sizeof( MyChunkHead );
	
	MyChunkHead	head;
	if ( readData(&head, toRead, &toRead) == false ) {
		return false;
	}
	(*type) = head.type;
	(*byte) = head.size;
	return true;
}
