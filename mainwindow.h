#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QButtonGroup>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextToSpeech>

#include "submain.h"
#include "face_sql.h"

#define IMAGE_HEIGHT   100

using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int systemInit(QString app_id, QString sdk_key);/* 系统初始化 */
    int sqlInit(); /* 数据库初始化 */
    void tablewidgetInit(void); /* 管理界面初始化 */
    void serialportInit(void);  /* 串口初始化 */
    void speechInit(void);

    bool loadUserInfo(QSqlDatabase db, QString table); /* 加载用户信息 */
    void CutIplImage(IplImage* src, IplImage* dst, int x, int y);
    QImage cvMat2QImage(const Mat &mat); /* mat 转 QImage */
    Mat QImage2cvMat(QImage &image); /* QImage 转 mat  */
    bool getFeatureByVideo(cv::Mat image, ASF_FaceFeature& feature);/* 从视频中获取人脸特征 */
    bool getFeatureByImage(QString img_path, ASF_FaceFeature& feature); /* 从图片中获取人脸特征 */
    void getImagesFeature(QString dirpath,QList<ASF_FaceFeature>& list); /* 从文件夹里面获取特征列表 */

    QStringList getImageFilePath(QString dirpath = "./IMAGES/"); /* 从文件夹中获取图片列表 */
    void insertOneRowInTable(int id, QString username, QImage image); /* 在tablewidget中插入一个人脸信息 */



private slots:

    void slot_VideoTimer(void);
    void slot_VideoRegisterTimer(void);
    void slotSerialReadyRead(void);

    void on_btnOpenVideo_clicked(); /* 识别界面上的打开视频按钮函数 */

    void on_btnRegiVideo_clicked(); /* 视频注册按钮函数 */

    void on_btnRegiImage_clicked(); /* 图片注册按钮函数 */

    void on_btnConfirmRegister_clicked(); /* 确认注册按钮函数 */

    void on_btnRefresh_clicked(); /* 刷新按钮函数 */

    void on_btnFind_clicked(); /* 查找按钮函数 */


    void on_btnOpenSerial_clicked(); /* 打开串口 */

private:
    Ui::MainWindow *ui;

    CvHaarClassifierCascade *cascadeClassifier; // 人脸识别模型
    CvMemStorage *memStorage;
    double similarity; // 相似度

    MHandle imageHandle = NULL; /* 引擎句柄 */
    MHandle videoHandle = NULL; /* 引擎句柄 */

    cv::VideoCapture *pCap = nullptr; /* 创建一个视频对象 */

    QTimer *pVideoTimer = nullptr;
    FACE_SQL *pSqllite;
    QSqlDatabase db;


    bool VideoRegisterFlag = false;
    QString mImagePath;
    QTimer *pVideoRegisterTimer = nullptr;
    
    int RegisterTypeFlag = -1; 
    bool REGISTER_FLAG = false;

    QList<ASF_FaceFeature> feature_list; /* 声明特征集合 */
    QList<cv::Mat> image_list;
    QStringList username_list;

    int line = 0;
    int user_id = 0;


    QStringList files_path;


    QSerialPort *pSerial;
    QTextToSpeech *pSpeech;

    bool m_Recog_Flag = false;
    bool temp_flag = false;
};

#endif // MAINWINDOW_H
