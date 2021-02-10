#include "dataholdersharedmemoryobject.h"
#include <QDataStream>


DataHolderSharedMemoryObject::DataHolderSharedMemoryObject(DataHolderSharedObject *dhData, const QString &sharedMemoName, const QString &semaName, const int &delay, const bool &verboseMode, QObject *parent) :
    SharedMemoWriter(sharedMemoName, semaName, "", delay, 33333, verboseMode, parent)
{
    this->dhData = dhData;


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
    DHDataTable dataTable;
    if(!arr.isEmpty()){
        QDataStream stream(&arr, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_6);
        stream >> dataTable;
    }
    //add via     void addRecord(quint16 pollCode, QString ni, qint64 msec, QVariantHash hash);



    const QList<quint16> lk = dataTable.keys();
    for(int i = 0, imax = lk.size(); i < imax; i++){
        const quint16 pollCode = lk.at(i);

        if(pollCode < 1)
            continue;

        const DHNI2data nisdata = dataTable.value(pollCode);
        const QList<QString> lkni = nisdata.keys();


        QStringList nis;
        QList<qint64> msecs;
        QList<QVariantHash> hashs;


        for(int j = 0, jmax = lkni.size(), rows = 0; j < jmax; j++){
            const QString ni = lkni.at(j);
            const DHMsecRecord record = nisdata.value(ni);

            if(!ni.isEmpty() && record.msec > 0 && !record.hash.isEmpty()){
                rows++;
                nis.append(ni);
                msecs.append(record.msec);
                hashs.append(record.hash);
            }

            if(rows > 100){
                //do not use heavy containers
                emit addRestoredRecords(pollCode, nis, msecs, hashs);

                nis.clear();
                msecs.clear();
                hashs.clear();
                rows = 0;
            }

        }

        if(!nis.isEmpty())
            emit addRestoredRecords(pollCode, nis, msecs, hashs);
    }


}

QByteArray DataHolderSharedMemoryObject::getTableAsAnArray()
{
    QByteArray writeArr;
    QDataStream stream(&writeArr, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_6);
    stream << dhData->getDataTable();
    return writeArr;
}
