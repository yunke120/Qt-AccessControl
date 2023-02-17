#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (systemInit("kf4Er8xXdzWCD97fS44ghb6Lg9MMDtzPbqftjacyuM2", "BHwSM2pzFzcHUE8ZT9EfuSJENVLcHgHXGq3q6bEEmnkb"))
    {
        qDebug() << "system init failed!";
        return;
    }

    if (sqlInit())
    {
        qErrnoWarning("sqlInit failed.");
        return;
    }

    //    qErrnoWarning("sqlInit success.");
    tablewidgetInit();

    this->loadUserInfo(db, "userData");
    for (int i = 0; i < username_list.size(); i++)
    {
        QImage image = cvMat2QImage(image_list.at(i));
        insertOneRowInTable(i, username_list.at(i), image);
    }
}

MainWindow::~MainWindow()
{
    if (pCap != nullptr && pCap->isOpened())
    {
        pCap->release();
        pCap = nullptr;
    }
    delete ui;
}

int MainWindow::systemInit(QString app_id, QString sdk_key)
{
    MRESULT res = MOK;
    res = ASFOnlineActivation(app_id.toLatin1().data(), sdk_key.toLatin1().data());
    if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
        return 1;
    else
        qDebug() << "ASFActivation sucess";

    // 初始化接口
    MInt32 mask1 = ASF_FACE_DETECT | ASF_FACERECOGNITION; /* 只用人脸检测和人脸特征 */
    res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY, 30, 5, mask1, &imageHandle);
    if (res != MOK)
        return 2;
    else
        qDebug() << "image ASFInitEngine success" << res;

    MInt32 mask2 = ASF_FACE_DETECT | ASF_FACERECOGNITION; /* 只用人脸检测和人脸特征 */
    res = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_0_ONLY, 25, 1, mask2, &videoHandle);
    if (res != MOK)
        return 3;
    else
        qDebug() << "video ASFInitEngine success";

    return 0;
}

int MainWindow::sqlInit()
{
    pSqllite = new FACE_SQL;
    db = pSqllite->create_db("face1", "userinfo");
    // unsigned not null primary key
    if (!pSqllite->isExistTable(db, "userData"))
    {
        if (pSqllite->create_table(db, "create table userData        \
                                  (                                        \
                                      id int primary key,  \
                                      username char(16) not null,            \
                                      pl char(9) not null,                   \
                                      reserve1 char(9),                      \
                                      reserve2 char(9),                      \
                                      featuresize int unsigned not null,     \
                                      featuredata blob not null,             \
                                      imgtype int not null,                  \
                                      imgcols int unsigned not null,         \
                                      imgrows int unsigned not null,         \
                                      imgsize int unsigned not null,         \
                                      imgdata blob not null                  \
                                      );"))
        {
            qDebug() << "create table success.";
        }
        else
        {
            qErrnoWarning("create table failed.");
            return 1;
        }
    }
    else
    {
        qDebug() << "table userData is existed.";
        return 0;
    }
    return 0;
}

bool MainWindow::loadUserInfo(QSqlDatabase _db, QString table)
{
    feature_list.clear();
    image_list.clear();
    username_list.clear();

    int user_len = pSqllite->getAllUsername(_db, table, username_list);

    //    qDebug() << username_list;
    for (int i = 0; i < user_len; i++)
    {
        SQL_DATA_TypeDef sdt;
        if (!pSqllite->getUserInfoByUsername(_db, "userData", username_list.at(i), sdt))
        {
            qDebug() << QString("no %1").arg(username_list.at(i));
            continue;
        }
        qDebug() << sdt.t_username;

        //        ASF_FaceFeature feature = {0};
        //        feature.featureSize = sdt.t_featuresize;
        //        feature.feature = (MByte *)malloc(feature.featureSize);
        //        memset(feature.feature, 0, feature.featureSize);
        //        feature.feature = (MByte *)sdt.t_featuredata.data();
        //        feature_list << feature;

        cv::Mat ret_image(sdt.t_imgrows, sdt.t_imgcols, sdt.t_imgtype, (void *)sdt.t_imgdata.data());
        image_list << ret_image.clone();

        QString path = "./IMAGES/" + sdt.t_username + ".jpg";
        ASF_FaceFeature feature2 = {0};
        getFeatureByImage(path, feature2);
        feature_list << feature2;
    }
    return true;
}

void MainWindow::CutIplImage(IplImage *src, IplImage *dst, int x, int y)
{
    CvSize size = cvSize(dst->width, dst->height);             // 区域大小
    cvSetImageROI(src, cvRect(x, y, size.width, size.height)); // 设置源图像ROI
    cvCopy(src, dst);                                          // 复制图像
    cvResetImageROI(src);                                      // 源图像用完后，清空ROI
}

/**
 * @brief MainWindow::cvMat2QImage 将Mat类型转为QImage类型
 * @param mat 输入Mat数据
 * @return QImage类
 */
QImage MainWindow::cvMat2QImage(const Mat &mat)
{
    if (mat.type() == CV_8UC1) // 8-bits unsigned, NO. OF CHANNELS = 1
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        image.setColorCount(256); // Set the color table (used to translate colour indexes to qRgb values)
        for (int i = 0; i < 256; i++)
            image.setColor(i, qRgb(i, i, i));

        uchar *pSrc = mat.data; // Copy input Mat
        for (int row = 0; row < mat.rows; row++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, static_cast<size_t>(mat.cols));
            pSrc += mat.step;
        }
        return image;
    }
    else if (mat.type() == CV_8UC3) // 8-bits unsigned, NO. OF CHANNELS = 3
    {
        const uchar *pSrc = reinterpret_cast<const uchar *>(mat.data);                             // Copy input Mat
        QImage image(pSrc, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888); // Create QImage with same dimensions as input Mat
        return image.rgbSwapped();
    }
    else if (mat.type() == CV_8UC4)
    {
        const uchar *pSrc = reinterpret_cast<const uchar *>(mat.data);
        QImage image(pSrc, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        return QImage(); // Mat could not be converted to QImage
    }
}

Mat MainWindow::QImage2cvMat(QImage &image)
{
    cv::Mat mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void *)image.constBits(), image.bytesPerLine());
    return mat;
}

bool MainWindow::getFeatureByVideo(Mat img, ASF_FaceFeature &copyfeature)
{
    MRESULT res = MOK;
    IplImage imgTmp = img;
    IplImage *img1 = cvCloneImage(&imgTmp); /* 深拷贝 */

    if (!img1)
        return false;

    ASF_MultiFaceInfo detectedFaces = {0};        /* 多人脸信息 */
    ASF_SingleFaceInfo SingleDetectedFaces = {0}; /* 单人脸信息 */
    ASF_FaceFeature feature = {0};                /* 人脸特征 */
    /* 4字节对齐 */
    IplImage *cutImg1 = cvCreateImage(cvSize(img1->width - img1->width % 4, img1->height), IPL_DEPTH_8U, img1->nChannels);
    CutIplImage(img1, cutImg1, 0, 0);

    ASVLOFFSCREEN offscreen = {0};
    offscreen.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
    offscreen.i32Width = cutImg1->width;
    offscreen.i32Height = cutImg1->height;
    offscreen.pi32Pitch[0] = cutImg1->widthStep;
    offscreen.ppu8Plane[0] = (MUInt8 *)cutImg1->imageData;

    res = ASFDetectFacesEx(videoHandle, &offscreen, &detectedFaces);
    if (res == MOK && detectedFaces.faceNum >= 1)
    {
        SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
        SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
        SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
        SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;
        SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];

        res = ASFFaceFeatureExtractEx(videoHandle, &offscreen, &SingleDetectedFaces, &feature);
        if (res == MOK)
        { /* 第一次提取特征需进行拷贝，因为使用同一个引擎，第二次拷贝会覆盖第一次提取的结果 */
            // 拷贝feature
            copyfeature.featureSize = feature.featureSize;
            copyfeature.feature = (MByte *)malloc(feature.featureSize);
            memset(copyfeature.feature, 0, feature.featureSize);
            memcpy(copyfeature.feature, feature.feature, feature.featureSize);
        }
        else
            return false;
    }
    else
        return false;
    return true;
}

bool MainWindow::getFeatureByImage(QString img_path, ASF_FaceFeature &copyfeature)
{
    MRESULT res = MOK;
    IplImage *img1 = cvLoadImage(img_path.toLatin1().data());

    if (!img1)
        return false;

    ASF_MultiFaceInfo detectedFaces = {0};        /* 多人脸信息 */
    ASF_SingleFaceInfo SingleDetectedFaces = {0}; /* 单人脸信息 */
    ASF_FaceFeature feature = {0};                /* 人脸特征 */
    /* 4字节对齐 */
    IplImage *cutImg1 = cvCreateImage(cvSize(img1->width - img1->width % 4, img1->height), IPL_DEPTH_8U, img1->nChannels);
    CutIplImage(img1, cutImg1, 0, 0);

    ASVLOFFSCREEN offscreen = {0};
    offscreen.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
    offscreen.i32Width = cutImg1->width;
    offscreen.i32Height = cutImg1->height;
    offscreen.pi32Pitch[0] = cutImg1->widthStep;
    offscreen.ppu8Plane[0] = (MUInt8 *)cutImg1->imageData;

    res = ASFDetectFacesEx(imageHandle, &offscreen, &detectedFaces);
    if (res == MOK && detectedFaces.faceNum >= 1)
    {
        SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
        SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
        SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
        SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;
        SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];

        res = ASFFaceFeatureExtractEx(imageHandle, &offscreen, &SingleDetectedFaces, &feature);
        if (res == MOK)
        { /* 第一次提取特征需进行拷贝，因为使用同一个引擎，第二次拷贝会覆盖第一次提取的结果 */
            // 拷贝feature
            copyfeature.featureSize = feature.featureSize;
            copyfeature.feature = (MByte *)malloc(feature.featureSize);
            memset(copyfeature.feature, 0, feature.featureSize);
            memcpy(copyfeature.feature, feature.feature, feature.featureSize);
        }
        else
            return false;
    }
    else
        return false;
    return true;
}

QStringList MainWindow::getImageFilePath(QString dirpath)
{

    QDir dir(dirpath);
    QStringList files_path;
    files_path.clear();
    if (!dir.exists())
        return files_path;

    QStringList nameFilters;
    nameFilters << "*.png"
                << "*.jpg";
    QStringList files_name = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

    for (int i = 0; i < files_name.size(); i++)
    {
        files_path << dirpath + files_name.at(i);
    }

    return files_path;
}

void MainWindow::getImagesFeature(QString dirpath, QList<ASF_FaceFeature> &list)
{

    MRESULT res = MOK;
    files_path = getImageFilePath(dirpath);

    /* 遍历要查找的图片，一一比对其相似度 */
    for (int i = 0; i < files_path.size(); i++)
    {
        //        qDebug() << files_path.at(i);
        IplImage *img2 = cvLoadImage(files_path.at(i).toLatin1().data());

        // 第二张人脸提取特征
        ASF_MultiFaceInfo detectedFaces2 = {0};
        ASF_SingleFaceInfo SingleDetectedFaces2 = {0};
        ASF_FaceFeature feature2 = {0};
        ASF_FaceFeature copyfeature2 = {0};
        IplImage *cutImg2 = cvCreateImage(cvSize(img2->width - img2->width % 4, img2->height), IPL_DEPTH_8U, img2->nChannels);
        CutIplImage(img2, cutImg2, 0, 0);

        ASVLOFFSCREEN offscreen2 = {0};
        offscreen2.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
        offscreen2.i32Width = cutImg2->width;
        offscreen2.i32Height = cutImg2->height;
        offscreen2.pi32Pitch[0] = cutImg2->widthStep;
        offscreen2.ppu8Plane[0] = (MUInt8 *)cutImg2->imageData;

        res = ASFDetectFacesEx(imageHandle, &offscreen2, &detectedFaces2);
        if (MOK == res && detectedFaces2.faceNum >= 1)
        {
            SingleDetectedFaces2.faceRect.left = detectedFaces2.faceRect[0].left;
            SingleDetectedFaces2.faceRect.top = detectedFaces2.faceRect[0].top;
            SingleDetectedFaces2.faceRect.right = detectedFaces2.faceRect[0].right;
            SingleDetectedFaces2.faceRect.bottom = detectedFaces2.faceRect[0].bottom;
            SingleDetectedFaces2.faceOrient = detectedFaces2.faceOrient[0];

            res = ASFFaceFeatureExtractEx(imageHandle, &offscreen2, &SingleDetectedFaces2, &feature2);
            copyfeature2 = {0};
            if (res == MOK)
            {
                // 拷贝feature
                copyfeature2.featureSize = feature2.featureSize;
                copyfeature2.feature = (MByte *)malloc(feature2.featureSize);
                memset(copyfeature2.feature, 0, feature2.featureSize);
                memcpy(copyfeature2.feature, feature2.feature, feature2.featureSize);
                list << copyfeature2;
            }
            else
            {
                qDebug() << QString("extract image_%1 feature failed").arg(i);
            }
        }
        else
        {
            qDebug() << QString("ASFDetectFaces %1 fail: %2").arg(i).arg(res);
        }
    }
    qDebug() << "Traverse complete";
}

void MainWindow::tablewidgetInit()
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    QHeaderView *headerViewLie = ui->tableWidget->verticalHeader();
    headerViewLie->setHidden(1); /* 不显示tableWidget自带序号 */
    QStringList headerHeng;
    headerHeng << QStringLiteral("序号")
               << QStringLiteral("用户名")
               << QStringLiteral("图片")
               << QStringLiteral("操作");
    ui->tableWidget->setColumnCount(headerHeng.size());
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setHorizontalHeaderLabels(headerHeng);
    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:lightgreen;}");
    ui->tableWidget->horizontalHeader()->setMinimumHeight(40);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    for (int i = 0; i < headerHeng.size(); i++)
    {
        ui->tableWidget->verticalHeader()->setDefaultSectionSize(200);
        if (0 == i) /* uv_number只可能为0和uv,而这两列都是有必要的 */
        {           /* 将uv显示的那一列宽度设置为跟随内容变化，这样在分辨率低的显示屏上一样可以显示完全 */
            ui->tableWidget->setColumnWidth(i, 40);
            continue;
        }
        //        ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        ui->tableWidget->setColumnWidth(i, 200);
    }

    user_id = 0;
}

void MainWindow::insertOneRowInTable(int id, QString username, QImage image)
{
    int rowcount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowcount);
    QTableWidgetItem *item0 = new QTableWidgetItem();
    item0->setText(QString::number(id));
    item0->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    ui->tableWidget->setItem(line, 0, item0);

    QTableWidgetItem *item1 = new QTableWidgetItem();
    item1->setText(username);
    item1->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    ui->tableWidget->setItem(line, 1, item1);

    QLabel *label = new QLabel();
    label->setPixmap(QPixmap::fromImage(image.scaled(200, 200)));
    label->setAlignment(Qt::AlignHCenter);
    ui->tableWidget->setCellWidget(line, 2, label);

    QPushButton *btn = new QPushButton();
    btn->setText(QStringLiteral("删除"));
    connect(btn, &QPushButton::clicked, this, [=]()
            {
            QPushButton *_sender = qobject_cast<QPushButton*>(sender());
            if (_sender == nullptr)
                {
                    return;
                }
                QModelIndex qIndex = ui->tableWidget->indexAt(QPoint(_sender->frameGeometry().x(), _sender->frameGeometry().y()));
                // 获取当前按钮所在的行和列
                int row = qIndex.row();
//                int column = qIndex.column();
            QString username =  ui->tableWidget->item(row, 1)->text(); // get username
            ui->tableWidget->removeRow(row);
            if(pSqllite->delete_data(db, "userData", username))
        {
            qDebug() << QString("delete %1 success").arg(username);

            QString name = "./IMAGES/" + username + ".jpg";
            QFileInfo FileInfo(name);
            if(FileInfo.isFile()) QFile::remove(name);

            loadUserInfo(db, "userData");

    }
            else{
            qDebug() << QString("delete %1 failed").arg(username);
    }
            line --; });
    ui->tableWidget->setCellWidget(line, 3, btn);
    line++;
}

void MainWindow::slot_VideoTimer()
{
    if (!pCap->isOpened())
    {
        qDebug() << "camera opened failed.";
        return;
    }

    cv::Mat image;
    //    cv::Mat ret_image;
    //    cv::Mat grayimage;
    if (!pCap->read(image))
    {
        qDebug() << " read image is empty";
        return;
    }

#if 1

    //    cv::cvtColor(image, grayimage, CV_BGR2GRAY);
    //    cv::cvtColor(image, ret_image, CV_BGR2RGB);

    IplImage imgTmp = image;
    IplImage *img1 = cvCloneImage(&imgTmp); /* 深拷贝 */

    ASF_MultiFaceInfo detectedFaces = {0};        /* 多人脸信息 */
    ASF_SingleFaceInfo SingleDetectedFaces = {0}; /* 单人脸信息 */
    ASF_FaceFeature feature = {0};                /* 人脸特征 */
    ASF_FaceFeature copyfeature = {0};            /* 人脸特征 */
    static int flag = false;
    /* 4字节对齐 */
    IplImage *cutImg1 = cvCreateImage(cvSize(img1->width - img1->width % 4, img1->height), IPL_DEPTH_8U, img1->nChannels);
    CutIplImage(img1, cutImg1, 0, 0);
    if (img1)
    {
        ASVLOFFSCREEN offscreen = {0};
        offscreen.u32PixelArrayFormat = ASVL_PAF_RGB24_B8G8R8;
        offscreen.i32Width = cutImg1->width;
        offscreen.i32Height = cutImg1->height;
        offscreen.pi32Pitch[0] = cutImg1->widthStep;
        offscreen.ppu8Plane[0] = (MUInt8 *)cutImg1->imageData;
        MRESULT res = MOK;
        res = ASFDetectFacesEx(videoHandle, &offscreen, &detectedFaces);

        if (res == MOK && detectedFaces.faceNum >= 1)
        {
            // 打印人脸检测结果
            for (int i = 0; i < detectedFaces.faceNum; i++)
            {
                cv::Point t1(detectedFaces.faceRect[i].left, detectedFaces.faceRect[i].top);
                cv::Point t2(detectedFaces.faceRect[i].right, detectedFaces.faceRect[i].bottom);
                if (flag)
                {
                    CvScalar green[] = {{{0, 255, 0}}};
                    cv::rectangle(image, t1, t2, green[0], 2);
                }
                else
                {
                    CvScalar red[] = {{{0, 0, 255}}};
                    cv::rectangle(image, t1, t2, red[0], 2);
                }
            }

            SingleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
            SingleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
            SingleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
            SingleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;
            SingleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];

            res = ASFFaceFeatureExtractEx(videoHandle, &offscreen, &SingleDetectedFaces, &feature);

            if (res == MOK)
            {
                /* 第一次提取特征需进行拷贝，因为使用同一个引擎，第二次拷贝会覆盖第一次提取的结果 */
                // 拷贝feature
                copyfeature.featureSize = feature.featureSize;
                copyfeature.feature = (MByte *)malloc(feature.featureSize);
                memset(copyfeature.feature, 0, feature.featureSize);
                memcpy(copyfeature.feature, feature.feature, feature.featureSize);
                qDebug() << "------------------";
                qDebug() << __LINE__ << copyfeature.featureSize;
                MFloat confidenceLevel;
                for (int i = 0; i < feature_list.size(); i++)
                {
                    ASFFaceFeatureCompare(videoHandle, &copyfeature, &feature_list[i], &confidenceLevel);
                    qDebug() << __LINE__ << feature_list.at(i).featureSize << confidenceLevel;

                    if (confidenceLevel > 0.90)
                    {
                        flag = true;
                    }
                    else
                    {
                        flag = false;
                    }
                }
            }
        }
        else
        {
            qDebug("ASFDetectFacesEx failed: %d\n", res);
        }

        //        cvReleaseImage(&img1);
    }
    else
    {
        qDebug() << __LINE__ << "img1 error";
    }
#endif

    QImage q_image = cvMat2QImage(image);
    ui->labelVideo->setPixmap(QPixmap::fromImage(q_image));
}

void MainWindow::slot_VideoRegisterTimer()
{

    if (!pCap->isOpened())
    {
        qDebug() << __LINE__ << "camera opened failed.";
        return;
    }
    cv::Mat image;
    cv::Mat ret_image;
    cv::Mat grayimage;
    if (!pCap->read(image))
    {
        qDebug() << __LINE__ << "The read image is empty";
        return;
    }
    cv::cvtColor(image, grayimage, CV_BGR2GRAY);
    cv::cvtColor(image, ret_image, CV_BGR2RGB);
    IplImage img(grayimage);
    IplImage dspimg(ret_image);
    if (cascadeClassifier != NULL)
    {
        CvScalar FaceCirclecolors[] = {
            {{0, 0, 255}},
            {{0, 128, 255}},
            {{0, 255, 255}},
            {{0, 255, 0}},
            {{255, 128, 0}},
            {{255, 255, 0}},
            {{255, 0, 0}},
            {{255, 0, 255}},
        };

        cvClearMemStorage(memStorage);

        CvSeq *seqFace = cvHaarDetectObjects(&img, cascadeClassifier, memStorage);

        for (int i = 0; i < seqFace->total; i++)
        {

            CvRect *r = (CvRect *)cvGetSeqElem(seqFace, i); /* 利用强制类型转换，转换为序列中实际存储的数据类型 */
            CvPoint center;
            int radius;
            center.x = cvRound((r->x + r->width * 0.5));
            center.y = cvRound((r->y + r->height * 0.5));
            radius = cvRound((r->width + r->height) * 0.25);

            cvCircle(&dspimg, center, radius, FaceCirclecolors[i % 8], 1);
        }

        if (REGISTER_FLAG)
        {
            REGISTER_FLAG = false;
            ASF_FaceFeature m_feature = {0};
            if (getFeatureByVideo(image, m_feature))
            {
                feature_list << m_feature;

                QString name = ui->lineEditName->text() + ".jpg";
                QString path = "./IMAGES/" + name;
                cv::imwrite(path.toStdString(), image);

                int maxid = pSqllite->getMaxID(db, "userData");
                qDebug() << "maxid:" << maxid;
                SQL_DATA_TypeDef sdt;
                sdt.t_id = maxid + 1;
                sdt.t_username = ui->lineEditName->text();
                sdt.t_pl = "_";
                sdt.t_reserve1 = "_";
                sdt.t_reserve2 = "_";
                sdt.t_featuresize = m_feature.featureSize;
                QByteArray featureByte = QByteArray::fromRawData((const char *)m_feature.feature, m_feature.featureSize);
                sdt.t_featuredata = featureByte;
                sdt.t_imgtype = ret_image.type();
                sdt.t_imgcols = ret_image.cols;
                sdt.t_imgrows = ret_image.rows;
                sdt.t_imgsize = ret_image.elemSize();
                const size_t data_size = ret_image.cols * ret_image.rows * ret_image.elemSize();
                QByteArray imgByte = QByteArray::fromRawData((const char *)ret_image.ptr(), data_size);
                sdt.t_imgdata = imgByte;

                if (pSqllite->insert_data_into_table(db, "userData", &sdt))
                {
                    QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("注册成功"));
                    pVideoRegisterTimer->stop();
                    pVideoRegisterTimer->deleteLater();
                    pVideoRegisterTimer = nullptr;
                    pCap->release();
                    SafeDelete(pCap)
                }
                else
                {
                    QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("注册失败"));
                }
            }
            else
            {
                QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("注册失败"));
            }
        }

        QImage q_image((const uchar *)ret_image.data, ret_image.cols, ret_image.rows, QImage::Format_RGB888); /* 截图彩色 */
        ui->labelVideoRegister->setPixmap(QPixmap::fromImage(q_image));
    }
}

void MainWindow::slot_DeleteRow(int, int)
{
}

void MainWindow::on_btnOpenVideo_clicked()
{
    if (ui->btnOpenVideo->text() == QStringLiteral("打开视频"))
    {
        pCap = new VideoCapture;
        pCap->open(0);
        pCap->set(CV_CAP_PROP_FRAME_WIDTH, ui->labelVideo->width());
        pCap->set(CV_CAP_PROP_FRAME_HEIGHT, ui->labelVideo->height());
        if (!pCap->isOpened())
        {
            qDebug() << __LINE__ << "camera0 open failed.";
            return;
        }
        ui->btnOpenVideo->setText(QStringLiteral("关闭视频"));
        pVideoTimer = new QTimer(this);
        pVideoTimer->setInterval(150);
        connect(pVideoTimer, &QTimer::timeout, this, &MainWindow::slot_VideoTimer);
        pVideoTimer->start();
    }
    else
    {
        ui->btnOpenVideo->setText(QStringLiteral("打开视频"));
        pVideoTimer->stop();
        pVideoTimer->deleteLater();
        pVideoTimer = nullptr;
        pCap->release();
        delete pCap;
        pCap = nullptr;
    }
}

void MainWindow::on_btnRegiVideo_clicked()
{
    if (pVideoTimer != nullptr && pVideoTimer->isActive())
    {
        qDebug() << __LINE__ << "请先停止识别";
        return;
    }

    pCap = new VideoCapture;
    pCap->open(0);
    pCap->set(CV_CAP_PROP_FRAME_WIDTH, ui->labelVideoRegister->width());
    pCap->set(CV_CAP_PROP_FRAME_HEIGHT, ui->labelVideoRegister->height());
    if (!pCap->isOpened())
    {
        qDebug() << __LINE__ << "camera0 open failed.";
        return;
    }

    memStorage = cvCreateMemStorage(0); /* 创建一个内存存储块，0默认大小64KB */
    cascadeClassifier = new CvHaarClassifierCascade;
    cascadeClassifier = (CvHaarClassifierCascade *)cvLoad("./haarcascade_frontalface_alt2.xml");

    pVideoRegisterTimer = new QTimer(this);
    pVideoRegisterTimer->setInterval(100);
    connect(pVideoRegisterTimer, &QTimer::timeout, this, &MainWindow::slot_VideoRegisterTimer);
    pVideoRegisterTimer->start();

    RegisterTypeFlag = 2; // 视频注册
}

void MainWindow::on_btnRegiImage_clicked()
{
    if (pVideoTimer != nullptr && pVideoTimer->isActive())
        pVideoTimer->stop();

    ui->labelVideoRegister->clear();

    QString imagefilepath = QFileDialog::getOpenFileName(this, "select register image", "../", "IMAGE(*.png *.jpg)");
    if (imagefilepath.isEmpty())
        return;
    mImagePath = imagefilepath;
    qDebug() << mImagePath;
    QImage image;
    image.load(imagefilepath);
    ui->labelVideoRegister->setPixmap(QPixmap::fromImage(image.scaled(ui->labelVideoRegister->width(),
                                                                      ui->labelVideoRegister->height(),
                                                                      Qt::KeepAspectRatio)));

    RegisterTypeFlag = 1; // 图片注册
}

void MainWindow::on_btnConfirmRegister_clicked()
{
    QString id = ui->lineEditName->text();
    if (id.isEmpty())
    {
        qDebug() << "请输入名称";
        return;
    }
    ASF_FaceFeature img_feature = {0};
    SQL_DATA_TypeDef sdt;

    if (RegisterTypeFlag == 1)
    {
        if (!getFeatureByImage(mImagePath, img_feature))
        {
            qDebug() << __LINE__ << "get feature failed";
            return;
        }

        qDebug() << __LINE__ << "get feature success";

        int maxid = pSqllite->getMaxID(db, "userData");

        feature_list << img_feature;
        sdt.t_id = maxid + 1;
        sdt.t_username = ui->lineEditName->text();
        sdt.t_pl = "_";
        sdt.t_reserve1 = "_";
        sdt.t_reserve2 = "_";

        sdt.t_featuresize = img_feature.featureSize;
        sdt.t_featuredata = QByteArray(reinterpret_cast<char *>(img_feature.feature), static_cast<int>(img_feature.featureSize));

        //        sdt.t_featuredata = QByteArray::fromRawData((const char *)img_feature.feature, img_feature.featureSize);

        cv::Mat img = cv::imread(mImagePath.toStdString());

        qDebug() << img.cols << img.rows;
        sdt.t_imgtype = img.type();
        sdt.t_imgcols = img.cols;
        sdt.t_imgrows = img.rows;
        sdt.t_imgsize = img.elemSize();
        const size_t data_size = img.cols * img.rows * img.elemSize();
        QByteArray imgByte = QByteArray::fromRawData((const char *)img.ptr(), data_size);
        sdt.t_imgdata = imgByte;

        if (pSqllite->insert_data_into_table(db, "userData", &sdt))
        {
            //                                   QString name = mImagePath.split('/').back();
            QString name = ui->lineEditName->text() + ".jpg";
            QString path = "./IMAGES/" + name;
            cv::imwrite(path.toStdString(), img);
            QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("注册成功"));
        }

        else
            QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("注册失败"));
    }
    else if (RegisterTypeFlag == 2)
    {
        REGISTER_FLAG = true;
    }
}

void MainWindow::on_btnRefresh_clicked()
{
    ui->tableWidget->setRowCount(0);
    line = 0;
    this->loadUserInfo(db, "userData");
    for (int i = 0; i < username_list.size(); i++)
    {
        qDebug() << __LINE__ << username_list.at(i);
        QImage image = cvMat2QImage(image_list.at(i));
        insertOneRowInTable(i, username_list.at(i), image);
    }
}

void MainWindow::on_btnFind_clicked()
{
    QString name = ui->lineEdit->text();

    SQL_DATA_TypeDef sdt;

    if (pSqllite->getUserInfoByUsername(db, "userData", name, sdt))
    {
        if (sdt.t_imgsize == 0)
        {
            QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("未查找到该人员信息"));
            return;
        }
        cv::Mat ret_image(sdt.t_imgrows, sdt.t_imgcols, sdt.t_imgtype, (void *)sdt.t_imgdata.data());

        QImage image = cvMat2QImage(ret_image);
        ui->tableWidget->setRowCount(0);
        line = 0;
        insertOneRowInTable(0, name, image);
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("未查找到该人员信息"));
        return;
    }
}
