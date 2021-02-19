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

    DHDataTable getDataTable() ;

    DHNI2data getPollCodeData(const quint16 &pollCode);

    DHMsecRecord getLastRecord(const quint16 &pollCode, const QString &ni);



signals:
    void onDataTableChanged();


public slots:
    void addRecord(quint16 pollCode, QString ni, qint64 msec, QVariantHash hash, QString srcname);

    void addRestoredRecords(quint16 pollCode, QStringList nis, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames);


private:
    void makeDataHolderTypesRegistration();




    QReadWriteLock myLock;
    DHDataTable dataTable;
};

//must be in a header file, outside the class!!!

QDataStream &operator <<(QDataStream &out, const DHMsecRecord &m);
QDataStream &operator >>(QDataStream &in, DHMsecRecord &m);
QDebug operator<<(QDebug d, const DHMsecRecord &m);


#endif // DATAHOLDERSHAREDOBJECT_H
