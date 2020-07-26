#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.importFace1Button, &QPushButton::clicked, this, &MainWindow::importFace1);
	connect(ui.importFace2Button, &QPushButton::clicked, this, &MainWindow::importFace2);
	connect(ui.faceDetectionButton, &QPushButton::clicked, this, &MainWindow::startSataFace);
	connect(ui.faceAlignmentButton, &QPushButton::clicked, this, &MainWindow::startFaceAlignment);
	face1 = nullptr;
	face2 = nullptr;
	face1Feat = nullptr;
	face2Feat = nullptr;
	faceFlag = true;

	//��ʼ������ʶ��ģ��
	faceIdentification = new seeta::FaceIdentification("./model/seeta_fr_v1.0.bin");
}

void MainWindow::importFace(int flag)
{
	//����֧��PNGͼƬ
	QString filename = QFileDialog::getOpenFileName(this, "ѡ��ͼƬ�ļ�", ".", "*.jpg *.bmp *.gif *.png");
	if (filename.isEmpty()) return;
	if (flag == 1)
	{
		this->face1 = new QImage(filename);
		ui.face1Label->setAlignment(Qt::AlignCenter);
		ui.face1Label->setPixmap(QPixmap::fromImage(*this->face1));//��ʾͼƬ
	}
	else if (flag == 2)
	{
		this->face2 = new QImage(filename);
		ui.face2Label->setAlignment(Qt::AlignCenter);
		ui.face2Label->setPixmap(QPixmap::fromImage(*this->face2));//��ʾͼƬ
	}
}

void MainWindow::importFace1()
{
	importFace(1);
}

void MainWindow::importFace2()
{
	importFace(2);
}

void MainWindow::startSataFace()
{
	if (this->face1 == nullptr || this->face2 == nullptr)
	{
		QMessageBox::information(this, "��ʾ��", "���ȵ�������ͼ��");
		return;
	}

	if (this->face1Feat == nullptr || this->face2Feat == nullptr)
	{
		QMessageBox::information(this, "��ʾ��", "���Ƚ���������⣡");
		return;
	}

	if (this->faceFlag == false)
	{
		//������ƶ�
		ui.similarityLabel->setText(QString("���ƶȣ�") + QString::number(0));
	}
	else
	{
		//�����ȶ�
		this->result = this->featureCompare(face1Feat, face2Feat);
		//������ƶ�
		ui.similarityLabel->setText(QString("���ƶȣ�") + QString::number(this->result));
	}
}

void MainWindow::startFaceAlignment()
{
	//��ʼ��
	ui.similarityLabel->setText(QString("���ƶȣ�"));

	if (this->face1Feat == nullptr) delete[] this->face1Feat;
	if (this->face2Feat == nullptr)delete[] this->face2Feat;
	this->face1Feat = this->face2Feat = nullptr;

	this->faceFlag = true;

	if (this->face1 == nullptr || this->face2 == nullptr)
	{
		QMessageBox::information(this, "��ʾ��", "���ȵ�������ͼ��");
		return;
	}

	cv::Mat face1Mat = QImage2cvMat(*this->face1);
	cv::Mat face2Mat = QImage2cvMat(*this->face2);

	//��face1Feat��face2Feat�����ڴ�
	face1Feat = newFeatureBuffer();
	face2Feat = newFeatureBuffer();

	std::thread t1(&MainWindow::faceDetectionFunc, this, std::ref(face1Mat), std::ref(face1Feat), "face_alignment_face1");
	std::thread t2(&MainWindow::faceDetectionFunc, this, std::ref(face2Mat), std::ref(face2Feat), "face_alignment_face2");
	t1.join();
	t2.join();

	cv::destroyAllWindows();
	cv::imshow("face_alignment_face1", face1Mat);
	cv::imshow("face_alignment_face2", face2Mat);
}

//�����Ƚ�
float MainWindow::featureCompare(float* feat1, float* feat2)
{
	return this->faceIdentification->CalcSimilarity(feat1, feat2);
}

//��float *���͵����ݷ����ڴ�
float* MainWindow::newFeatureBuffer()
{
	return new float[this->faceIdentification->feature_size()];
	//return new float[2048];
}

//������⣬������ȡ������ֵ��feat
void MainWindow::faceDetectionFunc(cv::Mat& mat, float* feat, std::string windName)
{
	/*
		1������ж��������������һ�����������������뻺����feat������true
		2�����û���������򷵻�false
	*/

	std::cout << std::this_thread::get_id() << endl;

	//��ʼ��ģ�ͣ���Ҫע����ǣ�ģ���ļ�ͬһʱ��ֻ�ܴ�һ�Σ����Ҽ���ģ��ʱ��Ҫһ����ʱ�䣬������Ҫ���м�������
	myMutex.lock();
	seeta::FaceDetection* faceDetection = new seeta::FaceDetection("./model/seeta_fd_frontal_v1.0.bin");//��ģ���ļ���·��ʵ����seeta::FaceDetection�Ķ���
	myMutex.unlock();
	faceDetection->SetMinFaceSize(20);//����Ҫ������������С�ߴ�(Ĭ��ֵ:20����������),Ҳ���������face_detector.SetMaxFaceSize(size);
	faceDetection->SetScoreThresh(2.f);//���ü�⵽�������ĵ÷���ֵ(Ĭ��ֵ:2.0)
	faceDetection->SetImagePyramidScaleFactor(0.8f);//����ͼ���������������(0 <����< 1��Ĭ��ֵ:0.8)
	faceDetection->SetWindowStep(4, 4);//���û������ڵĲ���(Ĭ��:4),face_detector.SetWindowStep(step_x, step_y);

	//��ʼ����������ģ��
	myMutex.lock();
	seeta::FaceAlignment* faceAlignment = new seeta::FaceAlignment("./model/seeta_fa_v1.1.bin");
	myMutex.unlock();

	//ͼƬ�Ҷȴ���
	cv::Mat src_img_gray;
	qDebug() << "mat.channels:" << mat.channels() << endl;
	cv::cvtColor(mat, src_img_gray, CV_BGR2GRAY);
	if (src_img_gray.empty())
	{
		myMutex.lock();
		this->faceFlag = false;
		myMutex.unlock();
		return;
	}
	//��ȡ�Ҷ�����
	seeta::ImageData src_img_gray_data(src_img_gray.cols, src_img_gray.rows, src_img_gray.channels());
	src_img_gray_data.data = src_img_gray.data;

	//��ȡͼƬ��ɫ
	cv::Mat src_img_color = mat;
	seeta::ImageData src_img_color_data(src_img_color.cols, src_img_color.rows, src_img_color.channels());
	src_img_color_data.data = src_img_color.data;

	//�������
	std::vector<seeta::FaceInfo>faces = faceDetection->Detect(src_img_gray_data);
	int32_t face_num = static_cast<int32_t>(faces.size());//������
	qDebug() << "��������" << face_num << endl;

	for (int32_t i = 0; i < face_num; ++i)
	{
		seeta::FacialLandmark tempPoints[5];//���������

		//���������㶨λ
		faceAlignment->PointDetectLandmarks(src_img_gray_data, faces[i], tempPoints);

		if (i == 0)//��һ���������⴦��
		{
			//��ȡ��һ������������ֵ��this->faceIdentification�������
			myMutex.lock();
			this->faceIdentification->ExtractFeatureWithCrop(src_img_color_data, tempPoints, feat);
			myMutex.unlock();

			//ʹ����ɫ������ѡ����һ������
			cv::rectangle(mat, cv::Point(faces[i].bbox.x, faces[i].bbox.y),
				cv::Point(faces[i].bbox.x + faces[i].bbox.width - 1, faces[i].bbox.y + faces[i].bbox.height - 1),
				CV_RGB(0, 0, 255));
		}
		else
		{
			//ʹ�ú�ɫ������ѡ����������
			cv::rectangle(mat, cv::Point(faces[i].bbox.x, faces[i].bbox.y),
				cv::Point(faces[i].bbox.x + faces[i].bbox.width - 1, faces[i].bbox.y + faces[i].bbox.height - 1),
				CV_RGB(255, 0, 0));
		}
		//�����㶨λ������㶨λ
		for (int j = 0; j < 5; ++j)
		{
			cv::circle(mat, cv::Point(tempPoints[j].x, tempPoints[j].y), 2, CV_RGB(0, 255, 0));
		}
	}
	cv::imwrite(windName + ".jpg", mat);

	//�ͷ���Դ
	delete faceDetection;
	delete faceAlignment;

	//δ��⵽��������faceFlag==false
	if (face_num == 0)
	{
		myMutex.lock();
		this->faceFlag = false;
		myMutex.unlock();
	}
}


// QImageתCV::Mat
cv::Mat MainWindow::QImage2cvMat(const QImage& inImage, bool inCloneImageData)
{
	switch (inImage.format())
	{
		// 8-bit, 4 channel
	case QImage::Format_ARGB32:
	case QImage::Format_ARGB32_Premultiplied:
	{
		cv::Mat  mat(inImage.height(), inImage.width(),
			CV_8UC4,
			const_cast<uchar*>(inImage.bits()),
			static_cast<size_t>(inImage.bytesPerLine())
		);

		return (inCloneImageData ? mat.clone() : mat);
	}
	// 8-bit, 3 channel
	case QImage::Format_RGB32:
	{
		if (!inCloneImageData)
		{
			qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
		}

		cv::Mat  mat(inImage.height(), inImage.width(),
			CV_8UC4,
			const_cast<uchar*>(inImage.bits()),
			static_cast<size_t>(inImage.bytesPerLine())
		);

		cv::Mat  matNoAlpha;

		cv::cvtColor(mat, matNoAlpha, cv::COLOR_BGRA2BGR);   // drop the all-white alpha channel

		return matNoAlpha;
	}
	// 8-bit, 3 channel
	case QImage::Format_RGB888:
	{
		if (!inCloneImageData)
		{
			qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
		}

		QImage   swapped = inImage.rgbSwapped();

		return cv::Mat(swapped.height(), swapped.width(),
			CV_8UC3,
			const_cast<uchar*>(swapped.bits()),
			static_cast<size_t>(swapped.bytesPerLine())
		).clone();
	}

	// 8-bit, 1 channel
	case QImage::Format_Indexed8:
	{
		cv::Mat  mat(inImage.height(), inImage.width(),
			CV_8UC1,
			const_cast<uchar*>(inImage.bits()),
			static_cast<size_t>(inImage.bytesPerLine())
		);

		return (inCloneImageData ? mat.clone() : mat);
	}
	default:
		qWarning() << "QImage2CvMat() - QImage format not handled in switch:" << inImage.format();
		break;
	}
	return cv::Mat();
}

//CV::MatתQImage
QImage MainWindow::cvMat2QImage(const cv::Mat& inMat)
{
	switch (inMat.type())
	{
		// 8-bit, 4 channel
	case CV_8UC4:
	{
		QImage image(inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_ARGB32);
		return image;
	}
	// 8-bit, 3 channel
	case CV_8UC3:
	{
		QImage image(inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	// 8-bit, 1 channel
	case CV_8UC1:
	{
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
		QImage image(inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_Grayscale8);
#else
		static QVector<QRgb>  sColorTable;

		// only create our color table the first time
		if (sColorTable.isEmpty())
		{
			sColorTable.resize(256);

			for (int i = 0; i < 256; ++i)
			{
				sColorTable[i] = qRgb(i, i, i);
			}
		}

		QImage image(inMat.data,
			inMat.cols, inMat.rows,
			static_cast<int>(inMat.step),
			QImage::Format_Indexed8);

		image.setColorTable(sColorTable);
#endif
		return image;
	}
	default:
		qWarning() << "cvMat2QImage() - cv::Mat image type not handled in switch:" << inMat.type();
		break;
	}
	return QImage();
}

MainWindow::~MainWindow()
{
	if (this->face1 != nullptr) delete this->face1;
	if (this->face2 != nullptr) delete this->face2;
	if (this->face1Feat != nullptr)delete[] this->face1Feat;
	if (this->face2Feat != nullptr)delete[] this->face2Feat;
	if (this->faceIdentification != nullptr)delete this->faceIdentification;
	this->face1 = nullptr;
	this->face2 = nullptr;
	this->face1Feat = nullptr;
	this->face2Feat = nullptr;
	this->faceIdentification = nullptr;
}

