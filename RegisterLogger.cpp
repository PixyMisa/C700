/*
 *  RegisterLogger.cpp
 *  VOPM
 *
 *  Created by osoumen on 12/11/04.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "RegisterLogger.h"
#include <string.h>

#if __APPLE_CC__
#include <Carbon/Carbon.h>
#endif

#if _WIN32
#include <windows.h>
#endif

//-----------------------------------------------------------------------------
RegisterLogger::RegisterLogger(int allocSize)
: m_pData( NULL )
, mDataSize( allocSize )
, mDataUsed( 0 )
, mDataPos( 0 )
, mTimeNumerator( 1 )
, mTimeDenominator( 15734 )
{
	if ( allocSize > 0 ) {
		m_pData = new unsigned char[allocSize];
	}
	BeginDump(0);
}

//-----------------------------------------------------------------------------
RegisterLogger::~RegisterLogger()
{
	if ( m_pData != NULL ) {
		delete [] m_pData;
	}
}

//-----------------------------------------------------------------------------
bool RegisterLogger::SetPos( int pos )
{
	if ( mDataSize < pos ) {
		return false;
	}
	mDataPos = pos;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::SaveToFile( const char *path, int clock )
{
	//�w�b�_�[��t���ă_���v���e���t�@�C���ɏ����o��
    /*
	S98Header	header = {
		{ 'S','9','8' },
		'3',
		mTimeNumerator,
		mTimeDenominator,
		0,								// COMPRESSING The value is 0 always.
		sizeof(S98Header) + mDataUsed,	// FILE OFFSET TO TAG
		sizeof(S98Header),				// FILE OFFSET TO DUMP DATA
		sizeof(S98Header) + mLoopPoint,	// FILE OFFSET TO LOOP POINT DUMP DATA
		1,								// DEVICE COUNT
		5,								// DEVICE TYPE (OPM)
		clock,							// CLOCK(Hz)
		0,								// PAN
		0								// RESERVE
	};
	
	char	tag[] = "[S98]title=Title\nartist=Artist\ncopyright=(c)\n";
     */
    int loopAddr = mLoopPoint + 3;
    unsigned char loopStart[3];
    loopStart[0] = loopAddr & 0xff;
    loopStart[1] = ((loopAddr >> 8) & 0x7f) + 0x80;
    loopStart[2] = (loopAddr >> 15) & 0xff;
	
#if __APPLE_CC__
	CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
	
	CFWriteStreamRef	filestream = CFWriteStreamCreateWithFile(NULL,savefile);
	if (CFWriteStreamOpen(filestream)) {
		//CFWriteStreamWrite(filestream, reinterpret_cast<UInt8*> (&header), sizeof(S98Header) );
        CFWriteStreamWrite(filestream, loopStart, 3 );
		CFWriteStreamWrite(filestream, m_pData, mDataUsed );
		//CFWriteStreamWrite(filestream, reinterpret_cast<UInt8*> (tag), sizeof(tag) );
		CFWriteStreamClose(filestream);
	}
	CFRelease(filestream);
	CFRelease(savefile);
	
	return true;
#else
	HANDLE	hFile;
	
	hFile = CreateFile( path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD	writeSize;
		//WriteFile( hFile, &header, sizeof(S98Header), &writeSize, NULL );
        WriteFile( hFile, loopStart, 3, &writeSize, NULL );
		WriteFile( hFile, m_pData, mDataUsed, &writeSize, NULL );
		//WriteFile( hFile, tag, sizeof(tag), &writeSize, NULL );
		CloseHandle( hFile );
	}
	return true;
#endif
}

//-----------------------------------------------------------------------------
void RegisterLogger::SetResolution( int numerator, int denominator )
{
	mTimeNumerator = numerator;
	mTimeDenominator = denominator;
}

//-----------------------------------------------------------------------------
void RegisterLogger::BeginDump( int time )
{
	mDumpBeginTime = time;
	mPrevTime = mDumpBeginTime;
	for ( int i=0; i<256; i++ ) {
        mReg[i] = -1;
	}
	mDataUsed = 0;
	mDataPos = 0;
	mLoopPoint = 0;
	mIsEnded = false;
	
//	printf("--BeginDump--\n");
}

//-----------------------------------------------------------------------------
bool RegisterLogger::DumpReg( int device, int addr, unsigned char data, int time )
{
    if (time < mDumpBeginTime) {
        return false;
    }
    
	if (addr >= 0 && addr < 128) {
		
		if ( mReg[addr] != data || addr == 0x4c || addr == 0x5c) {
			mReg[addr] = data;

			writeWaitFromPrev(time);
			
			if ( GetWritableSize() >= 3 ) {
				writeByte( device );
				writeByte( addr );
				writeByte( data );
			}
			
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
void RegisterLogger::MarkLoopPoint()
{
	mLoopPoint = mDataPos;
	//���[�v����͏�Ƀ��W�X�^���������܂��悤�ɂ���
	for ( int i=0; i<256; i++ ) {
		mReg[i] = -1;
	}
//	printf("--MarkLoopPoint--\n");
}

//-----------------------------------------------------------------------------
void RegisterLogger::EndDump(int time)
{
	if ( mDataUsed > 0 && mIsEnded == false ) {
		writeWaitFromPrev(time);
		writeEndByte();
		mIsEnded = true;
		
//		printf("--EndDump-- %d\n",time);
	}
}

//-----------------------------------------------------------------------------
bool RegisterLogger::writeByte( unsigned char byte )
{
	if ( ( mDataPos + 1 ) > (mDataSize-1) ) {	//END/LOOP���������߂�l�ɂP�o�C�g�c���Ă���
		return false;
	}
	m_pData[mDataPos] = byte;
	mDataPos++;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::writeEndByte()
{
	if ( ( mDataPos + 1 ) > mDataSize) {
		return false;
	}
	m_pData[mDataPos] = 0xfd;
	mDataPos++;
	if ( mDataPos > mDataUsed ) {
		mDataUsed = mDataPos;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool RegisterLogger::writeWaitFromPrev(int time)
{
	bool		result;
	
	int		now_time	= time - mDumpBeginTime;
	int		prev_time	= mPrevTime - mDumpBeginTime;
	int		adv_time	= now_time - prev_time;
	
	//�擪�ɋ󔒂�����Δ�΂�
	if ( mPrevTime == mDumpBeginTime ) {
		adv_time = 0;
	}
	
    long div = adv_time >> 16;
    long mod = adv_time & 0xffff;
    
    for (int i=0; i<div; i++) {
        result = writeByte(0xfe);
        if ( result == false ) return false;
        result = writeByte(0xff);
        if ( result == false ) {
            //�������񂾕��������߂�
            mDataPos -= 1;
            mDataUsed = mDataPos;
            return false;
        }
        result = writeByte(0xff);
        if ( result == false ) {
            //�������񂾕��������߂�
            mDataPos -= 2;
            mDataUsed = mDataPos;
            return false;
        }
    }
    if (mod > 0) {
        if (mod < 0x100) {
            result = writeByte(0xfc);
            if ( result == false ) return false;
            result = writeByte(mod & 0xff);
            if ( result == false ) {
                //�������񂾕��������߂�
                mDataPos -= 1;
                mDataUsed = mDataPos;
                return false;
            }
        }
        else {
            result = writeByte(0xfe);
            if ( result == false ) return false;
            result = writeByte(mod & 0xff);
            if ( result == false ) {
                //�������񂾕��������߂�
                mDataPos -= 1;
                mDataUsed = mDataPos;
                return false;
            }
            result = writeByte(mod >> 8);
            if ( result == false ) {
                //�������񂾕��������߂�
                mDataPos -= 2;
                mDataUsed = mDataPos;
                return false;
            }
        }
    }
    
	mPrevTime = time;
    /*
	if ( adv_time < 0 ) {
		mPrevTime -= adv_time;
	}*/
	return true;
}
