#pragma once

#include <QtWidgets/QMainWindow>
#include "x64/Release/uic/ui_MainWindow.h"
#include "ui_MainWindow.h"
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QImage>
#include <QDebug>
#include <QMessageBox>

#include <mutex>
#include <thread>

#include <opencv2/opencv.hpp>  
#include <opencv2/highgui/highgui.hpp>  

#include "SeetaFace\FaceAlignment\face_alignment.h"
#include "SeetaFace\FaceDetection\face_detection.h"
#include "SeetaFace\FaceIdentification\face_identification.h"

#pragma execution_character_set("utf-8")

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();
	
	void importFace(int flag);

public slots:
	void importFace1();
	void importFace2();
	void startSataFace();
	void startFaceAlignment();

private:
	Ui::MainWindowClass ui;

	QImage* face1;
	QImage* face2;

	seeta::FaceIdentification* faceIdentification;
	float* face1Feat;//图片1特征值   设置指针的原因：float数组
	float* face2Feat;//图片2特征值
	bool faceFlag;//两图中是否有人脸

	float result;//比对结果
	std::mutex myMutex;

private:
	//QImage转Mat
	cv::Mat QImage2cvMat(const QImage& inImage, bool inCloneImageData = true);
	//Mat转QImage
	QImage cvMat2QImage(const cv::Mat& inMat);
	//给float *类型的数据分配内存
	float* newFeatureBuffer();
	//特征抽取，并赋值给feat
	void faceDetectionFunc(cv::Mat& mat, float* feat, std::string windName);
	//特征比较
	float featureCompare(float* feat1, float* feat2);
};
