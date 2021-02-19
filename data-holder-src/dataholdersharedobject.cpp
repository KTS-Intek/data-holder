#include "dataholdersharedobject.h"

DataHolderSharedObject::DataHolderSharedObject(QObject *parent) : QObject(parent)
{
    makeDataHolderTypesRegistration();
}

//----------------------------------------------------------------------------

DHDataTable DataHolderSharedObject::getDataTable()
{

    QReadLocker locker(&myLock);
    const DHDataTable r = dataTable;
    return r ;
}

//----------------------------------------------------------------------------

DHNI2data DataHolderSharedObject::getPollCodeData(const quint16 &pollCode)
{
    QReadLocker locker(&myLock);
    const DHNI2data r = dataTable.value(pollCode);
    return r ;
}

//----------------------------------------------------------------------------

DHMsecRecord DataHolderSharedObject::getLastRecord(const quint16 &pollCode, const QString &ni)
{
    QReadLocker locker(&myLock);
    const DHMsecRecord r = dataTable.value(pollCode).value(ni);
    return r ;
}

//----------------------------------------------------------------------------

void DataHolderSharedObject::addRecord(quint16 pollCode, QString ni, qint64 msec, QVariantHash hash, QString srcname)
{
    if(pollCode == 0 )
        return;

    if(ni.isEmpty() || hash.isEmpty())
        return;

    const DHMsecRecord oldrecord = getLastRecord(pollCode, ni);

    bool sayThatChanged = false;
    if(msec > oldrecord.msec){
     //newer




        QWriteLocker locker(&myLock);
        DHNI2data pollCodeTable = dataTable.value(pollCode);
        pollCodeTable.insert(ni, DHMsecRecord(msec, hash, srcname, false));
        dataTable.insert(pollCode, pollCodeTable);
        sayThatChanged = true;

    }

    if(sayThatChanged)
        emit onDataTableChanged();



}
//----------------------------------------------------------------------------


void DataHolderSharedObject::addRestoredRecords(quint16 pollCode, QStringList nis, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames)
{
    //use this method only for data restoring from the shared memory
    if(pollCode == 0 )
        return;

    if(nis.isEmpty() || hashs.isEmpty())
        return;

    DHNI2data pollCodeTable = dataTable.value(pollCode);
    bool sayThatChanged = false;

    if(!nis.isEmpty()){
        QWriteLocker locker(&myLock);

        for(int i = 0, imax = nis.size(); i < imax; i++){
            const QString ni = nis.at(i);
            const DHMsecRecord oldrecord = pollCodeTable.value(ni);
            const qint64 msec = msecs.at(i);
            if(msec > oldrecord.msec){
                pollCodeTable.insert(ni, DHMsecRecord(msec, hashs.at(i), srcnames.at(i), true));
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
