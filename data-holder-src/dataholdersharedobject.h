#ifndef DATAHOLDERSHAREDOBJECT_H
#define DATAHOLDERSHAREDOBJECT_H

#include <QObject>
#include <QDebug>
#include <QDataStream>
#include <QReadWriteLock>


#include "dataholdersharedobjectprocessor.h"

class DataHolderSharedObject : public QObject
{
    Q_OBJECT
public:
    explicit DataHolderSharedObject(const bool &verboseMode, QObject *parent = nullptr);

    bool verboseMode;



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

    void setEventManagerRules(QVariantHash hashRules);


    void sendCommand2pollDevStr(quint16 pollCode, QString args);

    void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);

    void onThisCommandFailed(QString ruleNameId, QString counterId);


    //to iterator
    void setThisDevIDData(QString devID, quint16 pollCode, QString dataKey, QString value);

    void setThisAdditionalDevIDData(QString devID, QString additionalDevID, quint16 pollCode, QString dataKey, QString value);


    void append2log(QString message);

public slots:
    void createDataProcessor();

    void addRecord(quint16 pollCode, QString devID, QString additionalID, qint64 msec, QVariantHash hash, QString srcname);



    void addRestoredRecordsNI(quint16 pollCode, QStringList nis, QStringList sns, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames);
    void addRestoredRecordsSN(quint16 pollCode, QStringList sns, QStringList nis, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames);


    //from iterator
   void gimmeThisDevIDData(QString devID, quint16 pollCode, QString dataKey);

   void gimmeThisAdditionalDevIDData(QString devID, QString additionalDevID, quint16 pollCode, QString dataKey);



private:
    DHMsecRecordList addAPulseMeterRecord(const quint16 &pollCode, const QString &devID, const QString &additionalID, const qint64 &msec, const QVariantHash &hash, const QString &srcname);

    bool checkAddThisRecord(DHMsecRecordList &oldrecords, const qint64 &msec, const QString &chnnl);

    bool checkAddThisRecordMeters(DHMsecRecordList &oldrecords, const qint64 &msec, const QString &additionalID);

    void makeDataHolderTypesRegistration();

    bool checkAddRestoredRecords(DHDevId2data &pollCodeTable, const quint16 &pollCode, const QStringList &devIDs, const QStringList &additionalIDs, const QList<qint64> &msecs, const QList<QVariantHash> &hashs, const QStringList &srcnames);


    DataHolderSharedObjectProcessor *dataProcessor;

    QReadWriteLock myLock;
    DHDataTable dataTableNI, dataTableSN;
};

//must be in a header file, outside the class!!!

QDataStream &operator <<(QDataStream &out, const DHMsecRecord &m);
QDataStream &operator >>(QDataStream &in, DHMsecRecord &m);
QDebug operator<<(QDebug d, const DHMsecRecord &m);


#endif // DATAHOLDERSHAREDOBJECT_H
