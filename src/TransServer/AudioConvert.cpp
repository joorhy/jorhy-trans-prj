#include "AudioConvert.h"

CAudioConvert::CAudioConvert()
{
	m_pAudioCache = NULL;
	m_nSamples = 0;

	m_aacHandle = NULL;
	m_nInputSamples = 0;
	m_nOutputBytes = 0;
	m_frameNum = 0;
}

CAudioConvert::~CAudioConvert()
{

}

j_result_t CAudioConvert::Init()
{
	if (m_pAudioCache == NULL)
	{
		m_pAudioCache = new short int[1024 * 10];
	}

	//初始化解码库
	HI_VOICE_DecReset(&m_g726State, MEDIA_G726_40KBPS);

	//初始化AAC编码库
	m_aacHandle = faacEncOpen(8000, 1, &m_nInputSamples, &m_nOutputBytes);
	faacEncConfigurationPtr pAacConfig = faacEncGetCurrentConfiguration(m_aacHandle);
	pAacConfig->aacObjectType = MAIN;
	pAacConfig->mpegVersion = MPEG4;
	pAacConfig->useTns = 0;
	pAacConfig->allowMidside = 1;
	pAacConfig->outputFormat = 1;
	//pAacConfig->bitRate = 8 * 1024;
	pAacConfig->bandWidth = 0;
	pAacConfig->inputFormat = FAAC_INPUT_16BIT;
	faacEncSetConfiguration(m_aacHandle, pAacConfig);

	return J_OK;
}

void CAudioConvert::Deinit()
{
	if (m_aacHandle != NULL)
	{
		faacEncClose(m_aacHandle);
		m_aacHandle = NULL;
	}

	if (m_pAudioCache != NULL)
	{
		delete m_pAudioCache;
		m_pAudioCache = NULL;
	}
}

j_result_t CAudioConvert::Convert(const j_char_t *pInputData, j_int32_t nInputLen, j_char_t *pOutputData, j_int32_t &nOutputLen)
{
	short nPCMLen = 0;
	if (HI_VOICE_DecodeFrame(&m_g726State, (short *)pInputData, m_pAudioCache + m_nSamples, &nPCMLen) == HI_SUCCESS)
	{
		m_nSamples += nPCMLen;
		if (m_nSamples >= m_nInputSamples)
		{
			nOutputLen = 0;
			nOutputLen = faacEncEncode(m_aacHandle, (int32_t *)m_pAudioCache, m_nInputSamples, (unsigned char *)pOutputData, m_nOutputBytes);
			if (nOutputLen > 0)
			{
				//static FILE *fd = NULL;
				//if (fd == NULL)
				//	fd = fopen("test.aac", "wb+");
				//fwrite(pOutputData, 1, nOutputLen, fd);

				m_nSamples -= m_nInputSamples;
				memmove(m_pAudioCache, m_pAudioCache + m_nInputSamples, m_nInputSamples * 2);

				return J_OK;
			}
		}
	}

	return J_NOT_COMPLATE;
}