#pragma once

#include <QImage>
#include <QtCore>

namespace VideoMovieUtils
{
	struct SpriteFrameVo {
		uchar* argb = nullptr;	//png帧序列的 ARGB数据，注意需要使用方自己内存释放

		int frameIndex;

		int x;
		int y;
		int width;
		int height;

		~SpriteFrameVo();

		SpriteFrameVo* clone();
	};

	struct SpriteSheetVo
	{
		QVector<SpriteFrameVo*> frames;	//png帧序列的 ARGB数据，注意需要使用方自己内存释放
		int inverval;			//帧之间的时间间隔，单位:ms
		
		int width;
		int height;

		QImage::Format format;	//目前一定是 Format_RGBA8888

		~SpriteSheetVo();

		SpriteSheetVo* clone();
	};

	typedef std::function<void(SpriteSheetVo*)> FnCallback;
	//typedef void(*FnCallback)(SpriteSheetVo* vo);

	QString getSaveFolderFromFilePath(const QString& strFilePath);

	//SpriteSheetVo* VUTILITY_LIBRARY_EXPORT getSpriteSheet(const QString& strSaveFolder);
	bool getSpriteSheet(const QString& strSaveFolder, SpriteSheetVo*& ret, bool bDecode);

	//void VUTILITY_LIBRARY_EXPORT convertAlphaMp4ToPngs(QObject* context, const QString& strFilePath, bool bDecode, const FnCallback callBack);
	void convertAlphaMp4ToPngs(QObject* context, const QString& strFilePath, const QString& strSaveFolder, bool bDecode, const FnCallback callBack);

};
