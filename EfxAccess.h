/*
 *  EfxAccess.h
 *  C700
 *
 *  Created by osoumen on 12/10/08.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *  GUI������G�t�F�N�^�փA�N�Z�X���邽�߂̃N���X
 */

#pragma once

#include "C700defines.h"

#if AU
#include <AudioUnit/AUComponent.h>
#include <AudioToolbox/AudioToolbox.h>
#else
#include "audioeffectx.h"
class C700VST;
#endif

#include "SPCFile.h"
#include "PlistBRRFile.h"
#include "RawBRRFile.h"
#include "AudioFile.h"
#include "XIFile.h"
#include "C700Properties.h"

class EfxAccess
{
public:
	EfxAccess( void *efx );
	~EfxAccess();
#if AU
	void	SetEventListener( AUEventListenerRef listener ) { mEventListener = listener; }
#endif
	
	bool	CreateBRRFileData( RawBRRFile **outData );
	bool	CreateXIFileData( XIFile **outData );
	bool	CreatePlistBRRFileData( PlistBRRFile **outData );
	bool	SetPlistBRRFileData( const PlistBRRFile *data );
	
	bool	SetSourceFilePath( const char *path );
	bool	GetSourceFilePath( char *path, int maxLen );
	bool	SetProgramName( const char *pgname );
	bool	GetProgramName( char *pgname, int maxLen );
	bool	GetBRRData( BRRData *data );
	bool	SetBRRData( const BRRData *data );
	
	float	GetPropertyValue( int propertyId );
	void	SetPropertyValue( int propertyID, float value );
	float	GetParameter( int parameterId );
	void	SetParameter( void *sender, int index, float value );
	
    bool	SetSongRecordPath( const char *path );
	bool	GetSongRecordPath( char *path, int maxLen );
    bool	SetSongInfoString( int propertyId, const char *string );
	bool	GetSongInfoString( int propertyId, char *string, int maxLen );
    bool    SetSongPlayerCode( const void *data, int size );
    double  GetHostBeatPos();
    
private:
#if AU
	AudioUnit			mAU;
	AUEventListenerRef	mEventListener;
#else
	C700VST*			mEfx;
#endif
    std::map<int, PropertyDescription>  mPropertyParams;
};
