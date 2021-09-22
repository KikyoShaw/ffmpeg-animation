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
//	QByteArray argb;	//png֡���е� ARGB���ݣ�ע����Ҫʹ�÷��Լ��ڴ��ͷ�
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
//	*����֡��ȡ��ǰͼƬ
//	*/
//	QString getImageFileName(int index);
//		
protected:
	void paintEvent(QPaintEvent *event);

private:
	//���ƿ�����
	void paintNull(QPainter *painter);
	//���ü��
	void setInterval(int interval);
	////��������
	//void copyData(const MP4Data* data);

private slots:
	//��ʱ�����±�
	void sltIndexUpdate();

private:
	//���Ŷ���
	bool play(const MP4Data* list);
	//
	void playFinish();
	//������֡
	bool checkEndFrame();
	//������
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
	//���ͣ����һ֡,0�ǵ�һ֡,-1�����һ֡
	int m_stopIndex = -1;
	//MP4����
	QSharedPointer<const MP4Data> m_data;
	QMutex m_mutex;
	
};


