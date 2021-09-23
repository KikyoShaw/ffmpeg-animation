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

	//����fomat������
	AVFormatContext* formatContext = avformat_alloc_context();                                    
	//��������
	if (avformat_open_input(&formatContext, audioPath.toLocal8Bit().data(), nullptr, nullptr) != 0) {  
		qDebug() << "can`t open the file.";
		return -1;
	}
	//���ն˴�ӡ
	av_dump_format(formatContext, 0, audioPath.toLocal8Bit(), 0);                                  
	//�����������е���Ϣ
	if (avformat_find_stream_info(formatContext, nullptr) != 0) {                                   
		qDebug() << "can`t find stream infomation";
		return -1;
	}
	//���ҵ�һ����Ƶ����һ����Ƶ�п����ж��������Ƶ+��Ƶ����
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
	//��ȡ��������Ĳ�����
	AVCodecParameters *codecParam = formatContext->streams[videoStreamIndex]->codecpar;  
	//��ȡ�������
	AVCodec* codec = avcodec_find_decoder(codecParam->codec_id);  
	//��ȡ�����������
	AVCodecContext* codecContext = avcodec_alloc_context3(nullptr);
	//���ݱ�����������������������
	avcodec_parameters_to_context(codecContext, codecParam);                                    
	if (codec == nullptr) {
		qDebug() << "can`t find codec.";
		return -1;
	}
	//�����������
	if (avcodec_open2(codecContext, codec, nullptr) != 0) {                                         
		qDebug() << "can`t open codec";
		return -1;
	}
	//����һ�����ݰ�
	AVPacket *packet = av_packet_alloc();                                                      
	//����һ����Ƶ֡
	AVFrame* frame = av_frame_alloc();                                                            
	//����һ��QImage�������
	QImage output(codecParam->width, codecParam->height, QImage::Format_RGB888);    
	//����AVFrame��QImage����Ҫ������
	int outputLineSize[4];                                                                         
	av_image_fill_linesizes(outputLineSize, AV_PIX_FMT_RGB24, codecParam->width);
	uint8_t *outputDst[] = { output.bits() };
	//����һ����ʽת��������
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