#pragma once

#include <QWidget>
#include <QTimer>
#include <QMutex>
#include "VideoMovieUtils.h"

enum class MP4ViewEvent 
{
	START,
	//ONELOOPEND,
	STOP,
	//MARK,
	//FRAME
};

////VideoMovieUtils::SpriteFrameVo
//struct MP4Frame
//{
//	QByteArray argb;	//png帧序列的 ARGB数据，注意需要使用方自己内存释放
//	int frameIndex;
//	int x;
//	int y;
//	int width;
//	int height;
//};
//
////VideoMovieUtils::SpriteSheetVo
//struct MP4Data
//{
//	QVector<QSharedPointer<MP4Frame>> frames;
//	int interval;
//	int width;
//	int height;
//	QImage::Format format;
//};

class MP4View :public QWidget
{
	Q_OBJECT
	typedef VideoMovieUtils::SpriteSheetVo MP4Data;

public:
	MP4View(QWidget *parent = nullptr);
	~MP4View();

	void stopPlay();

	void startPlay(const QString & url, QString path, float scale = 1.0f, int loop = 0);

	void stopWithIndex(int index);
//	void setScale(float x, float y);
//	/*
//	*根据帧获取当前图片
//	*/
//	QString getImageFileName(int index);
//		
protected:
	void paintEvent(QPaintEvent *event);

private:
	//绘制空数据
	void paintNull(QPainter *painter);
	//设置间隔
	void setInterval(int interval);
	////复制数据
	//void copyData(const MP4Data* data);

private slots:
	//定时更新下标
	void sltIndexUpdate();

private:
	//播放动画
	bool play(const MP4Data* list);
	//
	void playFinish();
	//检查结束帧
	bool checkEndFrame();
	//检查结束
	void checkFinish();

signals:
	void onEvent(MP4ViewEvent);

private:
	QTimer m_timerUpdate;
	QString m_url;
	float m_scale = 0.5;
	int m_loop = 0;
	int m_loopTimes = 0;
	int m_inverval = 20;
	int m_currIndex = 0;
	//最后停到哪一帧,0是第一帧,-1是最后一帧
	int m_stopIndex = -1;
	//MP4数据
	QSharedPointer<const MP4Data> m_data;
	QMutex m_mutex;
	
};


