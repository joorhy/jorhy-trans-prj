#ifndef __AUDIO_CONVERT_H_
#define __AUDIO_CONVERT_H_
#include "j_includes.h"
#include "faac.h"
#include "hi_voice_api.h"

class CAudioConvert
{
public:
	CAudioConvert();
	~CAudioConvert();

public:
	j_result_t Init();
	void Deinit();
	j_result_t Convert(const j_char_t *pInputData, j_int32_t nInputLen, j_char_t *pOutputData, j_int32_t &nOutputLen);

private:
	//“Ù∆µΩ‚¬Î
	hiVOICE_G726_STATE m_g726State;

	//“Ù∆µ±‡¬Î
	unsigned int m_nSamples;
	short int *m_pAudioCache;
	faacEncHandle m_aacHandle;
	unsigned long m_nInputSamples;
	unsigned long m_nOutputBytes;
	j_uint32_t m_frameNum;
};

#endif //~__AUDIO_CONVERT_H_