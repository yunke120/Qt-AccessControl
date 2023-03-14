#ifndef SUBMAIN_H
#define SUBMAIN_H

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDateTime>

#include <opencv2/core/types_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <arcface/inc/amcomdef.h>
#include <arcface/inc/arcsoft_face_sdk.h>
#include <arcface/inc/asvloffscreen.h>
#include <arcface/inc/merror.h>
#include <direct.h>
#include <iostream>
#include <stdarg.h>
#include <string>
#include <crtdefs.h>
#include <windows.h>
#include <time.h>

#define APPID "kf4Er8xXdzWCD97fS44ghb6Lg9MMDtzPbqftjacyuM2"
#define SDKKEY "BHwSM2pzFzcHUE8ZT9EfuSJENVLcHgHXGq3q6bEEmnkb"

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; }
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; }


typedef struct SQL_DATA{
    int t_id;
    QString t_username;
    QString t_pl;
    QString t_reserve1;
    QString t_reserve2;
    int t_featuresize;
    QByteArray t_featuredata;
    int t_imgtype;
    int t_imgcols;
    int t_imgrows;
    unsigned int t_imgsize;
    QByteArray t_imgdata;
//    unsigned char* t_imgdata;
//    QDateTime t_datetime;
} SQL_DATA_TypeDef;




#endif // SUBMAIN_H
