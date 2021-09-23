#include "ffAnimationTest.h"
#include "FFDecodeVideo.h"
#include <QFileDialog>

ffAnimationTest::ffAnimationTest(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	connect(ui.pushButton, &QPushButton::clicked, this, [=]() {
		auto url = "https://image-app-test.jiaoyoushow.com//app/logo/giftanimation/videoGift.mp4?1632279285";
		ui.widget->startPlay(url, QString());
	});

	connect(ui.pushButton_2, &QPushButton::clicked, this, &ffAnimationTest::sltVideoToImage);
}

ffAnimationTest::~ffAnimationTest()
{
	ui.widget->stopPlay();
}

void ffAnimationTest::sltVideoToImage()
{
	auto OpenFile = QFileDialog::getOpenFileName(this,
		"Please choose an mp4 file",
		"",
		"Image Files(*.mp4 *.MP4);");

	if (OpenFile.isEmpty()) {
		return;
	}
	//auto OpenFile = "videoGift.mp4";

	videoToImage(OpenFile, "out/");
}
