#include "face_sql.h"

FACE_SQL::FACE_SQL(QObject *parent) : QObject(parent)
{
}

QSqlDatabase FACE_SQL::create_db(QString fileName, QString identifier)
{
    QSqlDatabase db;
    QString name = QCoreApplication::applicationDirPath() + "/" + fileName + ".db";
        db = QSqlDatabase::addDatabase("QSQLITE", identifier);
        db.setDatabaseName(name);
    return db;
}

bool FACE_SQL::open_db(QSqlDatabase db)
{
    return db.open();
}

bool FACE_SQL::isExistTable(QSqlDatabase db, QString table)
{
    if (!open_db(db)) {
        return false;
    }
    QSqlQuery query(db);
    query.exec(QString("select count(*) from sqlite_master where type='table' and name='%1'").arg(table));    //关键的判断
    if (query.next())
    {
        if (query.value(0).toInt() == 0)
        {
            return false; /* not exist */
        }
    }
    return true;
}

bool FACE_SQL::create_table(QSqlDatabase db, QString table)
{
    if(!open_db(db)){
        qDebug() << "open failed";
        return false;
    }
    QSqlQuery sqlquery(db);
    return sqlquery.exec(table);
}

bool FACE_SQL::insert_data_into_table(QSqlDatabase db, QString table, SQL_DATA_TypeDef *data_t)
{
    QSqlQuery sqlquery(db);
    /* 此处判断唯一性 */
    QString command = QString("INSERT INTO %1 (id,username,pl,reserve1,reserve2,featuresize,featuredata,imgtype,imgcols,imgrows,imgsize,imgdata) VALUES (:id,:username,:pl,:reserve1,:reserve2,:featuresize,:featuredata,:imgtype,:imgcols,:imgrows,:imgsize,:imgdata);").arg(table);
    sqlquery.prepare(command);
    sqlquery.bindValue(":id", data_t->t_id);
    sqlquery.bindValue(":username", data_t->t_username);
    sqlquery.bindValue(":pl", data_t->t_pl);
    sqlquery.bindValue(":reserve1", data_t->t_reserve1);
    sqlquery.bindValue(":reserve2", data_t->t_reserve2);
    sqlquery.bindValue(":featuresize", data_t->t_featuresize);
    sqlquery.bindValue(":featuredata", data_t->t_featuredata);
    sqlquery.bindValue(":imgtype", data_t->t_imgtype);
    sqlquery.bindValue(":imgcols", data_t->t_imgcols);
    sqlquery.bindValue(":imgrows", data_t->t_imgrows);
    sqlquery.bindValue(":imgsize", data_t->t_imgsize);
    sqlquery.bindValue(":imgdata", data_t->t_imgdata);

    if(sqlquery.exec()) return true;
    else {
        qDebug() << sqlquery.lastError().text();
        return false;
    }

}

bool FACE_SQL::delete_data(QSqlDatabase db, QString table, QString username)
{
    QSqlQuery sqlquery(db);
    QString delete_sql = QString("delete from %1 where username = \"%2\"").arg(table).arg(username);
    if(sqlquery.exec(delete_sql)) return true;
    else
    {
        qDebug() << sqlquery.lastError().text();
        return false;
    }
}

bool FACE_SQL::get_userInfo(QSqlDatabase db, QString table, int id, SQL_DATA_TypeDef &data_t)
{
    QSqlQuery sqlquery(db);
    QString command = QString("select * from %1 where id = \"%2\";").arg(table).arg(id);
    if(!sqlquery.exec(command)) {
        qDebug() << sqlquery.lastError().text();
        return false;
    }
    else {
        sqlquery.next();
        data_t.t_id = sqlquery.value(0).toInt();
        data_t.t_username = sqlquery.value(1).toString();
        data_t.t_pl = sqlquery.value(2).toString();
        data_t.t_reserve1 = sqlquery.value(3).toString();
        data_t.t_reserve2 = sqlquery.value(4).toString();
        data_t.t_featuresize = sqlquery.value(5).toInt();
        data_t.t_featuredata = sqlquery.value(6).toByteArray();
        data_t.t_imgtype = sqlquery.value(7).toInt();
        data_t.t_imgcols = sqlquery.value(8).toInt();
        data_t.t_imgrows = sqlquery.value(9).toInt();
        data_t.t_imgsize = sqlquery.value(10).toUInt();
        QByteArray data2 = sqlquery.value(11).toByteArray();
        data_t.t_imgdata = data2;

        return true;
    }
}

bool FACE_SQL::update_userInfo(QSqlDatabase db, QString table, int id, QString col_name, QString new_value)
{
    QSqlQuery sqlquery(db);
    QString command = QString("update %1 set %2 = %3 where id = :id;").arg(table)
                                                                    .arg(col_name)
                                                                    .arg(new_value);
    sqlquery.bindValue(":id", id);
    if(!sqlquery.exec()) {
        return false;
    }
    else {
        return true;
    }

}

void FACE_SQL::close_db(QSqlDatabase db)
{
    if(open_db(db)) db.close();
}

int FACE_SQL::getAllUsername(QSqlDatabase db, QString table, QStringList &usernameList)
{
    QSqlQuery sqlquery(db);
    QString command = QString("select username from \"%1\"").arg(table);

    sqlquery.prepare(command);
    if (!sqlquery.exec())
    {
        qDebug() << sqlquery.lastError();
    }
    else
    {
        while (sqlquery.next())
        {
            QString username = sqlquery.value(0).toString();
            usernameList << username;
        }
    }
    return usernameList.length();
}

bool FACE_SQL::getUserInfoByUsername(QSqlDatabase db, QString table, QString username, SQL_DATA_TypeDef &data_t)
{
    QSqlQuery sqlquery(db);
    QString command = QString("select * from %1 where username = \"%2\";").arg(table).arg(username);
    qDebug() << command;
    if(!sqlquery.exec(command)) {
        qDebug() << sqlquery.lastError().text();
        return false;
    }
    else {
        sqlquery.next();
        data_t.t_id = sqlquery.value(0).toInt();
        data_t.t_username = sqlquery.value(1).toString();
        data_t.t_pl = sqlquery.value(2).toString();
        data_t.t_reserve1 = sqlquery.value(3).toString();
        data_t.t_reserve2 = sqlquery.value(4).toString();
        data_t.t_featuresize = sqlquery.value(5).toInt();
        data_t.t_featuredata = sqlquery.value(6).toByteArray();
        data_t.t_imgtype = sqlquery.value(7).toInt();
        data_t.t_imgcols = sqlquery.value(8).toInt();
        data_t.t_imgrows = sqlquery.value(9).toInt();
        data_t.t_imgsize = sqlquery.value(10).toUInt();
        data_t.t_imgdata = sqlquery.value(11).toByteArray();

        return true;
    }

}

int FACE_SQL::getMaxID(QSqlDatabase db, QString table)
{
    QSqlQuery sqlquery(db);
    QString command = QString("select max(id) from \"%1\"").arg(table);
    int max_id = 0;
    sqlquery.prepare(command);
    if (!sqlquery.exec())
    {
        qDebug() << sqlquery.lastError();
    }
    else
    {
        while (sqlquery.next())
        {
            max_id = sqlquery.value(0).toInt();

        }
    }
    return max_id;
}

int FACE_SQL::getRows(QSqlDatabase db, QString table)
{
    QSqlQuery sqlquery(db);
    QString command = QString("select count(*) from \"%1\"").arg(table);
    int count = 0;
    sqlquery.prepare(command);
    if (!sqlquery.exec())
    {
        qDebug() << sqlquery.lastError();
    }
    else
    {
        while (sqlquery.next())
        {
            count = sqlquery.value(0).toInt();

        }
    }
    return count;
}


