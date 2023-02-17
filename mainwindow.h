#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "submain.h"
#include "face_sql.h"
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

    int systemInit(QString app_id, QString sdk_key);
    int sqlInit();
    bool loadUserInfo(QSqlDatabase db, QString table);
    void CutIplImage(IplImage* src, IplImage* dst, int x, int y);
    QImage cvMat2QImage(const Mat &mat);
    Mat QImage2cvMat(QImage &image);
    bool getFeatureByVideo(cv::Mat image, ASF_FaceFeature& feature);
    bool getFeatureByImage(QString img_path, ASF_FaceFeature& feature);
    QStringList getImageFilePath(QString dirpath = "./IMAGES/");
    void getImagesFeature(QString dirpath,QList<ASF_FaceFeature>& list);
    void tablewidgetInit(void);
    void insertOneRowInTable(int id, QString username, QImage image);

private slots:

    void slot_VideoTimer(void);
    void slot_VideoRegisterTimer(void);
    void slot_DeleteRow(int,int);
    void on_btnOpenVideo_clicked();

    void on_btnRegiVideo_clicked();

    void on_btnRegiImage_clicked();

    void on_btnConfirmRegister_clicked();

    void on_btnRefresh_clicked();

    void on_btnFind_clicked();


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
};

#endif // MAINWINDOW_H
