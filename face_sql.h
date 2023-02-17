#ifndef FACE_SQL_H
#define FACE_SQL_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include "submain.h"


class FACE_SQL : public QObject
{
    Q_OBJECT
public:
    explicit FACE_SQL(QObject *parent = nullptr);

    QSqlDatabase create_db(QString fileName, QString identifier = "dd");
    bool open_db(QSqlDatabase db);
    bool isExistTable(QSqlDatabase db, QString table);
    bool create_table(QSqlDatabase db, QString sql_command);
    bool insert_data_into_table(QSqlDatabase db, QString table, SQL_DATA_TypeDef* data_t);
    bool delete_data(QSqlDatabase db, QString table,QString username);
    bool get_userInfo(QSqlDatabase db, QString table, int id, SQL_DATA_TypeDef& data_t);
    bool update_userInfo(QSqlDatabase db, QString table, int id, QString col_name, QString new_value);
    void close_db(QSqlDatabase db);
    int getAllUsername(QSqlDatabase db, QString table, QStringList &usernameList);
    bool getUserInfoByUsername(QSqlDatabase db, QString table, QString username, SQL_DATA_TypeDef &data_t);
    int getMaxID(QSqlDatabase db, QString table);
    int getRows(QSqlDatabase db, QString table);
/* "CREATE TABLE facedata(faceid int unsigned primary key, username varchar(40) not null, featuresize int not null,  )" */
signals:



};

#endif // FACE_SQL_H
