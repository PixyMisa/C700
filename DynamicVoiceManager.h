﻿//
//  DynamicVoiceManager.h
//  C700
//
//  Created by osoumen on 2014/11/30.
//
//

#ifndef __C700__DynamicVoiceManager__
#define __C700__DynamicVoiceManager__

#include <iostream>
#include <list>

class DynamicVoiceManager
{
public:
    enum VoiceAllocMode {
        ALLOC_MODE_OLDEST = 0,
        ALLOC_MODE_SAMECH
    };
    DynamicVoiceManager();
    ~DynamicVoiceManager();
    
    void    Initialize(int voiceLimit);
    void    Reset();
    void    ChangeVoiceLimit(int voiceLimit);
    void    SetVoiceAllocMode(VoiceAllocMode mode);
    int     GetVoiceLimit() { return mVoiceLimit; }
    int     AllocVoice(int prio, int ch, int uniqueID, bool monoMode,
                       int *releasedCh, bool *isLegato);
    int     ReleaseVoice(int relPrio, int ch, int uniqueID, int *relVo);
    bool    IsPlayingVoice(int v);
    void    SetChLimit(int ch, int value);
    int     GetChLimit(int ch);
    int     GetNoteOns(int ch);
    int     GetVoiceMidiCh(int vo) { return mVoCh[vo]; }
    int     GetVoiceUniqueID(int vo) { return mVoUniqueID[vo]; }
    void    SetKeyOn(int vo);
    bool    IsKeyOn(int vo);
    
private:
    static const int MAX_VOICE = 16;
    std::list<int>	mPlayVo;				//ノートオン状態のボイス
	std::list<int>	mWaitVo;				//ノートオフ状態のボイス
    int mVoCh[MAX_VOICE];
    int mVoPrio[MAX_VOICE];
    int mVoUniqueID[MAX_VOICE];
    bool mVoKeyOn[MAX_VOICE];
    int mVoiceLimit;
    int mChNoteOns[16];
    int mChLimit[16];
    VoiceAllocMode mAllocMode;
    
    void pushWaitVo(int vo);
    int	findFreeVoice();
    int stealVoice(int ch);
    int findVoice(int ch=-1);
};

#endif /* defined(__C700__DynamicVoiceManager__) */
