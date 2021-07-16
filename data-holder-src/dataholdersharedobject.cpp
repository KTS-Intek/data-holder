#include "dataholdersharedobject.h"

DataHolderSharedObject::DataHolderSharedObject(QObject *parent) : QObject(parent)
{
    makeDataHolderTypesRegistration();
}

//----------------------------------------------------------------------------

DHDataTable DataHolderSharedObject::getDataTableNI()
{

    QReadLocker locker(&myLock);
    const auto r = dataTableNI;
    return r ;
}

//----------------------------------------------------------------------------

DHDataTable DataHolderSharedObject::getDataTableSN()
{
    QReadLocker locker(&myLock);
    const auto r = dataTableSN;
    return r ;
}

//----------------------------------------------------------------------------

DHDevId2data DataHolderSharedObject::getPollCodeDataNI(const quint16 &pollCode)
{
    QReadLocker locker(&myLock);
    const auto r = dataTableNI.value(pollCode);
    return r ;
}

//----------------------------------------------------------------------------

DHMsecRecord DataHolderSharedObject::getLastRecordNI(const quint16 &pollCode, const QString &devID)
{
    QReadLocker locker(&myLock);
    const DHMsecRecord r = dataTableNI.value(pollCode).value(devID);
    return r ;
}

//----------------------------------------------------------------------------

DHDevId2data DataHolderSharedObject::getPollCodeDataSN(const quint16 &pollCode)
{
    QReadLocker locker(&myLock);
    const auto r = dataTableSN.value(pollCode);
    return r ;
}

//----------------------------------------------------------------------------

DHMsecRecord DataHolderSharedObject::getLastRecordSN(const quint16 &pollCode, const QString &devID)
{
    QReadLocker locker(&myLock);
    const DHMsecRecord r = dataTableSN.value(pollCode).value(devID);
    return r ;
}

//----------------------------------------------------------------------------

void DataHolderSharedObject::addRecord(quint16 pollCode, QString devID, QString additionalID, qint64 msec, QVariantHash hash, QString srcname)
{
    if(pollCode == 0 )
        return;

    if(hash.isEmpty())
        return;

    bool sayThatChanged = false;

    if(!devID.isEmpty()){
        const DHMsecRecord oldrecord = getLastRecordNI(pollCode, devID);

        if(msec > oldrecord.msec){
         //newer

            QWriteLocker locker(&myLock);
            auto pollCodeTable = dataTableNI.value(pollCode);
            pollCodeTable.insert(devID, DHMsecRecord(msec, additionalID, hash, srcname, false));
            dataTableNI.insert(pollCode, pollCodeTable);
            sayThatChanged = true;

        }

    }

    if(!additionalID.isEmpty()){
        const DHMsecRecord oldrecord = getLastRecordSN(pollCode, additionalID);

        if(msec > oldrecord.msec){
         //newer

            QWriteLocker locker(&myLock);
            auto pollCodeTable = dataTableSN.value(pollCode);
            pollCodeTable.insert(additionalID, DHMsecRecord(msec, devID,  hash, srcname, false));
            dataTableSN.insert(pollCode, pollCodeTable);
            sayThatChanged = true;

        }

    }

    if(sayThatChanged)
        emit onDataTableChanged();



}

void DataHolderSharedObject::addRestoredRecordsNI(quint16 pollCode, QStringList nis, QStringList sns, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames)
{


    //use this method only for data restoring from the shared memory
    if(pollCode == 0 )
        return;


    //sns can contain empty rows, but it can't be empty
    if(nis.isEmpty() || sns.isEmpty() || hashs.isEmpty())
        return;

    DHDevId2data pollCodeTableNI = getPollCodeDataNI(pollCode);

    bool sayThatChanged = false;

    if(!nis.isEmpty()){
        QWriteLocker locker(&myLock);

        if(checkAddRestoredRecords(pollCodeTableNI, nis, sns, msecs, hashs, srcnames)){
            dataTableNI.insert(pollCode, pollCodeTableNI);
            sayThatChanged = true;
        }

    }
    if(sayThatChanged)
        emit onDataTableChanged();

}

void DataHolderSharedObject::addRestoredRecordsSN(quint16 pollCode, QStringList sns, QStringList nis, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames)
{
    //use this method only for data restoring from the shared memory
    if(pollCode == 0 )
        return;

    if(nis.isEmpty() || sns.isEmpty() || hashs.isEmpty())
        return;

    DHDevId2data pollCodeTableSN = getPollCodeDataSN(pollCode);

    bool sayThatChanged = false;

    if(!sns.isEmpty()){
        QWriteLocker locker(&myLock);

        if(checkAddRestoredRecords(pollCodeTableSN, sns, nis, msecs, hashs, srcnames)){
            dataTableSN.insert(pollCode, pollCodeTableSN);
            sayThatChanged = true;
        }

    }
    if(sayThatChanged)
        emit onDataTableChanged();
}

//----------------------------------------------------------------------------

void DataHolderSharedObject::makeDataHolderTypesRegistration()
{
    //call this function once, before using this types
    if(!QMetaType::isRegistered(QMetaType::type("DHMsecRecord"))){
        qRegisterMetaTypeStreamOperators<DHMsecRecord>("DHMsecRecord");

    }
}

bool DataHolderSharedObject::checkAddRestoredRecords(DHDevId2data &pollCodeTable, const QStringList &devIDs, const QStringList &additionalIDs, const QList<qint64> &msecs, const QList<QVariantHash> &hashs, const QStringList &srcnames)
{

    bool sayThatChanged = false;
    for(int i = 0, imax = devIDs.size(); i < imax; i++){

        const QString devID = devIDs.at(i);

        if(devID.isEmpty())
            continue;

        const DHMsecRecord oldrecord = pollCodeTable.value(devID);
        const qint64 msec = msecs.at(i);
        if(msec > oldrecord.msec){
            pollCodeTable.insert(devID, DHMsecRecord(msec, additionalIDs.at(i), hashs.at(i), srcnames.at(i), true));
            sayThatChanged = true;
        }

    }
    return sayThatChanged;
}


//----------------------------------------------------------------------------
//qint64 msec;
//QString additionalID;

//QVariantHash hash;
//QString srcname;
//bool wasRestored;

//must be in a cpp file!!!
QDataStream &operator <<(QDataStream &out, const DHMsecRecord &m)
{
    return out << m.msec << m.additionalID << m.hash << m.srcname << m.wasRestored;
}

QDataStream &operator >>(QDataStream &in, DHMsecRecord &m)
{
    in >> m.msec >> m.additionalID >> m.hash >> m.srcname >> m.wasRestored;
    return in;
}

QDebug operator<<(QDebug d, const DHMsecRecord &m)
{
    d << m.msec << m.additionalID << m.hash << m.srcname << m.wasRestored;
    return d;
}
