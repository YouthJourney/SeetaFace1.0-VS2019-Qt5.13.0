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
	float* face1Feat;//ͼƬ1����ֵ   ����ָ���ԭ��float����
	float* face2Feat;//ͼƬ2����ֵ
	bool faceFlag;//��ͼ���Ƿ�������

	float result;//�ȶԽ��
	std::mutex myMutex;

private:
	//QImageתMat
	cv::Mat QImage2cvMat(const QImage& inImage, bool inCloneImageData = true);
	//MatתQImage
	QImage cvMat2QImage(const cv::Mat& inMat);
	//��float *���͵����ݷ����ڴ�
	float* newFeatureBuffer();
	//������ȡ������ֵ��feat
	void faceDetectionFunc(cv::Mat& mat, float* feat, std::string windName);
	//�����Ƚ�
	float featureCompare(float* feat1, float* feat2);
};
