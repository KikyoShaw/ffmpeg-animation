/*
* Copyright (c) 2001 Fabrice Bellard
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

/**
* @file
* video decoding with libavcodec API example
*
* @example decode_video.c
*/

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>


extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

struct FrameInfoVo
{
	int frameIndex;

	int x;
	int y;
	int width;
	int height;
};

struct MovieInfoVo {
	int fps_num;
	int fps_den;

	int64_t duration;

	int width = 0;
	int height = 0;

	std::vector<FrameInfoVo*> frames;

	~MovieInfoVo() {
		for (std::vector<FrameInfoVo*>::iterator it = frames.begin(); it != frames.end(); ++it) {
			delete *it;
			*it = nullptr;
		}
		frames.clear();
	}
};


//void SaveBmp(AVCodecContext *CodecContex, AVFrame *Picture, int width, int height, int num)
//{
//	AVPicture pPictureRGB;//RGB图片
//
//	static struct SwsContext *img_convert_ctx;
//	img_convert_ctx = sws_getContext(width, height, CodecContex->pix_fmt, width, height, \
//		PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
//	// 确认所需缓冲区大小并且分配缓冲区空间
//	avpicture_alloc(&pPictureRGB, PIX_FMT_RGB24, width, height);
//	sws_scale(img_convert_ctx, Picture->data, Picture->linesize, \
//		0, height, pPictureRGB.data, pPictureRGB.linesize);
//
//	int lineBytes = pPictureRGB.linesize[0], i = 0;
//
//	char fileName[1024] = { 0 };
//	char * bmpSavePath = "%d.bmp";
//	//time_t ltime;
//	//time(<ime);
//	//sprintf(fileName,bmpSavePath , ltime);???????????????????????????
//	sprintf(fileName, bmpSavePath, num);
//
//	FILE *pDestFile = fopen(fileName, "wb");
//	BITMAPFILEHEADER btfileHeader;
//	btfileHeader.bfType = MAKEWORD(66, 77);
//	btfileHeader.bfSize = lineBytes*height;
//	btfileHeader.bfReserved1 = 0;
//	btfileHeader.bfReserved2 = 0;
//	btfileHeader.bfOffBits = 54;
//
//	BITMAPINFOHEADER bitmapinfoheader;
//	bitmapinfoheader.biSize = 40;
//	bitmapinfoheader.biWidth = width;
//	bitmapinfoheader.biHeight = height;
//	bitmapinfoheader.biPlanes = 1;
//	bitmapinfoheader.biBitCount = 24;
//	bitmapinfoheader.biCompression = BI_RGB;
//	bitmapinfoheader.biSizeImage = lineBytes*height;
//	bitmapinfoheader.biXPelsPerMeter = 0;
//	bitmapinfoheader.biYPelsPerMeter = 0;
//	bitmapinfoheader.biClrUsed = 0;
//	bitmapinfoheader.biClrImportant = 0;
//
//	fwrite(&btfileHeader, 14, 1, pDestFile);
//	fwrite(&bitmapinfoheader, 40, 1, pDestFile);
//	for (i = height - 1; i >= 0; i--)
//	{
//		fwrite(pPictureRGB.data[0] + i*lineBytes, lineBytes, 1, pDestFile);
//	}
//
//	fclose(pDestFile);
//	avpicture_free(&pPictureRGB);
//}

struct Rect {
	int top;
	int bottom;
	int left;
	int right;
};

void MinimizeARGB(
	const int lineBytes, const int byteCountPerPix, uint8_t* buf, int dst_width, int height,
	Rect& rect
) {
	const int mid_y = height / 2 + 1;
	const int mid_x = dst_width / 2 + 1;


	rect.top = mid_y + 1;
	rect.bottom = mid_y - 1;
	rect.left = mid_x + 1;
	rect.right = mid_x - 1;


	//算 top bottom，水平扫描
	for (int i = 0; i < dst_width; ++i) {
		const int x = i * byteCountPerPix;

		bool bTopFind = false;
		bool bBottomFind = false;
		for (int j = 0; j < mid_y; ++j) {

			if (!bTopFind) {
				int u = j * lineBytes + x;
				if (buf[u] != 0) {
					bTopFind = true;
					//不是全透
					if (rect.top > j) {
						rect.top = j;
					}
				}
			}
			
			if (!bBottomFind) {
				int t_j = height - 1 - j;
				int u = t_j * lineBytes + x;
				if (buf[u] != 0) {
					bBottomFind = true;
					//不是全透
					if (rect.bottom < t_j) {
						rect.bottom = t_j;
					}
				}
			}

			if (bTopFind && bBottomFind) {
				break;
			}
			
		}
	}

	//判定是否全透
	if (rect.bottom <= rect.top) {
		rect.left = 0;
		rect.right = 0;
		rect.top = 0;
		rect.bottom = 0;
		return;
	}

	//算 left right,垂直扫描
	for (int i = 0; i < height; ++i) {

		bool bLeftFind = false;
		bool bRightFind = false;

		for (int j = 0; j < mid_x; ++j) {
			if (!bLeftFind) {
				int u = i * lineBytes + j * byteCountPerPix;
				if (buf[u] != 0) {
					bLeftFind = true;
					//不是全透
					if (rect.left > j) {
						rect.left = j;
					}
				}
			}

			if (!bRightFind) {
				int t_j = (dst_width - 1 - j);
				int u = i * lineBytes + t_j * byteCountPerPix;
				if (buf[u] != 0) {
					bRightFind = true;
					//不是全透
					if (rect.right < t_j) {
						rect.right = t_j;
					}
				}
			}

			if (bLeftFind && bRightFind) {
				break;
			}
		}
	}


}

bool SaveARGB(AVCodecContext *ctx_codec, SwsContext*& ctx_sws_rgb, AVFrame *pFrame, const char* fileName, FrameInfoVo* frameInfo)
{
	//uint8_t* dst_pointers[4] = { 0 };
	//int dst_linesizes[4] = { 0 };

	//const int width = pFrame->width;
	//const int height = pFrame->height;

	////ctx_sws_rgb = sws_getContext(width, height, ctx_codec->pix_fmt, width, height,
	////	AV_PIX_FMT_RGB24, SWS_BICUBIC | SWS_PRINT_INFO, NULL, NULL, NULL);
	////SwsContext *ctx_sws_rgb = sws_getContext(width, height, AVPixelFormat(pFrame->format), width, height,
	////	AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
	//
	//// 确认所需缓冲区大小并且分配缓冲区空间
	//av_image_alloc(dst_pointers, dst_linesizes, width, height, AV_PIX_FMT_RGB24, 1);
	////avpicture_alloc(&pPictureRGB, AV_PIX_FMT_RGB24, width, height);

	//sws_scale(ctx_sws_rgb, pFrame->data, pFrame->linesize,
	//	0, height, dst_pointers, dst_linesizes);

	const int width = pFrame->width;
	const int height = pFrame->height;

	//打开ffmpeg格式转换和缩放器
	ctx_sws_rgb = sws_getCachedContext(ctx_sws_rgb, width, height, ctx_codec->pix_fmt,
		width, height, AV_PIX_FMT_RGB24, SWS_BILINEAR,
		NULL, NULL, NULL);

	AVFrame* rgbFrame = av_frame_alloc();
	rgbFrame->height = pFrame->height;
	rgbFrame->width = pFrame->width;
	rgbFrame->format = AV_PIX_FMT_RGB24;
	av_frame_get_buffer(rgbFrame, 1);
	uint8_t** dst_pointers = rgbFrame->data;
	int* dst_linesizes = rgbFrame->linesize;
	sws_scale(ctx_sws_rgb, pFrame->data, pFrame->linesize,
		0, height, dst_pointers, dst_linesizes);



	const int lineBytes = dst_linesizes[0];
	const int byteCountPerPix = lineBytes / width;

	const int dst_width = width / 2;

	uint8_t* buf = dst_pointers[0];

	
	Rect rect;
	MinimizeARGB(lineBytes, byteCountPerPix, buf, dst_width, height, rect);
	if (rect.bottom <= rect.top) {
		//全透明
		frameInfo->x = 0;
		frameInfo->y = 0;
		frameInfo->width = 0;
		frameInfo->height = 0;
		return true;
	}

	frameInfo->x = rect.left;
	frameInfo->y = rect.top;
	frameInfo->width = rect.right - rect.left + 1;
	frameInfo->height = rect.bottom - rect.top + 1;
	
	const int saveSize = frameInfo->width * frameInfo->height * 4;
	uint8_t* saveARGBBuf = new uint8_t[saveSize];

	int k = 0;
	for (int i = rect.top; i <= rect.bottom; ++i)
	{
		uint8_t* alphaChannel = buf + i * lineBytes;
		uint8_t* rgbChannel = alphaChannel + dst_width * byteCountPerPix;

		for (int j = rect.left; j <= rect.right; ++j) {
			int u = j * byteCountPerPix;
			saveARGBBuf[k++] = rgbChannel[u];		//r
			saveARGBBuf[k++] = rgbChannel[u + 1];	//g
			saveARGBBuf[k++] = rgbChannel[u + 2];	//b
			saveARGBBuf[k++] = alphaChannel[u];		//alpha
		}
	}

	FILE* f;
	fopen_s(&f, fileName, "wb");
	fwrite(saveARGBBuf, sizeof(uint8_t), saveSize, f);
	fclose(f);


	delete[] saveARGBBuf;

	//av_freep(&dst_pointers[0]);
	av_frame_free(&rgbFrame);

	//avpicture_free(&pPictureRGB);

	return true;
}

void saveMovieInfoVo(const MovieInfoVo& vo, const char* fileName) {
	std::string str = "{";

	str += "\n\"fps_num\":" + std::to_string(vo.fps_num) + ",";
	str += "\n\"fps_den\":" + std::to_string(vo.fps_den) + ",";
	str += "\n\"duration\":" + std::to_string(vo.duration) + ",";
	str += "\n\"width\":" + std::to_string(vo.width) + ",";
	str += "\n\"height\":" + std::to_string(vo.height) + ",";

	str += "\n\"frames\":[";
	int l = vo.frames.size();
	for (int i = 0; i < l; ++i) {
		FrameInfoVo* pvo = vo.frames[i];

		str += "\n{";

		str += "\n\"i\":" + std::to_string(pvo->frameIndex) + ",";
		str += "\n\"x\":" + std::to_string(pvo->x) + ",";
		str += "\n\"y\":" + std::to_string(pvo->y) + ",";
		str += "\n\"w\":" + std::to_string(pvo->width) + ",";
		str += "\n\"h\":" + std::to_string(pvo->height);

		if (i == l - 1) {
			str += "\n}";
		}
		else {
			str += "\n},";
		}
	}

	str += "\n]";

	str += "\n}";

	FILE* f;
	fopen_s(&f, fileName, "w");
	fwrite(str.c_str(), 1, str.length(), f);
	fclose(f);
}

int decode_video(const char *fin, const char *outfilename)
{
	MovieInfoVo movieInfoVo;

	AVFormatContext* ctx_format = nullptr;
	AVCodecContext* ctx_codec = nullptr;
	AVCodec* codec = nullptr;
	AVFrame* frame = av_frame_alloc();

	int stream_idx;
	SwsContext* ctx_sws_rgb = nullptr;
	AVStream *vid_stream = nullptr;
	AVPacket* pkt = av_packet_alloc();

	//av_register_all();

	if (int ret = avformat_open_input(&ctx_format, fin, nullptr, nullptr) != 0) {
		std::cout << 1 << std::endl;
		return ret;
	}
	if (avformat_find_stream_info(ctx_format, nullptr) < 0) {
		std::cout << 2 << std::endl;
		return -1; // Couldn't find stream information
	}
	av_dump_format(ctx_format, 0, fin, false);

	for (int i = 0; i < ctx_format->nb_streams; i++)
		if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			stream_idx = i;
			vid_stream = ctx_format->streams[i];
			break;
		}
	if (vid_stream == nullptr) {
		std::cout << 4 << std::endl;
		return -1;
	}

	movieInfoVo.fps_num = vid_stream->avg_frame_rate.num;
	movieInfoVo.fps_den = vid_stream->avg_frame_rate.den;
	movieInfoVo.duration = ctx_format->duration;

	codec = avcodec_find_decoder(vid_stream->codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}
	ctx_codec = avcodec_alloc_context3(codec);

	if (avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar) < 0)
		std::cout << 512;
	if (avcodec_open2(ctx_codec, codec, nullptr) < 0) {
		std::cout << 5;
		return -1;
	}

	//av_new_packet(pkt, pic_size);

	char buf[1024];
	while (av_read_frame(ctx_format, pkt) >= 0) {
		if (pkt->stream_index == stream_idx) {
			int ret = avcodec_send_packet(ctx_codec, pkt);
			if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				std::cout << "avcodec_send_packet: " << ret << std::endl;
				break;
			}
			while (ret >= 0) {
				ret = avcodec_receive_frame(ctx_codec, frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					//std::cout << "avcodec_receive_frame: " << ret << std::endl;
					break;
				}
				//std::cout << "frame: " << ctx_codec->frame_number << std::endl;

				snprintf(buf, sizeof(buf), "%s/%d", outfilename, ctx_codec->frame_number);

				int dst_width = frame->width / 2;
				if (movieInfoVo.width < dst_width) {
					movieInfoVo.width = dst_width;
				}
				if (movieInfoVo.height < frame->height) {
					movieInfoVo.height = frame->height;
				}
				FrameInfoVo* frameInfo = new FrameInfoVo();
				frameInfo->frameIndex = ctx_codec->frame_number;
				movieInfoVo.frames.push_back(frameInfo);

				SaveARGB(ctx_codec, ctx_sws_rgb, frame, buf, frameInfo);
				
			}

		}
		av_packet_unref(pkt);
	}

	avformat_close_input(&ctx_format);
	av_packet_unref(pkt);
	sws_freeContext(ctx_sws_rgb);
	avformat_free_context(ctx_format);

	{
		snprintf(buf, sizeof(buf), "%s/%s", outfilename, "config");
		saveMovieInfoVo(movieInfoVo, buf);
	}
	
	return 0;
}