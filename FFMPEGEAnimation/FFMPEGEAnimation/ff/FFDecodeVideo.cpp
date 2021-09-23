#include "FFDecodeVideo.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avio.h"
#include "libavutil/imgutils.h"
}

#include <QDebug>
#include <QImage>

int videoToImage(const QString &audioPath, const QString&outputDir) {

	//分配fomat上下文
	AVFormatContext* formatContext = avformat_alloc_context();                                    
	//打开输入流
	if (avformat_open_input(&formatContext, audioPath.toLocal8Bit().data(), nullptr, nullptr) != 0) {  
		qDebug() << "can`t open the file.";
		return -1;
	}
	//在终端打印
	av_dump_format(formatContext, 0, audioPath.toLocal8Bit(), 0);                                  
	//加载输入流中的信息
	if (avformat_find_stream_info(formatContext, nullptr) != 0) {                                   
		qDebug() << "can`t find stream infomation";
		return -1;
	}
	//查找第一个视频流（一个视频中可能有多个流（音频+视频））
	int videoStreamIndex = -1;
	for (uint i = 0; i < formatContext->nb_streams; i++) {
		if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {                
			videoStreamIndex = i;
			break;
		}
	}
	if (videoStreamIndex == -1) {
		qDebug() << "didn`t find a video stream";
		return -1;
	}
	//获取编解码器的参数集
	AVCodecParameters *codecParam = formatContext->streams[videoStreamIndex]->codecpar;  
	//获取编解码器
	AVCodec* codec = avcodec_find_decoder(codecParam->codec_id);  
	//获取编解码上下文
	AVCodecContext* codecContext = avcodec_alloc_context3(nullptr);
	//根据编解码器参数填充编解码上下文
	avcodec_parameters_to_context(codecContext, codecParam);                                    
	if (codec == nullptr) {
		qDebug() << "can`t find codec.";
		return -1;
	}
	//开启编解码器
	if (avcodec_open2(codecContext, codec, nullptr) != 0) {                                         
		qDebug() << "can`t open codec";
		return -1;
	}
	//分配一个数据包
	AVPacket *packet = av_packet_alloc();                                                      
	//分配一个视频帧
	AVFrame* frame = av_frame_alloc();                                                            
	//构造一个QImage用作输出
	QImage output(codecParam->width, codecParam->height, QImage::Format_RGB888);    
	//构造AVFrame到QImage所需要的数据
	int outputLineSize[4];                                                                         
	av_image_fill_linesizes(outputLineSize, AV_PIX_FMT_RGB24, codecParam->width);
	uint8_t *outputDst[] = { output.bits() };
	//构造一个格式转换上下文
	SwsContext *imgConvertContext = sws_getContext(codecParam->width, codecParam->height, (AVPixelFormat)codecParam->format, codecParam->width, codecParam->height, 
		AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	int index = 0;
	while (true) {
		if (av_read_frame(formatContext, packet) < 0)
			break;
		if (packet->stream_index == videoStreamIndex) {
			if (avcodec_send_packet(codecContext, packet) != 0)
				continue;
			if (avcodec_receive_frame(codecContext, frame) != 0)
				continue;
			sws_scale(imgConvertContext, frame->data, frame->linesize, 0, codecParam->height, outputDst, outputLineSize);
			output.save(outputDir + QString::number(index++) + ".jpg");
		}
	}
	av_frame_free(&frame);
	av_packet_free(&packet);
	avcodec_free_context(&codecContext);
	avformat_close_input(&formatContext);
	return 0;
}