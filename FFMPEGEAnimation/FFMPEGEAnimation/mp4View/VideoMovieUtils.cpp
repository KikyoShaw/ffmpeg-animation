
#include "VideoMovieUtils.h"

namespace VideoMovieUtils {

SpriteFrameVo::~SpriteFrameVo() {
	delete argb;
	argb = nullptr;
}

SpriteFrameVo* SpriteFrameVo::clone() {
	SpriteFrameVo* ret = new SpriteFrameVo();
	ret->frameIndex = this->frameIndex;
	ret->x = this->x;
	ret->y = this->y;
	ret->width = this->width;
	ret->height = this->height;
	if (nullptr == argb) {
		return ret;
	}

	const int l = width * height * 4;
	ret->argb = new uchar[l];
	memcpy(ret->argb, this->argb, l);
	return ret;
}

SpriteSheetVo::~SpriteSheetVo() {
	for (QVector<SpriteFrameVo*>::iterator it = frames.begin(); it != frames.end(); ++it) {
		delete *it;
		*it = nullptr;
	}
	frames.clear();
}

SpriteSheetVo* SpriteSheetVo::clone() {
	//QVector<SpriteFrameVo*> frames;	//png帧序列的 ARGB数据，注意需要使用方自己内存释放
	//int inverval;			//帧之间的时间间隔，单位:ms

	//int width;
	//int height;

	//QImage::Format format;	//目前一定是 Format_RGBA8888

	SpriteSheetVo* ret = new SpriteSheetVo();
	ret->inverval = this->inverval;
	ret->width = this->width;
	ret->height = this->height;
	ret->format = this->format;


	for (auto it = this->frames.begin(); it != this->frames.end(); ++it) {
		ret->frames.push_back((*it)->clone());
	}

	return ret;

}

void insertAndSort(SpriteSheetVo& vo, SpriteFrameVo* frameVo) {
	int l = vo.frames.size();
	vo.frames.push_back(frameVo);

	int findIndex = -1;
	for (int i = l - 1; i >= 0; --i) {
		SpriteFrameVo* t = vo.frames[i];
		if (t->frameIndex <= frameVo->frameIndex) {
			break;
		}

		vo.frames[i + 1] = t;
		findIndex = i;
	}

	if (findIndex >= 0) {
		vo.frames[findIndex] = frameVo;
	}
}

bool hasConfigFile(const QString& strSaveFolder) {
	QDir saveFolder(strSaveFolder);
	QFile configFile(saveFolder.absoluteFilePath("config"));
	return configFile.exists();
}

SpriteSheetVo* getSpriteSheet(const QString& strSaveFolder) {
	QDir saveFolder(strSaveFolder);
	if (!saveFolder.exists()) {
		saveFolder.mkpath(strSaveFolder);
	}
	QFile configFile(saveFolder.absoluteFilePath("config"));
	if (!configFile.exists()) {
		return nullptr;
	}

	if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return nullptr;
	}

	QByteArray bArrJson = configFile.readAll();
	QJsonDocument jsonDoc;
	jsonDoc = QJsonDocument::fromJson(bArrJson);
	if (jsonDoc.isNull())
	{
		return nullptr;
	}
	if (!jsonDoc.isObject()) {
		return nullptr;
	}

	SpriteSheetVo* pvo = new SpriteSheetVo();
	SpriteSheetVo& vo = *pvo;

	vo.format = QImage::Format_RGBA8888;

	QJsonObject jsonObj = jsonDoc.object();
	QJsonValue fps_num = jsonObj.value("fps_num");
	QJsonValue fps_den = jsonObj.value("fps_den");
	if (!fps_num.isDouble() || !fps_den.isDouble()) {
		delete pvo;
		pvo = nullptr;
		return false;
	}
	vo.inverval = ceil(1000 * fps_den.toInt() / fps_num.toDouble());

	QJsonValue width = jsonObj.value("width");
	if (!width.isDouble()) {
		delete pvo;
		pvo = nullptr;
		return pvo;
	}
	vo.width = width.toInt();
	QJsonValue height = jsonObj.value("height");
	if (!height.isDouble()) {
		delete pvo;
		pvo = nullptr;
		return pvo;
	}
	vo.height = height.toInt();

	QJsonValue duration = jsonObj.value("duration");
	if (!duration.isDouble()) {
		delete pvo;
		pvo = nullptr;
		return pvo;
	}

	//遍历帧数据
	QJsonValue frames = jsonObj.value("frames");
	if (!frames.isArray()) {
		delete pvo;
		pvo = nullptr;
		return pvo;
	}
	QJsonArray jsonFrames = frames.toArray();
	const int frameCount = jsonFrames.size();
	for (int i = 0; i < frameCount; ++i) {
		QJsonValue jvFrames = jsonFrames[i];
		if (!jvFrames.isObject()) {
			delete pvo;
			pvo = nullptr;
			return pvo;
		}
		QJsonObject jv = jvFrames.toObject();

		QJsonValue jvW = jv.value("w");
		if (!jvW.isDouble()) {
			delete pvo;
			pvo = nullptr;
			return pvo;
		}
		int w = jvW.toInt();

		QJsonValue jvFrameIndex = jv.value("i");
		if (!jvFrameIndex.isDouble()) {
			delete pvo;
			pvo = nullptr;
			return pvo;
		}
		int frameIndex = jvFrameIndex.toInt();

		if (w <= 0) {
			SpriteFrameVo* frameVo = new SpriteFrameVo();
			frameVo->frameIndex = frameIndex;
			frameVo->argb = nullptr;
			frameVo->x = 0;
			frameVo->y = 0;
			frameVo->width = 0;
			frameVo->height = 0;

			vo.frames.push_back(frameVo);
			continue;
		}

		QJsonValue jvH = jv.value("h");
		if (!jvH.isDouble()) {
			delete pvo;
			pvo = nullptr;
			return pvo;
		}
		int h = jvH.toInt();

		QJsonValue jvX = jv.value("x");
		if (!jvX.isDouble()) {
			delete pvo;
			pvo = nullptr;
			return pvo;
		}
		int x = jvX.toInt();

		QJsonValue jvY = jv.value("y");
		if (!jvY.isDouble()) {
			delete pvo;
			pvo = nullptr;
			return pvo;
		}
		int y = jvY.toInt();

		const int argbBufSize = w * h * 4;

		QString frameFilePath = saveFolder.absoluteFilePath(QString("%1").arg(frameIndex));
		QFileInfo info(frameFilePath);
		if (!info.exists() || info.size() != argbBufSize) {
			delete pvo;
			pvo = nullptr;
			return pvo;
		}

		SpriteFrameVo* frameVo = new SpriteFrameVo();
		frameVo->frameIndex = frameIndex;
		frameVo->argb = new uchar[argbBufSize];
		frameVo->x = x;
		frameVo->y = y;
		frameVo->width = w;
		frameVo->height = h;
		
		FILE* f;
		errno_t err = fopen_s(&f, frameFilePath.toUtf8().data(), "rb");
		if (0 != err) {
			delete frameVo;
			delete pvo;
			pvo = nullptr;
			return pvo;
		}
		fread_s(frameVo->argb, argbBufSize, sizeof(uchar), argbBufSize, f);
		fclose(f);

		
		vo.frames.push_back(frameVo);
	}

	//const int argbBufSize = vo.width * vo.height * 4;
	//saveFolder.setFilter(QDir::Files); //===设置过滤配置,接受文件
	//QFileInfoList fileInfoList = saveFolder.entryInfoList();
	//QStringList fileList;
	//for (int inf = 0; inf < fileInfoList.count(); inf++) {
	//	//====如果需要筛选指定文件可以在这里添加判断
	//	const QFileInfo& info = fileInfoList.at(inf);
	//	if (info.size() != argbBufSize) continue;

	//	QString fileName = info.fileName();
	//	bool bConvertInt = false;
	//	const int frameIndex = fileName.toInt(&bConvertInt);
	//	if (!bConvertInt) continue;

	//	SpriteFrameVo* frameVo = new SpriteFrameVo();
	//	frameVo->frameIndex = frameIndex;
	//	frameVo->argb = new uchar[argbBufSize];
	//	frameVo->x = 0;
	//	frameVo->y = 0;
	//	frameVo->width = vo.width;
	//	frameVo->height = vo.height;

	//	FILE* f;
	//	errno_t err = fopen_s(&f, info.absoluteFilePath().toUtf8().data(), "rb");
	//	if (0 != err) {
	//		delete frameVo;
	//		delete pvo;
	//		pvo = nullptr;
	//		return pvo;
	//	}
	//	fread_s(frameVo->argb, argbBufSize, sizeof(uchar), argbBufSize, f);

	//	fclose(f);

	//	insertAndSort(vo, frameVo);

	//}

	return pvo;

}

bool getSpriteSheet(const QString& strSaveFolder, SpriteSheetVo*& ret, bool bDecode)
{
	if (bDecode) {
		//需要解码的话，尝试解码
		ret = getSpriteSheet(strSaveFolder);
		return nullptr != ret;
	}
	else {
		//不需要解码，判定是否已经解码完成了
		return hasConfigFile(strSaveFolder);
	}
}

void convertAlphaMp4ToPngs(QObject* context, const QString& strFilePath, const QString& strSaveFolder, bool bDecode, const FnCallback callBack)
{
	{
		SpriteSheetVo* ret = nullptr;
		if (getSpriteSheet(strSaveFolder, ret, bDecode)) {
			callBack(ret);
			return;
		}
	}

	QString strCmd = QString("FFMPEGExtend.exe mp42png \"%1\" \"%2\"").arg(strFilePath).arg(strSaveFolder);
	qInfo().noquote() << "start mp4 parse" << strCmd;
	QProcess* process = new QProcess(context);
	QObject::connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), context, [=](int exitCode, QProcess::ExitStatus exitStatus) {

		process->close();
		process->deleteLater();

		if (bDecode) {
			callBack(getSpriteSheet(strSaveFolder));
		}
		else {
			callBack(nullptr);
		}
	});

	process->start(strCmd);
}

QString getSaveFolderFromFilePath(const QString& strFilePath) {
	QFileInfo fileInfo(strFilePath);
	QDir dir(fileInfo.path());

	//qint64 mtick = fileInfo.lastModified().toSecsSinceEpoch();
	//QString strSaveFolder = dir.absoluteFilePath(fileInfo.baseName()) + QString("%1").arg(mtick);
	QString strSaveFolder = dir.absoluteFilePath(fileInfo.baseName());

	return strSaveFolder;
}

void convertAlphaMp4ToPngs(QObject* context, const QString& strFilePath, bool bDecode, const FnCallback callBack) {
	QString strSaveFolder = getSaveFolderFromFilePath(strFilePath);

	convertAlphaMp4ToPngs(context, strFilePath, strSaveFolder, bDecode, callBack);
}

}
