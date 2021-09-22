#include "MP4View.h"
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "mp4mgr.h"

#define MP4_LOG (qInfo().noquote() << QStringLiteral("[MP4]"))

constexpr char* MP4_Path = "res/mp4/";

MP4View::MP4View(QWidget * parent) : QWidget(parent)
{
	connect(&m_timerUpdate, &QTimer::timeout, this, &MP4View::sltIndexUpdate);
}

MP4View::~MP4View()
{
}

void MP4View::stopPlay()
{
	m_data.clear();
	m_timerUpdate.stop();
	m_url.clear();
	m_scale = 0.5;
	m_loop = 0;
	m_loopTimes = 0;
	m_inverval = 50;
	m_currIndex = 0;
	m_stopIndex = -1;
}

void MP4View::startPlay(const QString & url, QString path, float scale, int loop)
{
	if (url.isEmpty()) {
		MP4_LOG << "url is empty";
		playFinish();
		return;
	}
	if (scale < 0.1) {
		MP4_LOG << "scale is empty";
		playFinish();
		return;
	}
	if (loop < 0) {
		MP4_LOG << "loop is empty";
		playFinish();
		return;
	}
	if (path.isEmpty()) {
		path = MP4_Path;
	}
	//��ֹͣ��ǰ���ڲ���
	stopPlay();
	//��������
	m_url = url;
	m_scale = scale;
	m_loop = loop;
	//���ز��Ŷ���
	QPointer<QObject> obj(this);
	mp4Mgr.loadAlphaMp4(url, path, true, [=](MP4Data* list) {
		if (obj.isNull()) {
			return;
		}
		if (m_url != url) {
			playFinish();
			return;
		}
		if (!play(list)) {
			playFinish();
			return;
		}
	});
}

bool MP4View::play(const MP4Data* list)
{
	if (list == Q_NULLPTR) {
		return false;
	}
	//��������
	QMutexLocker lock(&m_mutex);
	m_data = QSharedPointer<const MP4Data>(list);
	lock.unlock();
	//copyData(list);
	//��������
	m_timerUpdate.start(m_inverval);
	emit onEvent(MP4ViewEvent::START);
	return true;
}

void MP4View::sltIndexUpdate()
{
	m_currIndex++;
	//������
	checkFinish();
	//ˢ��
	update();
}

void MP4View::playFinish()
{
	//���ͣ��֡
	if (checkEndFrame()) {
		return;
	}
	stopPlay();
	emit onEvent(MP4ViewEvent::STOP);
}

bool MP4View::checkEndFrame()
{
	if (m_stopIndex >= 0) {
		m_currIndex = m_stopIndex;
		return true;
	}
	return false;
}

void MP4View::checkFinish()
{
	//����ѭ������
	auto size = m_data ? m_data->frames.size() : 0;
	if (m_currIndex >= size) {
		m_loopTimes++;
		m_currIndex = 0;
	}
	//������
	if (m_loop > 0 && m_loopTimes >= m_loop) {
		playFinish();
	}
}

void MP4View::stopWithIndex(int index)
{
	m_stopIndex = index;
}

//void MP4View::setScale(float x, float y)
//{
//	setExtraScale(new float[1]{ x }, new float[1]{ y });
//}
//
//QString MP4View::getImageFileName(int index)
//{
//	return mDataParser.getImageFileName(index);
//}

void MP4View::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QMutexLocker lock(&m_mutex);
	QPainter painter(this);
	//
	if (m_data.isNull()) {
		paintNull(&painter);
		return;
	}
	//����±�
	auto size = m_data->frames.size();
	if (m_currIndex < 0 || m_currIndex >= size) {
		paintNull(&painter);
		return;
	}
	//���֡����
	auto frame = m_data->frames.value(m_currIndex);
	if (frame == Q_NULLPTR) {
		paintNull(&painter);
		return;
	}
	auto temp = QImage(frame->argb, frame->width, frame->height, m_data->format);
	if (temp.isNull()) {
		paintNull(&painter);
		return;
	}
	//������ʾ
	//MP4������Χ
	QSize viewSize = QSize(m_data->width, m_data->height) * m_scale;
	QRect viewRect(QPoint(0, 0), viewSize);
	//���ڷ�Χ
	QRect painterRect(0, 0, painter.device()->width(), painter.device()->height());
	//�Ƚ���������
	viewRect.moveCenter(painterRect.center());
	//���ƶ�֡����
	auto framePoint = QPoint(frame->x, frame->y) * m_scale;
	auto point = viewRect.topLeft() + framePoint;
	//ÿ֡��Χ
	QSize frameSize = QSize(frame->width, frame->height) * m_scale;
	//��Ⱦ
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.drawPixmap(point, QPixmap::fromImage(temp).scaled(frameSize));
}

void MP4View::paintNull(QPainter * painter)
{
	QStyleOption opt;
	opt.initFrom(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, painter, this);
}

void MP4View::setInterval(int interval)
{
	if (interval <= 0 || interval > 500) {
		return;
	}
	m_inverval = interval;
}

//void MP4View::copyData(const MP4Data * data)
//{
//	if (data == Q_NULLPTR) {
//		return;
//	}
//	//���ü��
//	setInterval(data->inverval);
//	//��������
//	QMutexLocker lock(&m_mutex);
//	m_data = QSharedPointer<MP4Data>(new MP4Data);
//	m_data->format = data->format;
//	m_data->height = data->height;
//	m_data->width = data->width;
//	m_data->interval = data->inverval;
//	for (auto frame : data->frames) {
//		//���ݿձ�ʾ��͸��ͼƬ
//		if (frame == Q_NULLPTR) {
//			m_data->frames.append(Q_NULLPTR);
//		}
//		else {
//			auto node = QSharedPointer<MP4Frame>(new MP4Frame);
//			node->frameIndex = frame->frameIndex;
//			node->height = frame->height;
//			node->width = frame->width;
//			node->x = frame->x;
//			node->y = frame->y;
//			node->argb = QByteArray((char*)frame->argb);
//			m_data->frames.append(node);
//		}
//	}
//}