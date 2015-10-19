#ifndef __XL_PROTOCOL_H__
#define __XL_PROTOCOL_H__
#include "windows.h"

#pragma pack(push)
#pragma pack(1)
struct CXlProtocol
{
	enum FrameType
	{
		xl_frame_message = 0,
        xl_frame_request = 1,
        xl_framer_response = 2
	};
	/// 媒体数据类型
	enum ControlType
	{
		xl_ctrl_data = 0,
        xl_ctrl_start = 1,
        xl_ctrl_stream = 2,
        xl_ctrl_end = 3,
        xl_ctrl_stop = 4
	};

	/// 命令状态
	enum CmdState
	{
		xl_init_state = 0,
		xl_read_head_state,
		xl_read_data_state,
		xl_write_body_state,
		xl_error_state,
	};

	/// 标识码定义
	enum FlagType
	{
		xl_pack_start = 0x00,
		xl_pack_continue = 0x01,
		xl_pack_end = 0x02,
	};


	/// 消息类型
	enum MessageType
	{
		xld_copyed_file = 0x01,				///< 已拷贝文件
		xld_readed_file = 0x02,				///< 已读消息
		xld_camera_status = 0x03,			///< 摄像头状态

		xlc_msg_force_offline = 0x01,		///< 用户被强制下线
		xlc_msg_user_deleted = 0x02,		///< 用户被删除
		xlc_copyed_file = 0x03,				///< 已拷贝文件
		xlc_readed_file = 0x04,				///< 已读消息
		xlc_camera_status = 0x05,			///< 摄像头状态
		xlc_dev_info_changed = 0x06,		///< 设备信息发生变化

		xlc_dev_state = 0xA0,				///< 设备状态
	};

	/// 对讲状态
	enum AudioState
    {
        xla_busy,
        xla_acccept,
        xla_refuse,
        xla_end,
        xla_request,
        xla_offline,
        xla_timeout
	};

	/// 操作指令定义  
	enum CmdType
	{
		//异步消息及异常推送
		xld_message = 0x00,					///< 异步消息及异常推送
		//基础管理
		xld_register = 0x01,				///< 主机注册
		xld_conrrent_time = 0x02,			///< 车载设备校时
		xld_heartbeat = 0x03,				///< 心跳检测
		//设备信息
		xld_get_dvr_info = 0x04,			///< 设备信息获取
		xld_set_dvr_info = 0x05,			///< 配置设备信息
		xld_server_ready = 0x06,			///< 配置设备信息
		//实时视频
		xld_start_real_play = 0x10,			///< 开始实时视频播放
		xld_stop_real_play = 0x10,			///< 停止实时视频播放
		xld_start_real_view = 0x12,			///< 开始实时视频预览
		xld_stop_real_view = 0x12,			///< 停止实时视频预览
		//远程回放&视频下载
		xld_start_vod_play = 0x14,			///< 开始录像回放
		xld_stop_vod_play = 0x14,			///< 停止录像回放
		xld_start_vod_download = 0x15,		///< 开始录像下载
		xld_stop_vod_download = 0x16,		///< 停止录像下载
		//运行数据记录
		xld_alarm_info = 0x21,				///< 实时报警推送
		xld_get_on_off_log = 0x22,			///< 开关机日志
		xld_first_vod_timestamp = 0x23,		///< 推送/获取第一个视频文件的开始时间
		xld_update_vod_info = 0x24,			///< 更新录像信息
		//联络数据传输
		xld_upload_file = 0x30,				///< 文件传输
		xld_trans_context = 0x31,			///< 文本消息通讯
		//对讲命令
		xld_talk_data_in = 0x40,			///< 对讲数据
		xld_talk_data_out = 0x41,			///< 对讲数据
		xld_talk_cmd_out = 0x42,			///< 对讲请求
		xld_talk_cmd_in = 0x43,				///< 发起对讲
		//客户端之间通信

		//异步消息及异常推送
		xlc_message = 0x50,					///< 异步消息及异常推送
		//客户端在线心跳
		xlc_heart_beat = 0x51,				///< 客户端在线心跳
		//用户验证
		xlc_login = 0x52,					///< 用户登录
		xlc_logout = 0x53,					///< 注销登录
		//实时报警
		xlc_real_alarm = 0x60,				///< 实时报警信息使能
		//实时视频
		xlc_real_play = 0x70,				///< 实时视频播放
		//远程回放
		xlc_vod_play = 0x71,				///< 录像回放
		xlc_vod_download = 0x72,			///< 录像下载
		//联络数据传输
		xlc_trans_context = 0x80,			///< 发送文本消息
		xlc_upload_file = 0x81,				///< 文件传输
		//对讲命令
		xlc_talk_data_out = 0x90,				///< 对讲数据
		xlc_talk_data_in = 0x91,				///< 对讲数据
		xlc_talk_cmd_in = 0x92,				///< 对讲请求
		xlc_talk_cmd_out = 0x93,				///< 发起对讲
		//客户端之间通信
	};

	struct CmdHeader
	{
		char beginCode;			///< 开始标志-0xFF
		char version;			///< 版本号
		int seq;				///< 序列号
		unsigned char cmd;		///< 控制码,见CmdType定义
		char flag;				///< 状态码 0x0表示开始, 0x1传输中, 0x2传输完成
		int length;				///< 数据长度,不包括头数据和尾数据				
	};

	struct CmdTail
	{
		char verify;			///< 校验码
		char endCode;			///< 结束标志-0xFE
	};
};
#pragma pack(pop)

#endif //!__XL_PROTOCOL_H__