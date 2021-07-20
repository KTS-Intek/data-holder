#ifndef DATAHOLDERSHAREDOBJECT_H
#define DATAHOLDERSHAREDOBJECT_H

#include <QObject>
#include <QDebug>
#include <QDataStream>
#include <QReadWriteLock>

#include "dataholdertypes.h"

class DataHolderSharedObject : public QObject
{
    Q_OBJECT
public:
    explicit DataHolderSharedObject(QObject *parent = nullptr);

    DHDataTable getDataTableNI() ;
    DHDataTable getDataTableSN() ;

    DHDevId2data getPollCodeDataNI(const quint16 &pollCode);

    DHMsecRecord getLastRecordNI(const quint16 &pollCode, const QString &devID);

    DHMsecRecordList getLastRecordsNI(const quint16 &pollCode, const QString &devID);


    DHDevId2data getPollCodeDataSN(const quint16 &pollCode);

    DHMsecRecord getLastRecordSN(const quint16 &pollCode, const QString &devID);

    DHMsecRecordList getLastRecordsSN(const quint16 &pollCode, const QString &devID);

    bool isItAPulseMeterNextChannel(const qint64 &msec, const quint16 &pollCode, const QString &additionalID, const DHMsecRecord &oldrecord);

    bool isItAPulseMeterPollCode(const quint16 &pollCode);


signals:
    void onDataTableChanged();


public slots:
    void addRecord(quint16 pollCode, QString devID, QString additionalID, qint64 msec, QVariantHash hash, QString srcname);



    void addRestoredRecordsNI(quint16 pollCode, QStringList nis, QStringList sns, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames);
    void addRestoredRecordsSN(quint16 pollCode, QStringList sns, QStringList nis, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames);



private:
    bool addAPulseMeterRecord(const quint16 &pollCode, const QString &devID, const QString &additionalID, const qint64 &msec, const QVariantHash &hash, const QString &srcname);

    bool checkAddThisRecord(DHMsecRecordList &oldrecords, const qint64 &msec, const QString &chnnl);


    void makeDataHolderTypesRegistration();

    bool checkAddRestoredRecords(DHDevId2data &pollCodeTable, const quint16 &pollCode, const QStringList &devIDs, const QStringList &additionalIDs, const QList<qint64> &msecs, const QList<QVariantHash> &hashs, const QStringList &srcnames);




    QReadWriteLock myLock;
    DHDataTable dataTableNI, dataTableSN;
};

//must be in a header file, outside the class!!!

QDataStream &operator <<(QDataStream &out, const DHMsecRecord &m);
QDataStream &operator >>(QDataStream &in, DHMsecRecord &m);
QDebug operator<<(QDebug d, const DHMsecRecord &m);


#endif // DATAHOLDERSHAREDOBJECT_H
