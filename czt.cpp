/*
**	czt.c -- Chirp z-Tramsform (CZT)
**	�C�Ӄf�[�^���ł̌v�Z���\�ȍ��� DFT �A���S���Y��
**
**	�w�g�����W�X�^�Z�p�x1993�N2���� P.363�`368 �̉�����Q�l�ɂ��Ă��܂�.
**
**	Public domain by MIYASAKA Masaru <alkaid@coral.ocn.ne.jp> (Sep 15, 2003)
**
**	[TAB4]
*/

#include <stdlib.h>
#include <math.h>

#include "czt.h"

#undef PI
#define PI	3.1415926535897932384626433832795	/* �~����(��) */

/*
**	�d�݃f�[�^�E�C���p���X�����f�[�^�����
*/
static void make_cztdata(int n, int no, int nx, float wr[], float wi[],
                         float vr[], float vi[])
{
	/* �����x���d�v�łȂ��Ȃ� float �ł��������� */
	//float r, d = PI / n;
	//int i, j, n2 = n * 2;

	//for (i = j = 0; i < n; i++, j = (j + (2 * i - 1)) % n2) {
	//	r = d * j;
	//	wr[i] =  cosf(r);					/*  cos(i*i*PI/n) */
	//	wi[i] = -sinf(r);					/* -sin(i*i*PI/n) */
	//}
	//
	double t, dc, ds, co, si;
	int i, j, k;

	t  = sin((PI / n) / 2);
	dc = 2 * t * t;
	ds = sqrt(dc * (2 - dc));
	t  = 2 * dc;
	co = vr[0] = 1;
	si = vi[0] = 0;

	for (i = 1, k = (n + 1) / 2; i < k; i++) {
		co -= dc;  dc += t * co;
		si += ds;  ds -= t * si;
		vr[i] = co;  vr[n - i] = -co;	/* vr[i] = cos(i*PI/n) */
		vi[i] = si;  vi[n - i] =  si;	/* vi[i] = sin(i*PI/n) */
	}
	if (n % 2 == 0) {
		vr[k] = 0;
		vi[k] = 1;
	}

	j = 0;  k = 0;
	for (i = 0; i < n; i++) {
		co = vr[j];
		si = vi[j];
		if (k % 2 != 0) {
			co = -co;
			si = -si;
		}
		wr[i] =  co;				/* wr[i] =  cos(i*i*PI/n) */
		wi[i] = -si;				/* wi[i] = -sin(i*i*PI/n) */
		j += 2 * i + 1;
		k += j / n;  j %= n;
	}
	//

	vr[0] = 1.0;
	vi[0] = 0;
	for (i = 1; i < n; i++) {
		vr[i] = vr[nx - i] =  wr[i];	/* nx * cos(i*i*PI/n) */
		vi[i] = vi[nx - i] = -wi[i];	/* -nx * sin(i*i*PI/n) */
	}
	for (i = no, j = nx - n; i <= j; i++, j--) {
		vr[j] = vr[i] = 0;					/* �c��̊g�����ɂ� 0 �� */
		vi[j] = vi[i] = 0;
	}
}


/*
**	CZT�v�Z�p�\���̂ɑ΂��A�W�{�� n, �o�̓f�[�^�� no �p�̐��\�f�[�^��
**	�쐬����B
**
**	cztp	= CZT�v�Z�p�\���̂ւ̃|�C���^
**	n		= �W�{�_�̐�
**	no		= �o�͂���f�[�^�̐�
**	return	= 0:����I�� 1:n�������Ȑ� 2:�������s��
*/
int czt_init(czt_struct *cztp, int n, int no)
{
	int nx;

	if (n <= 1) return 1;
	if (no <= 1 || n < no) no = n;

	nx = n + no;		/* (n + no)��2�̐�����܂Ŋg������(nx) */
	cztp->m=1;
	while ( (1<<(cztp->m)) < nx ) (cztp->m)++;
	nx = 1<<(cztp->m);
	
	//2^n�̎��͒ʏ��FFT�ŏ�������
	if (nx == n+no) {
		cztp->no_czt = true;
		cztp->m--;
		cztp->w.realp = NULL;
		cztp->w.imagp = NULL;
		cztp->v.realp = NULL;
		cztp->t.realp = NULL;
		
		cztp->fftsetup = vDSP_create_fftsetup(cztp->m, FFT_RADIX2);
		if (cztp->fftsetup == 0) return 2;
	}
	else {
		cztp->no_czt = false;
		
		cztp->samples     = n;
		cztp->samples_out = no;
		cztp->samples_ex  = nx;
		
		cztp->fftsetup = vDSP_create_fftsetup(cztp->m, FFT_RADIX2);
		if (cztp->fftsetup == 0) return 2;
		
		cztp->w.realp = (float*)malloc(n  * sizeof(float));
		cztp->w.imagp = (float*)malloc(n  * sizeof(float));
		cztp->v.realp = (float*)malloc(2 * nx * sizeof(float));
		cztp->t.realp = (float*)malloc(2 * nx * sizeof(float));
		if (cztp->w.realp == NULL || cztp->w.imagp == NULL ||
			cztp->v.realp == NULL || cztp->t.realp == NULL) {
			czt_end(cztp);
			return 2;
		}
		//cztp->w.imagp = cztp->w.realp + n;
		cztp->v.imagp = cztp->v.realp + nx;
		cztp->t.imagp = cztp->t.realp + nx;

		make_cztdata(n, no, nx, cztp->w.realp, cztp->w.imagp, cztp->v.realp, cztp->v.imagp);
		vDSP_fft_zip(cztp->fftsetup, &cztp->v, 1, cztp->m, FFT_FORWARD);
	}
	return 0;
}


/*
**	CZT�v�Z�p�\���̂̐��\�f�[�^���������Ă��̃������̈���J������B
**
**	cztp	= CZT�v�Z�p�\���̂ւ̃|�C���^
*/
void czt_end(czt_struct *cztp)
{
	if (cztp->w.realp != NULL) { free(cztp->w.realp); cztp->w.realp = NULL; }
	if (cztp->w.imagp != NULL) { free(cztp->w.imagp); cztp->w.imagp = NULL; }
	if (cztp->v.realp != NULL) { free(cztp->v.realp); cztp->v.realp = NULL; }
	if (cztp->t.realp != NULL) { free(cztp->t.realp); cztp->t.realp = NULL; }

	cztp->samples     = 0;
	cztp->samples_out = 0;
	cztp->samples_ex  = 0;

	vDSP_destroy_fftsetup(cztp->fftsetup);
}


/*
**	CZT (Chirp z-Tramsform) �A���S���Y���ɂ�鍂�� DFT.
**	re[] ������, im[] ������. ���ʂ� re[],im[] �ɏ㏑�������.
**	inv!=0 (=TRUE) �Ȃ�t�ϊ����s��. cztp �ɂ�, �v�Z�p�f�[�^��
**	�����Ă���\���̂��w�肷��.
*/
void czt(czt_struct *cztp, int inv, DSPSplitComplex *input)
{
	float xr, xi, yr, yi;
	//float	t;
	int i, n, no, nx;
	
	if (cztp->no_czt) {
		vDSP_fft_zip(cztp->fftsetup, input, 1, cztp->m, inv);
		return;
	}
	
	n  = cztp->samples;
	no = cztp->samples_out;
	nx = cztp->samples_ex;
	
	/* ���͂̏d�݃f�[�^��Z */
	for (i = 0; i < n; i++) {
		yr = cztp->w.realp[i];
		yi = cztp->w.imagp[i];
		if (inv == FFT_INVERSE) yi = -yi;
		cztp->t.realp[i] = input->realp[i] * yr - input->imagp[i] * yi;
		cztp->t.imagp[i] = input->imagp[i] * yr + input->realp[i] * yi;
	}
	//vDSP_zvmul(input,1,&cztp->w,1,&cztp->t,1,n,inv);
	/* �c��̊g�����ɂ� 0 �� */
	for (i=n; i < nx; i++) {
		cztp->t.realp[i] = 0;
		cztp->t.imagp[i] = 0;
	}

	vDSP_fft_zip(cztp->fftsetup, &cztp->t, 1, cztp->m, FFT_FORWARD);

	/* �R���{�����[�V�������Z */
	for (i = 0; i < nx; i++) {
		xr = cztp->t.realp[i];
		xi = cztp->t.imagp[i];
		yr = cztp->v.realp[i];
		yi = cztp->v.imagp[i];
		if (inv == FFT_INVERSE) yi = -yi;
		cztp->t.realp[i] = xr * yr - xi * yi;
		cztp->t.imagp[i] = xi * yr + xr * yi;
	}
	//vDSP_zvmul(&cztp->t,1,&cztp->v,1,&cztp->t,1,nx,inv);

	vDSP_fft_zip(cztp->fftsetup, &cztp->t, 1, cztp->m, FFT_INVERSE);
	
	/* �o�͂̏d�݃f�[�^��Z */
	/*
	for (i = 0; i < no; i++) {
		yr = cztp->w.realp[i];
		yi = cztp->w.imagp[i];
		if (inv == FFT_INVERSE) yi = -yi;
		input->realp[i] = cztp->t.realp[i] * yr - cztp->t.imagp[i] * yi;
		input->imagp[i] = cztp->t.imagp[i] * yr + cztp->t.realp[i] * yi;
	}*/
	vDSP_zvmul(&cztp->t,1,&cztp->w,1,input,1,no,inv);

//	if (inv == FFT_FORWARD) {						/* �t�ϊ��łȂ��Ȃ�n�Ŋ��� */
//		t = 1.0 / n;			/* �t����������(���Z�͒x���̂�) */
//		vDSP_vsmul(input->realp,1,&t,input->realp,1,no);
//		vDSP_vsmul(input->imagp,1,&t,input->imagp,1,no);
//	}
}

//��{���g�������߂�
int estimatebasefreq(short *src, int length)
{
	if (length > 0x80000) length = 0x80000;
	
	DSPSplitComplex	temp;
	czt_struct cztd;
	int i, index=1, pitch, error, half=length/2;
	if (half > 530)
		half = 530;
	
	temp.realp = (float*)malloc((sizeof(float)*length));
	temp.imagp = (float*)malloc((sizeof(float)*length));
	
	for (i = 0; i < length; i++) {
		temp.realp[i] = src[i];
		temp.imagp[i] = 0;
	}
	error = czt_init(&cztd, length, length);
	if (error) {
#ifdef DEBUG_PRINT
		printf("error:%d\n",error);
#endif
		return 0;
	}
	czt(&cztd, FFT_FORWARD, &temp);
	
	//�p���[�X�y�N�g����
	vDSP_zvmags(&temp, 1, temp.realp, 1, length);
	for (i = 0; i < length; i++) {
		temp.realp[i] = powf(temp.realp[i], 1.0/3.0);
		temp.imagp[i] = 0;
	}
	czt(&cztd, FFT_INVERSE, &temp);		/* �t�ϊ����Ď��ȑ��ւ����߂� */
	czt_end(&cztd);
	
	//���l���N���b�v������
	for (i = 0; i < half; i++) {
		if (temp.realp[i] < 0.0)
			temp.realp[i] = 0.0;
		temp.imagp[i] = temp.realp[i];
	}
	for (i = 0; i < half; i++) {
		if ((i % 2) == 0)
            temp.realp[i] -= temp.imagp[i/2];
		else
            temp.realp[i] -= ((temp.imagp[i/2] + temp.imagp[i/2 + 1]) / 2);
	}
	//���l���N���b�v������
	/*
	for (i = 0; i < half; i++) {
		if (real[i] < 0.0)
			real[i] = 0.0;
	}
	*/
	//�s�b�`����
	float	max;
	vDSP_maxvi(temp.realp, 1, &max, (vDSP_Length*)&index, half);
	
	if ( index == 0) {
		index = 1;
	}
	pitch = length/index;
	
	free(temp.realp);
	free(temp.imagp);
	
	return pitch;
}

