#include "dataholdersharedobject.h"

DataHolderSharedObject::DataHolderSharedObject(QObject *parent) : QObject(parent)
{
    makeDataHolderTypesRegistration();
}

//----------------------------------------------------------------------------

DHDataTable DataHolderSharedObject::getDataTable()
{

    QReadLocker locker(&myLock);
    const auto r = dataTable;
    return r ;
}

//----------------------------------------------------------------------------

DHDevId2data DataHolderSharedObject::getPollCodeData(const quint16 &pollCode)
{
    QReadLocker locker(&myLock);
    const auto r = dataTable.value(pollCode);
    return r ;
}

//----------------------------------------------------------------------------

DHMsecRecord DataHolderSharedObject::getLastRecord(const quint16 &pollCode, const QString &devID)
{
    QReadLocker locker(&myLock);
    const DHMsecRecord r = dataTable.value(pollCode).value(devID);
    return r ;
}

//----------------------------------------------------------------------------

void DataHolderSharedObject::addRecord(quint16 pollCode, QString devID, qint64 msec, QVariantHash hash, QString srcname)
{
    if(pollCode == 0 )
        return;

    if(devID.isEmpty() || hash.isEmpty())
        return;

    const DHMsecRecord oldrecord = getLastRecord(pollCode, devID);

    bool sayThatChanged = false;
    if(msec > oldrecord.msec){
     //newer

        QWriteLocker locker(&myLock);
        auto pollCodeTable = dataTable.value(pollCode);
        pollCodeTable.insert(devID, DHMsecRecord(msec, hash, srcname, false));
        dataTable.insert(pollCode, pollCodeTable);
        sayThatChanged = true;

    }

    if(sayThatChanged)
        emit onDataTableChanged();



}
//----------------------------------------------------------------------------


void DataHolderSharedObject::addRestoredRecords(quint16 pollCode, QStringList devIDs, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames)
{
    //use this method only for data restoring from the shared memory
    if(pollCode == 0 )
        return;

    if(devIDs.isEmpty() || hashs.isEmpty())
        return;

    DHDevId2data pollCodeTable = dataTable.value(pollCode);
    bool sayThatChanged = false;

    if(!devIDs.isEmpty()){
        QWriteLocker locker(&myLock);

        for(int i = 0, imax = devIDs.size(); i < imax; i++){
            const QString devID = devIDs.at(i);
            const DHMsecRecord oldrecord = pollCodeTable.value(devID);
            const qint64 msec = msecs.at(i);
            if(msec > oldrecord.msec){
                pollCodeTable.insert(devID, DHMsecRecord(msec, hashs.at(i), srcnames.at(i), true));
                sayThatChanged = true;
            }



        }
        if(sayThatChanged)
            dataTable.insert(pollCode, pollCodeTable);

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


//----------------------------------------------------------------------------

//must be in a cpp file!!!
QDataStream &operator <<(QDataStream &out, const DHMsecRecord &m)
{
    return out << m.msec << m.hash;
}

QDataStream &operator >>(QDataStream &in, DHMsecRecord &m)
{
    in >> m.msec >> m.hash;
    return in;
}

QDebug operator<<(QDebug d, const DHMsecRecord &m)
{
    d << m.msec << m.hash;
    return d;
}
