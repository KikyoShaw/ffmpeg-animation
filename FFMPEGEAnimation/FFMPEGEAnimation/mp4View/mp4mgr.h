#pragma once

#include <QObject>
#include "VideoMovieUtils.h"
#include <QNetworkAccessManager>
#include <QMutex>

class QNetworkAccessManager;

class Mp4Mgr : public QObject
{
	Q_OBJECT
public:
	static Mp4Mgr& getInstance();
	~Mp4Mgr();

	// ����MP4��ʽ���������֡���Ҵ����ص�
	void loadAlphaMp4(const QString &url, const QString& saveFolder, bool bDecode, const VideoMovieUtils::FnCallback callBack);

private:
	QNetworkAccessManager m_networkManager;

private:
	Mp4Mgr(QObject *parent = Q_NULLPTR);
	Q_DISABLE_COPY(Mp4Mgr)
};

#define mp4Mgr Mp4Mgr::getInstance()
