#ifndef __RTSPDEF_H__
#define __RTSPDEF_H__

static const char *rtsp_end = "\r\n\r\n";

static unsigned const samplingFrequencyTable[16] = {
	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
	16000, 12000, 11025, 8000, 7350, 0, 0, 0
};

#define VIDEO_TRACK_ID 	0
#define AUDIO_TRACK_ID	1

static const char *ret_options =
"RTSP/1.0 200 OK\r\n"
"Server: Forcetv (Build/489.16; Platform/Win32; Release/Forcetv; state/beta; )\r\n"
"Cseq: %d\r\n"
"Public: OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN\r\n\r\n";

static const char *ret_redrect =
"RTSP/1.0 301 Moved\r\n"
"CSeq: %d\r\n"
"Location: rtsp://222.214.218.237:6601/LDMsMTI5OTg4MCwyLDEsMCww\r\n\r\n";

static const char *ret_describ =
"RTSP/1.0 200 OK\r\n"
"Server: Forcetv (Build/489.16; Platform/Win32; Release/Forcetv; state/beta; )\r\n"
"Cseq: %d\r\n"
"Cache-Control: must-revalidate\r\n"
"Session: %X\r\n"
"Content-length: %d\r\n"
"Content-Type: application/sdp\r\n"
"x-Accept-Retransmit: our-retransmit\r\n"
"x-Accept-Dynamic-Rate: 1\r\n"
"Content-Base: %s/\r\n\r\n";

static const char *live_sdp =
"v=0\r\n"
"o=- 0 0 IN IP4 %s\r\n"
"c=IN IP4 0.0.0.0\r\n"
"a=control:*\r\n"
"s=Force-live\r\n"
"t=0 0\r\n"
"a=range:npt=0-\r\n"
"m=video 0 RTP/AVP 96\r\n"
"a=control:trackID=0\r\n"
"a=rtpmap:96 H264/90000\r\n"
"a=fmtp:96 profile-level-id=%x%x%x; sprop-parameter-sets=%s,%s; packetization-mode=1\r\n"
"m=audio 0 RTP/AVP 97\r\n"
"b=AS:96\r\n"
"a=rtpmap:97 MPEG4-GENERIC/8000\r\n"
"a=fmtp:97 streamtype=5;profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config=%s\r\n"
"a=control:trackID=1\r\n";

static const char *ret_setup_tcp =
"RTSP/1.0 200 OK\r\n"
"Server: Forcetv (Build/489.16; Platform/Win32; Release/Forcetv; state/beta; )\r\n"
"Cseq: %d\r\n"
"Cache-Control: must-revalidate\r\n"
"Session: %X;timeout=65\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=%s;ssrc=%s;mode=PLAY\r\n\r\n";

static const char *ret_setup_udp =
"RTSP/1.0 200 OK\r\n"
"Server: Forcetv (Build/489.16; Platform/Win32; Release/Forcetv; state/beta; )\r\n"
"CSeq: %d\r\n"
"Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
"Session: %X\r\n\r\n";

static const char *ret_play =
"RTSP/1.0 200 OK\r\n"
"Server: Forcetv (Build/489.16; Platform/Win32; Release/Forcetv; state/beta; )\r\n"
"Cseq: %d\r\n"
"Session: %X\r\n"
"Range: npt=0-\r\n"
//"RTP-Info: url=%strackID=0;seq=0;rtptime=0\r\n\r\n";
"RTP-Info: url=%s/trackID=0;seq=0;rtptime=0;url=%s/trackID=1;seq=0;rtptime=0\r\n\r\n";

static const char *ret_pause =
"RTSP/1.0 200 OK\r\n"
"Server: Forcetv (Build/489.16; Platform/Win32; Release/Forcetv; state/beta; )\r\n"
"Cseq: %d\r\n"
"Session: %X\r\n\r\n";

static const char *ret_teardown =
"RTSP/1.0 200 OK\r\n"
"Server: Forcetv (Build/489.16; Platform/Win32; Release/Forcetv; state/beta; )\r\n"
"Cseq: %d\r\n"
"Session: %X\r\n"
"Connection: Close\r\n\r\n";

static const char *ret_setup_unsport =
"RTSP/1.0 461 Unsupported Transport\r\n\r\n";

#endif //~__RTSPDEF_H__