#include "dataholdersharedobject.h"

#include "myucdevicetypes.h"

DataHolderSharedObject::DataHolderSharedObject(const bool &verboseMode, QObject *parent) : QObject(parent)
{
    this->verboseMode = verboseMode;

    makeDataHolderTypesRegistration();

    createDataProcessor();
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
    const auto r = dataTableNI.value(pollCode).value(devID);
    return r ;
}
//----------------------------------------------------------------------------
DHMsecRecordList DataHolderSharedObject::getLastRecordsNI(const quint16 &pollCode, const QString &devID)
{
    QReadLocker locker(&myLock);
    const auto r = dataTableNI.value(pollCode).values(devID);
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
    const auto r = dataTableSN.value(pollCode).value(devID);
    return r ;
}

DHMsecRecordList DataHolderSharedObject::getLastRecordsSN(const quint16 &pollCode, const QString &devID)
{
    QReadLocker locker(&myLock);
    const auto r = dataTableSN.value(pollCode).values(devID);
    return r ;
}

//----------------------------------------------------------------------------

bool DataHolderSharedObject::isItAPulseMeterNextChannel(const qint64 &msec, const quint16 &pollCode, const QString &additionalID, const DHMsecRecord &oldrecord)
{

    if(msec == oldrecord.msec && !oldrecord.wasRestored && oldrecord.additionalID == additionalID){
        //pulse meters have the same NI ans SN for different channels,

        return isItAPulseMeterPollCode(pollCode);


    }

    return false;
}

//----------------------------------------------------------------------------
bool DataHolderSharedObject::isItAPulseMeterPollCode(const quint16 &pollCode)
{
    const auto pcSimple = (pollCode%20);
    return (pcSimple == UC_METER_PULSE);
}

//----------------------------------------------------------------------------

void DataHolderSharedObject::createDataProcessor()
{
    dataProcessor = new DataHolderSharedObjectProcessor(verboseMode, this);
    dataProcessor->createLinesIterator();
    connect(this, &DataHolderSharedObject::setEventManagerRules, dataProcessor, &DataHolderSharedObjectProcessor::setEventManagerRules);

    connect(dataProcessor, &DataHolderSharedObjectProcessor::sendCommand2pollDevMap , this, &DataHolderSharedObject::sendCommand2pollDevMap);
    connect(dataProcessor, &DataHolderSharedObjectProcessor::sendCommand2pollDevStr , this, &DataHolderSharedObject::sendCommand2pollDevStr);
    connect(dataProcessor, &DataHolderSharedObjectProcessor::sendAMessageDevMap     , this, &DataHolderSharedObject::sendAMessageDevMap);

    connect(this, &DataHolderSharedObject::onThisCommandFailed, dataProcessor, &DataHolderSharedObjectProcessor::onThisCommandFailed);

    //from iterator
    connect(dataProcessor, &DataHolderSharedObjectProcessor::gimmeThisDevIDData             , this, &DataHolderSharedObject::gimmeThisDevIDData             );
    connect(dataProcessor, &DataHolderSharedObjectProcessor::gimmeThisAdditionalDevIDData   , this, &DataHolderSharedObject::gimmeThisAdditionalDevIDData   );


   //to iterator
    connect(this, &DataHolderSharedObject::setThisDevIDData             , dataProcessor, &DataHolderSharedObjectProcessor::setThisDevIDData             );
    connect(this, &DataHolderSharedObject::setThisAdditionalDevIDData   , dataProcessor, &DataHolderSharedObjectProcessor::setThisAdditionalDevIDData   );

    connect(dataProcessor, &DataHolderSharedObjectProcessor::append2log, this, &DataHolderSharedObject::append2log);
    connect(dataProcessor, &DataHolderSharedObjectProcessor::addThisDHEvent, this, &DataHolderSharedObject::addThisDHEvent);

    connect(this, &DataHolderSharedObject::testThisRule     , dataProcessor, &DataHolderSharedObjectProcessor::testThisRule);
    connect(this, &DataHolderSharedObject::resetThisRules   , dataProcessor, &DataHolderSharedObjectProcessor::resetThisRules);
    connect(this, &DataHolderSharedObject::smartSystemEvent , dataProcessor, &DataHolderSharedObjectProcessor::smartSystemEvent);

}

//----------------------------------------------------------------------------

//void DataHolderSharedObject::checkThisDevNoAnswer(quint16 pollCode, QString devID, QString additionalID, qint64 msec, QString srcname)
//{

//}

//----------------------------------------------------------------------------

void DataHolderSharedObject::addRecord(quint16 pollCode, QString devID, QString additionalID, qint64 msec, QVariantHash hash, QString srcname)
{
    if(pollCode == 0 )
        return;

    if(hash.isEmpty())
        return;




    DHMsecRecordList outl;

    if(isItAPulseMeterPollCode(pollCode)){
        outl = addAPulseMeterRecord(pollCode, devID, additionalID, msec, hash, srcname);
    }else{

        if(!devID.isEmpty()){

            DHMsecRecordList oldrecords = getLastRecordsNI(pollCode, devID);

            if(checkAddThisRecordMeters(oldrecords, msec, additionalID)){
                oldrecords.append(DHMsecRecord(msec, additionalID, hash, srcname, false));
                outl.append(oldrecords.constLast());


                QWriteLocker locker(&myLock);
                auto pollCodeTable = dataTableNI.value(pollCode);


                pollCodeTable.remove(devID);//if you use insertMulti, than it must be deleted,
                //if insertMulti is used, then be ready for keys dublication

                for(int i = 0, imax = oldrecords.size(); i < imax; i++)
                    pollCodeTable.insertMulti(devID, oldrecords.at(i));


                dataTableNI.insert(pollCode, pollCodeTable);

                if(verboseMode)
                    qDebug() << "addRecord dataTableNI NI " << devID;

            }

//            const DHMsecRecord oldrecord = getLastRecordNI(pollCode, devID);
//this doesn't work properly with virtual meters
//            if(msec > oldrecord.msec){
//                //newer
//                const DHMsecRecord newrecord(msec, additionalID, hash, srcname, false);

//                outl.append(newrecord);

//                QWriteLocker locker(&myLock);
//                auto pollCodeTable = dataTableNI.value(pollCode);

//                pollCodeTable.insert(devID, newrecord);
//                dataTableNI.insert(pollCode, pollCodeTable);

//                if(verboseMode)
//                    qDebug() << "addRecord dataTableNI " << devID << pollCodeTable.size() ;




//            }


        }

        if(!additionalID.isEmpty()){
            const DHMsecRecord oldrecord = getLastRecordSN(pollCode, additionalID);

            if(msec > oldrecord.msec){
                //newer
                const DHMsecRecord newrecord(msec, devID,  hash, srcname, false);
                outl.append(newrecord);

                QWriteLocker locker(&myLock);
                auto pollCodeTable = dataTableSN.value(pollCode);
                pollCodeTable.insert(additionalID, newrecord);
                dataTableSN.insert(pollCode, pollCodeTable);

                if(verboseMode)
                    qDebug() << "addRecord dataTableSN " << additionalID << pollCodeTable.size();
            }
        }

    }

    if(outl.isEmpty())
        return;




    if(verboseMode)
        qDebug() << "addRecord " << pollCode << devID << additionalID << srcname;
    emit onDataTableChanged();



    const auto oneRecord = outl.constFirst();

    if(oneRecord.additionalID == additionalID){
        dataProcessor->checkThisDevice(pollCode, devID, oneRecord);
    }

}


//----------------------------------------------------------------------------


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

        if(checkAddRestoredRecords(pollCodeTableNI, pollCode, nis, sns, msecs, hashs, srcnames)){
            dataTableNI.insert(pollCode, pollCodeTableNI);
            sayThatChanged = true;
        }

    }
    if(sayThatChanged)
        emit onDataTableChanged();

}

//----------------------------------------------------------------------------

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

        if(checkAddRestoredRecords(pollCodeTableSN, pollCode, sns, nis, msecs, hashs, srcnames)){
            dataTableSN.insert(pollCode, pollCodeTableSN);
            sayThatChanged = true;
        }

    }
    if(sayThatChanged)
        emit onDataTableChanged();
}

//----------------------------------------------------------------------------

void DataHolderSharedObject::gimmeThisDevIDData(QString devID, quint16 pollCode, QString dataKey)
{
    const auto value = getLastRecordNI(pollCode, devID).hash.value(dataKey).toString();

    emit setThisDevIDData(devID, pollCode, dataKey, value);
}

//----------------------------------------------------------------------------

void DataHolderSharedObject::gimmeThisAdditionalDevIDData(QString devID, QString additionalDevID, quint16 pollCode, QString dataKey)
{
    const auto values = getLastRecordsNI(pollCode, devID);

    for(int i = 0, imax = values.size(); i < imax; i++){
        if(values.at(i).additionalID == additionalDevID){
            emit setThisAdditionalDevIDData(devID, additionalDevID, pollCode, dataKey, values.at(i).hash.value(dataKey).toString());
            return;
        }
    }


}



//----------------------------------------------------------------------------

DHMsecRecordList DataHolderSharedObject::addAPulseMeterRecord(const quint16 &pollCode, const QString &devID, const QString &additionalID, const qint64 &msec, const QVariantHash &hash, const QString &srcname)
{

    DHMsecRecordList outl;

    if(verboseMode)
        qDebug() << "addAPulseMeterRecord ";

    const QString chnnl = hash.value("chnnl").toString();
    if(chnnl.isEmpty())
        return outl;



    if(!devID.isEmpty()){
        DHMsecRecordList oldrecords = getLastRecordsNI(pollCode, devID);

        //    QHash(("tvlu", QVariant(QString, "123"))("chnnl", QVariant(QString, "0")))

        if(checkAddThisRecord(oldrecords, msec, chnnl)){
            oldrecords.append(DHMsecRecord(msec, additionalID, hash, srcname, false));

            outl.append(oldrecords.constLast());

            QWriteLocker locker(&myLock);
            auto pollCodeTable = dataTableNI.value(pollCode);


            pollCodeTable.remove(devID);//if you use insertMulti, than it must be deleted,
            //if insertMulti is used, then be ready for keys dublication

            for(int i = 0, imax = oldrecords.size(); i < imax; i++)
                pollCodeTable.insertMulti(devID, oldrecords.at(i));


            dataTableNI.insert(pollCode, pollCodeTable);

            if(verboseMode)
                qDebug() << "addAPulseMeterRecord NI " << devID;
        }



    }

    if(!additionalID.isEmpty()){
        DHMsecRecordList oldrecords = getLastRecordsSN(pollCode, additionalID);


        if(checkAddThisRecord(oldrecords, msec, chnnl)){
            oldrecords.append(DHMsecRecord(msec, devID, hash, srcname, false));
            outl.append(oldrecords.constLast());


            QWriteLocker locker(&myLock);
            auto pollCodeTable = dataTableSN.value(pollCode);

            pollCodeTable.remove(additionalID);//if you use insertMulti, than it must be deleted,

            for(int i = 0, imax = oldrecords.size(); i < imax; i++)
                pollCodeTable.insertMulti(additionalID, oldrecords.at(i));


            dataTableSN.insert(pollCode, pollCodeTable);

            if(verboseMode)
                qDebug() << "addAPulseMeterRecord SN " << additionalID;
        }

    }



    return outl;

}
//----------------------------------------------------------------------------


bool DataHolderSharedObject::checkAddThisRecord(DHMsecRecordList &oldrecords, const qint64 &msec, const QString &chnnl)
{
    //    QHash(("tvlu", QVariant(QString, "123"))("chnnl", QVariant(QString, "0")))

    for(int i = 0, imax = oldrecords.size(); i < imax; i++){
        //remove the same channel if this data is newer, or say that nothing has to be changed
        const DHMsecRecord oldrecord = oldrecords.at(i);
        const QString chnnlOld = oldrecord.hash.value("chnnl").toString();
        if(chnnl == chnnlOld){
            //it has found the necessary channel, so stop searching
            if(msec > oldrecord.msec){
                oldrecords.removeAt(i);
                return true;
            }
            return false;//there is nothing to change

        }

    }
    return true;//if list is empty or there is no such channel
}

//----------------------------------------------------------------------------

bool DataHolderSharedObject::checkAddThisRecordMeters(DHMsecRecordList &oldrecords, const qint64 &msec, const QString &additionalID)
{
    for(int i = 0, imax = oldrecords.size(); i < imax; i++){
        //remove the same channel if this data is newer, or say that nothing has to be changed
        const DHMsecRecord oldrecord = oldrecords.at(i);

        if(additionalID == oldrecord.additionalID){
            //it has found the necessary channel, so stop searching
            if(msec > oldrecord.msec){
                oldrecords.removeAt(i);
                return true;
            }
            return false;//there is nothing to change

        }

    }
    return true;//if list is empty or there is no such channel
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

bool DataHolderSharedObject::checkAddRestoredRecords(DHDevId2data &pollCodeTable, const quint16 &pollCode, const QStringList &devIDs, const QStringList &additionalIDs, const QList<qint64> &msecs, const QList<QVariantHash> &hashs, const QStringList &srcnames)
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
        }else{
            if(isItAPulseMeterNextChannel(msec, pollCode, additionalIDs.at(i), oldrecord)){
                pollCodeTable.insertMulti(devID, DHMsecRecord(msec, additionalIDs.at(i), hashs.at(i), srcnames.at(i), true));
                sayThatChanged = true;
            }
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
