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
#include "DynamicVoiceManager.h"
#include "C700TimeThread.h"

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
class C700Driver
{
public:
    enum EvtType {
        NOTE_ON = 0,
        NOTE_OFF,
        PROGRAM_CHANGE,
        PITCH_BEND,
        CONTROL_CHANGE,
        START_REGLOG,
        MARKLOOP_REGLOG,
        END_REGLOG
    };
    
    typedef struct {
        EvtType         type;
        unsigned char	ch;
        unsigned char	note;
        unsigned char	velo;
        unsigned int	uniqueID;
        int				remain_samples;
    } MIDIEvt;
    
    typedef struct {
        int         prog;
        float		pitchBend;
        int			vibDepth;
        int         volume;
        int         expression;
        int         pan;
        float       pbRange;
        //bool        portaOn;
        float       portaTc;
        float       portaStartPitch;
        int         lastNote;
        bool        damper;
        
        unsigned int changeFlg;
        InstParams  changedVP;
    } ChannelStatus;
    
    typedef struct VoiceStatus {
		int				pb;
		int				vibdepth;
		bool			reg_pmod;
		float			vibPhase;
        float           portaPitch;
		
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
        
        int				targetPitch;
        
        VoiceStatus() : pb(0), vibdepth(0), reg_pmod(0), vibPhase(0), portaPitch(0),
                        vol_l(0), vol_r(0), velo(0), volume(0), expression(0), pan(0), srcn(0),
                        targetPitch(0) {}
        void Reset();
	} VoiceStatus;
    
	C700Driver();
	virtual				~C700Driver();
	
	virtual void		Reset();

    void                EnqueueMidiEvent(const MIDIEvt *evt);
    virtual void		AllNotesOff();
    virtual void		AllSoundOff();
    virtual void		ResetAllControllers();
    
 
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
    void        SetEventDelayClocks(int clocks);
    double      GetProcessDelayTime();
    int         GetNoteOnNotes(int ch) { return mVoiceManager.GetNoteOns(ch); }
	    
	void		Process( unsigned int frames, float *output[2] );
	int			GetKeyMap( int bank, int key ) const { return mKeyMap[bank][key]; }
	const InstParams	*getVP(int pg) const { return &mVPset[pg]; }
	void		SetVPSet( InstParams *vp );
    
	void		RefreshKeyMap(void);
    
    C700DSP*    GetDsp() { return &mDSP; }
    
private:
	static const int INTERNAL_CLOCK = 32000;
    static const int CYCLES_PER_SAMPLE = 21168;
    static const int PORTAMENT_CYCLE_SAMPLES = 32;  // �|���^�����g�������s���T���v����(32kHz���Z)
    static const int PITCH_CYCLE_SAMPLES = 32;  // �s�b�`�ύX���s���T���v����(32kHz���Z)
    static const int CLOCKS_PER_SAMPLE = 32;
    
    static const int VOLUME_DEFAULT = 100;
    static const int EXPRESSION_DEFAULT = 127;
    static const int DEFAULT_PBRANGE = 2;
	
	double			mSampleRate;
	
	int				mProcessFrac;
	int				mProcessbuf[2][16];		//���T���v�����O�p�o�b�t�@
	int				mProcessbufPtr;			//���T���v�����O�p�o�b�t�@�������݈ʒu
	
    MutexObject         mREGLOGEvtMtx;
    std::list<MIDIEvt>	mREGLOGEvt;			//���W�X�^���O�̂��߂̃L���[
    MutexObject         mMIDIEvtMtx;
	std::list<MIDIEvt>	mMIDIEvt;			//�󂯎�����C�x���g�̃L���[
	std::list<MIDIEvt>	mDelayedEvt;		//�x�����s�C�x���g�̃L���[
    bool                mKeyOnFlag[kMaximumVoices]; // ����Process��KeyOn����
    bool                mKeyOffFlag[kMaximumVoices]; // ����Process��KeyOff����
    int                 mEchoOnFlag; // ����Process��EchoOn����
    bool                mEchoOnMask[kMaximumVoices]; // ����Process��Echo��ύX����
    int                 mPMOnFlag; // ����Process��PMOn����
    bool                mPMOnMask[kMaximumVoices]; // ����Process��PMON��ύX����
    int                 mNoiseOnFlag; // ����Process��NoiseOn����
    bool                mNoiseOnMask[kMaximumVoices]; // ����Process��NON��ύX����
	
    DynamicVoiceManager mVoiceManager;
	    
	bool			mDrumMode[NUM_BANKS];
	velocity_mode	mVelocityMode;
    ChannelStatus   mChStat[16];
    float			mVibfreq;
	float			mVibdepth;

    int             mPortamentCount;        // DSP������1�T���v���o�͂���閈�ɃJ�E���g����A�|���^�����g����������PORTAMENT_CYCLE_SAMPLES ���炷
    int             mPitchCount[kMaximumVoices];// DSP������1�T���v���o�͂���閈�ɃJ�E���g����A�s�b�`�ύX������PITCH_CYCLE_SAMPLES ���炷�A�m�[�g�I�����ɂ�0�ɂ���

    int             mEventDelaySamples;     // ����x���T���v��(�����T���v�����O���[�g)
    int             mEventDelayClocks;      // ����x���N���b�N
	
	int				mKeyMap[NUM_BANKS][128];	//�e�L�[�ɑΉ�����v���O����No.
	const InstParams *mVPset;
    
    MemManager      mMemManager;
    C700DSP         mDSP;
    VoiceStatus		mVoiceStat[kMaximumVoices];
    
    int             mVoiceLimit;
    bool            mIsAccurateMode;
    bool            mFastReleaseAsKeyOff;   // sustainmode��sr=31�̏ꍇ�L�[�I�t�ŏ�������
    
    // RPN, NRPN
    int             mDataEntryValue[16];
    int             mIsSettingNRPN[16];
    int             mRPN[16];
    int             mNRPN[16];
    
    const InstParams	*getMappedVP(int bank, int key) const { return &mVPset[mKeyMap[bank][key]]; }

    InstParams      getChannelVP(int ch, int note);
    void            processPortament(int vo);
    void            calcPanVolume(int value, int *volL, int *volR);
    virtual void    doProgramChange( int ch, int value );
	virtual void    doPitchBend( int ch, int value1, int value2 );
    virtual void    doControlChange( int ch, int controlNum, int value );
	float           vibratoWave(float phase);
	int             calcPBValue(int ch, float pitchBend, int basePitch);
    virtual bool    doNoteOnFirst( MIDIEvt dEvt );
	virtual void    doNoteOnDelayed(const MIDIEvt *evt);
	virtual int     doNoteOff( const MIDIEvt *evt );
    bool            doRegLogEvents( const MIDIEvt *evt );
    bool            doFirstEvents( const MIDIEvt *evt );
    bool            doDelayedEvents( const MIDIEvt *evt );
    int             calcEventDelaySamples() { return ((mEventDelayClocks / CLOCKS_PER_SAMPLE) * mSampleRate) / INTERNAL_CLOCK; }
    float           calcGM2PortamentCurve(int value);
    
    // control changes
    // change=�v���Z�b�g�p�����[�^�̕ύX
    // set=����ȊO�̈�ʓI�ȃR���g���[���`�F���W
    void            setModWheel( int ch, int value );
    void            setDamper( int ch, bool on );
    void            setVolume( int ch, int value );
    void            setExpression( int ch, int value );
    void            setPanpot( int ch, int value );
    
    void            changePortamentOn( int ch, bool on );
    void            setPortamentTime( int ch, int value );
    void            setPortamentControl( int ch, int note );

    void            changeChPriority( int ch, int value );
    void            changeChLimit( int ch, int value );
    void            changeReleasePriority( int ch, int value );
    void            changeMonoMode( int ch, bool on );
    
//    void        changeChRate(int ch, double rate);
//    void        changeChBasekey(int ch, int basekey);
//    void        changeChLowkey(int ch, int lowkey);
//    void        changeChHighkey(int ch, int highkey);
    
    void            changeChAR(int ch, int ar);
    void            changeChDR(int ch, int dr);
    void            changeChSL(int ch, int sl);
    void            changeChSR1(int ch, int sr);
    void            changeChSR2(int ch, int sr);
    //void          changeChVolL(int ch, int voll);
    //void          changeChVolR(int ch, int volr);
    void            changeChEcho(int ch, int echo);
    void            changeChPMON(int ch, int pmon);
    void            changeChNON(int ch, int non);
    void            changeChBank(int ch, int bank);
    void            changeChSustainMode(int ch, int sustainMode);

    void            setRPNLSB(int ch, int value);
    void            setRPNMSB(int ch, int value);
    void            setNRPNLSB(int ch, int value);
    void            setNRPNMSB(int ch, int value);
    void            setDataEntryLSB(int ch, int value);
    void            setDataEntryMSB(int ch, int value);
    void            sendDataEntryValue(int ch);
    void            setPBRange( int ch, float value );
};
