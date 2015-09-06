#ifndef __HIKSDKPARSER2_H_
#define __HIKSDKPARSER2_H_
#include "j_includes.h"
#include "x_time.h"

#define DATA_BUFFER_SIZE (1024 * 1024)
#define HIK_PACK_LENGHT(x) (((*(x + 3) & 0xFF) << 8) + (*(x + 2) & 0xFF))
#define HIK_PACK_LENGHT2(x) (((*(x + 2) & 0xFF) << 8) + (*(x + 3) & 0xFF) + 4)

class CHikSdkParser2
{
public:
	CHikSdkParser2();
	~CHikSdkParser2();

public:
	///J_VideoParser
	int Init(int nDataType = jo_video_normal);
	int Deinit();
	int InputData(const char *pData, int nLen);
	int GetOnePacket(char *pData, J_StreamHeader &streamHeader);

private:
	int GetDataFlag();

private:
	char *m_pDataBuff;
	int m_nDataSize;

	char *m_pOutBuff;
	j_uint32_t m_nDataLen;
	j_uint32_t m_nFrameLen;
	j_uint32_t m_frameNum;
	j_boolean_t m_bIsComplate;;
};

#endif //~__HIKSDKPARSER2_H_