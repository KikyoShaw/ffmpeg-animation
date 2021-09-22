#include "ffAnimationTest.h"

ffAnimationTest::ffAnimationTest(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	connect(ui.pushButton, &QPushButton::clicked, this, [=]() {
		auto url = "https://image-app-test.jiaoyoushow.com//app/logo/giftanimation/videoGift.mp4?1632279285";
		ui.widget->startPlay(url, QString());
	});
}

ffAnimationTest::~ffAnimationTest()
{
	ui.widget->stopPlay();
}
