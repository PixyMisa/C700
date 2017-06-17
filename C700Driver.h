/*
 *  C700Driver.h
 *  C700
 *
 *  Created by osoumen on 06/09/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#pragma once

#include "MemManager.h"
#include "C700defines.h"
#include "C700DSP.h"
#include "MidiDriverBase.h"

//-----------------------------------------------------------------------------
typedef enum
{
    kVelocityMode_Constant,
    kVelocityMode_Square,
    kVelocityMode_Linear
} velocity_mode;
//-----------------------------------------------------------------------------
typedef enum
{
    kEngineType_Old = 0,
    kEngineType_Relaxed,
    kEngineType_Accurate
} engine_type;
//-----------------------------------------------------------------------------
typedef enum
{
    kVoiceAllocMode_Oldest = 0,
    kVoiceAllocMode_SameChannel,
} voicealloc_mode;


//-----------------------------------------------------------------------------
class C700Driver : public MidiDriverBase
{
public:
    typedef struct VoiceStatus {
		int				pb;
		bool			reg_pmod;
        float           pitch;
		
		//int				ar,dr,sl,sr,vol_l,vol_r;    // �~���[
        int             vol_l,vol_r;
		
		int				velo;
        int             volume;
        int             expression;
        int             pan;
        int             srcn;
        //unsigned char	*brrdata;    // �~���[
		//unsigned int	loopPoint;    // �~���[
		//bool			loop;    // �~���[
        
		//bool			echoOn;    // �~���[
        bool            non;
        
        Portament_Linear porta;
        Lfo_Linear      lfo;
        
        VoiceStatus() : pb(0), reg_pmod(0), pitch(0),
                        vol_l(0), vol_r(0), velo(0), volume(0), expression(0), pan(0), srcn(0) {}
        void Reset();
	} VoiceStatus;
    
	C700Driver();
	virtual				~C700Driver();
	
	virtual void		Reset();

    virtual void        doPreMidiEvents();
    virtual void        doPostMidiEvents();
 
    
    void        StartRegisterLog( int inFrame );
    void        MarkLoopRegisterLog( int inFrame );
    void        EndRegisterLog( int inFrame );
    
    // global params
	void		SetVoiceLimit( int value );
	void		SetPBRange( float value );
	void		SetEngineType( engine_type type );
    void        SetVoiceAllocMode( voicealloc_mode mode );
    void        SetFastReleaseAsKeyOff( bool value );
	void		SetMultiMode( int bank, bool value );
	bool		GetMultiMode( int bank ) const;
	void		SetVelocityMode( velocity_mode value );
	void		SetVibFreq( int ch, float value );
	void		SetVibDepth( int ch, float value );
	void		SetMainVol_L( int value );
	void		SetMainVol_R( int value );
	void		SetEchoVol_L( int value );
	void		SetEchoVol_R( int value );
	void		SetFeedBackLevel( int value );
	void		SetDelayTime( int value );
	int			GetDelayTime();
	void		SetFIRTap( int tap, int value );
    
    void        SetBrrSample( int srcn, const unsigned char *data, int size, int loopPoint);
    void        DelBrrSample( int srcn );
    void        UpdateLoopPoint( int prog );
    void        UpdateLoopFlag( int prog );
    void        UpdatePortamentoTime( int prog );

	void		SetSampleRate( double samplerate );
	    
	void		Process( unsigned int frames, float *output[2] );
	int			GetKeyMap( int bank, int key ) const { return mKeyMap[bank][key]; }
	const InstParams	*getVP(int pg) const { return &mVPset[pg]; }
	void		SetVPSet( InstParams *vp );
    
    void        SetEventDelayClocks(int clocks);
    double      GetProcessDelayTime();

	void		RefreshKeyMap(void);
    
    C700DSP*    GetDsp() { return &mDSP; }
    
protected:
    virtual int         getKeyOnPriority(int ch, int note);
    virtual int         getReleasePriority(int ch, int note);
    virtual bool        isMonoMode(int ch, int note);
    virtual bool        isPatchLoaded(int ch, int note);

private:
	static const int    INTERNAL_CLOCK = 32000;
    static const int    CYCLES_PER_SAMPLE = 21168;
    static const int    PORTAMENT_CYCLE_SAMPLES = 32;  // �|���^�����g�������s���T���v����(32kHz���Z)
    static const int    PITCH_CYCLE_SAMPLES = 32;  // �s�b�`�ύX���s���T���v����(32kHz���Z)
    static const int    CLOCKS_PER_SAMPLE = 32;
    
    enum RegLogEvtType {
        START_REGLOG = 0,
        MARKLOOP_REGLOG,
        END_REGLOG
    };
    
    typedef struct {
        RegLogEvtType         type;
        int				remain_samples;
    } RegLogEvt;
    
	double              mSampleRate;
	
	int                 mProcessFrac;
	int                 mProcessbuf[2][16];		//���T���v�����O�p�o�b�t�@
	int                 mProcessbufPtr;			//���T���v�����O�p�o�b�t�@�������݈ʒu
	
    MutexObject         mREGLOGEvtMtx;
    std::list<RegLogEvt> mREGLOGEvt;			//���W�X�^���O�̂��߂̃L���[
    int                 mKeyOnFlag; // ����Process��KeyOn����
    int                 mKeyOffFlag; // ����Process��KeyOff����
    int                 mEchoOnFlag; // ����Process��EchoOn����
    int                 mPMOnFlag; // ����Process��PMOn����
    int                 mNoiseOnFlag; // ����Process��NoiseOn����
	
	bool                mDrumMode[NUM_BANKS];
	velocity_mode       mVelocityMode;
    unsigned int        mCCChangeFlg[16];
    InstParams          mChannnelInst[16];
    float               mPortaStartPitch[16];
    float               mChPortaTc[16];
    
    int                 mPortamentCount;        // DSP������1�T���v���o�͂���閈�ɃJ�E���g����A�|���^�����g����������PORTAMENT_CYCLE_SAMPLES ���炷
    int                 mPitchCount[kMaximumVoices];// DSP������1�T���v���o�͂���閈�ɃJ�E���g����A�s�b�`�ύX������PITCH_CYCLE_SAMPLES ���炷�A�m�[�g�I�����ɂ�0�ɂ���

    int                 mEventDelayClocks;      // ����x���N���b�N
	
	int                 mKeyMap[NUM_BANKS][128];	//�e�L�[�ɑΉ�����v���O����No.
	const InstParams    *mVPset;
    
    MemManager          mMemManager;
    C700DSP             mDSP;
    VoiceStatus         mVoiceStat[kMaximumVoices];
    
    int                 mVoiceLimit;
    bool                mIsAccurateMode;
    bool                mFastReleaseAsKeyOff;   // sustainmode��sr=31�̏ꍇ�L�[�I�t�ŏ�������
    
    
    const InstParams	*getMappedVP(int bank, int key) const { return &mVPset[mKeyMap[bank][key]]; }
    InstParams          getChannelVP(int ch, int note);
    
    bool                doRegLogEvents( const RegLogEvt *evt );
    
    int                 calcEventDelaySamples() { return ((mEventDelayClocks / CLOCKS_PER_SAMPLE) * mSampleRate) / INTERNAL_CLOCK; }

    virtual float       getPortamentFreq() { return (static_cast<float>(INTERNAL_CLOCK) / PORTAMENT_CYCLE_SAMPLES); };
    int                 calcPBValue(int ch, int pitchBend, int basePitch);


    // channel messages
    virtual void        handleProgramChange( int ch, int value );
    virtual void        handlePitchBend( int ch, int pitchbend );
    virtual void        handleControlChange( int ch, int controlNum, int value );
    virtual bool        handleNoteOnFirst( unsigned char v, unsigned char midiCh, unsigned char note, unsigned char velo, bool isLegato, int killedMidiCh );
    virtual bool        handleNoteOnDelayed(unsigned char vo, unsigned char midiCh, unsigned char note, unsigned char velo, bool isLegato);
    virtual void        handleNoteOff( const MIDIEvt *evt, int vo );
    
    // control changes
    // change=�v���Z�b�g�p�����[�^�̕ύX
    // set=����ȊO�̈�ʓI�ȃR���g���[���`�F���W
    // CC�ł������̂̓��A���^�C���R���g���[�����K�v�Ȃ��̂����S�ŁA�S�v���Z�b�g�p�����[�^�ł͂Ȃ�
    // �m�[�g�I�����ɐ؂�ւ��Ηǂ����̂�Kernel��VoiceParam�𒼐ڏ���������悤�ɂȂ��Ă���
    virtual void		handleAllNotesOff();
    virtual void		handleAllSoundOff(int ch);
    virtual void		handleResetAllControllers(int ch);
    
    virtual void        handleModWheelChange( int ch, int value );
    virtual void        handleVolumeChange( int ch, int value );
    virtual void        handleExpressionChange( int ch, int value );
    virtual void        handlePanpotChange( int ch, int value );
    virtual void        handlePortamentStartNoteChange( int ch, int note );
    virtual void        handlePortaTimeChange( int ch, int ccValue, float centPerMilis );
    virtual void        handleDataEntryValueChange(int ch, bool isNRPN, int addr, int value);
    virtual void        handleDamperChange( int ch, bool on ) {}
    virtual void        handlePortamentOnChange( int ch, bool on ) {}


    void                changeChPriority( int ch, int value );
    void                changeReleasePriority( int ch, int value );
    void                changeMonoMode( int ch, bool on );
    
//    void              changeChRate(int ch, double rate);
//    void              changeChBasekey(int ch, int basekey);
//    void              changeChLowkey(int ch, int lowkey);
//    void              changeChHighkey(int ch, int highkey);
    
    void                changeChAR(int ch, int ar);
    void                changeChDR(int ch, int dr);
    void                changeChSL(int ch, int sl);
    void                changeChSR1(int ch, int sr);
    void                changeChSR2(int ch, int sr);
    //void              changeChVolL(int ch, int voll);
    //void              changeChVolR(int ch, int volr);
    void                changeChEcho(int ch, int echo);
    void                changeChPMON(int ch, int pmon);
    void                changeChNON(int ch, int non);
    void                changeChBank(int ch, int bank);
    void                changeChSustainMode(int ch, int sustainMode);
};
