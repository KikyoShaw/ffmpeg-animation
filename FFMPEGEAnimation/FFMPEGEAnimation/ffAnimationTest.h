#pragma once

#include <QtWidgets/QWidget>
#include "ui_ffAnimationTest.h"

class ffAnimationTest : public QWidget
{
    Q_OBJECT

public:
    ffAnimationTest(QWidget *parent = Q_NULLPTR);
	~ffAnimationTest();

private:
    Ui::ffAnimationTestClass ui;
};
