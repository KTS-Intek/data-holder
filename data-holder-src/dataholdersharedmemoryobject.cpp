#include "dataholdersharedmemoryobject.h"
#include <QDataStream>


DataHolderSharedMemoryObject::DataHolderSharedMemoryObject(DataHolderSharedObject *dhData, const QString &sharedMemoName, const QString &semaName, const int &delay, const bool &verboseMode, QObject *parent) :
    SharedMemoWriter(sharedMemoName, semaName, "", delay, 33333, verboseMode, parent)
{
    this->dhData = dhData;


}

DataHolderSharedMemoryObject::DHOneRestoredPollCodeData DataHolderSharedMemoryObject::fromDHDevId2data(const DHDevId2data &idsData)
{
    const QList<QString> lkIDs = idsData.keys();

    DHOneRestoredPollCodeData mydata;

//    QStringList nis;
//    QStringList sns;
//    QList<qint64> msecs;
//    QList<QVariantHash> hashs;


    for(int j = 0, jmax = lkIDs.size(); j < jmax; j++){
        const QString devID = lkIDs.at(j);
        const DHMsecRecord record = idsData.value(devID);

        if(!devID.isEmpty() && record.msec > 0 && !record.hash.isEmpty()){
            mydata.devIDs.append(devID);
            mydata.additionalDevIDs.append(record.additionalID);
            mydata.msecs.append(record.msec);
            mydata.hashs.append(record.hash);
            mydata.srcnames.append(record.srcname);

        }

//        if(rows > 100){
//            //do not use heavy containers
//            emit addRestoredRecords(pollCode, nis, msecs, hashs);

//            nis.clear();
//            msecs.clear();
//            hashs.clear();
//            rows = 0;
//        }

    }
    return mydata;

}

void DataHolderSharedMemoryObject::onThreadStarted()
{
//    mymaximums.write2ram = 60;
    connect(this, &DataHolderSharedMemoryObject::ready2flushArr, this, &DataHolderSharedMemoryObject::sendMeTheTableNow);

    setMirrorMode(true);

    initObjectLtr();
}

void DataHolderSharedMemoryObject::onTableChanged()
{
    changeSharedMemArrDataCounter();
}

void DataHolderSharedMemoryObject::onAppIsGoinToQuite()
{
    flushAllNowAndDie();//flush and die

}

void DataHolderSharedMemoryObject::sendMeTheTableNow()
{
    flushNowArr(getTableAsAnArray());
}

void DataHolderSharedMemoryObject::onTableRestored(QByteArray arr)
{
    DHDataTable dataTableNI, dataTableSN;
    if(!arr.isEmpty()){
        QDataStream stream(&arr, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_6);
        stream >> dataTableNI >> dataTableSN;
    }
    //add via     void addRecord(quint16 pollCode, QString ni, qint64 msec, QVariantHash hash);


    if(!dataTableNI.isEmpty())
        onTableRestoredNI(dataTableNI);

    dataTableNI.clear();
    if(!dataTableSN.isEmpty())
        onTableRestoredSN(dataTableSN);
    dataTableSN.clear();


}

void DataHolderSharedMemoryObject::onTableRestoredNI(const DHDataTable &dataTableNI)
{


    const QList<quint16> lk = dataTableNI.keys();
    for(int i = 0, imax = lk.size(); i < imax; i++){
        const quint16 pollCode = lk.at(i);

        if(pollCode < 1)
            continue;


        const auto mydata = fromDHDevId2data(dataTableNI.value(pollCode));

        if(!mydata.devIDs.isEmpty())
            emit addRestoredRecordsNI(pollCode, mydata.devIDs, mydata.additionalDevIDs, mydata.msecs, mydata.hashs, mydata.srcnames);
    }
}

void DataHolderSharedMemoryObject::onTableRestoredSN(const DHDataTable &dataTableSN)
{

    const QList<quint16> lk = dataTableSN.keys();
    for(int i = 0, imax = lk.size(); i < imax; i++){
        const quint16 pollCode = lk.at(i);

        if(pollCode < 1)
            continue;

        const auto mydata = fromDHDevId2data(dataTableSN.value(pollCode));
        if(!mydata.devIDs.isEmpty())
            emit addRestoredRecordsSN(pollCode, mydata.devIDs, mydata.additionalDevIDs, mydata.msecs, mydata.hashs, mydata.srcnames);
    }
}

QByteArray DataHolderSharedMemoryObject::getTableAsAnArray()
{
    QByteArray writeArr;
    QDataStream stream(&writeArr, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_6);
    stream << dhData->getDataTableNI() << dhData->getDataTableSN();
    return writeArr;
}
