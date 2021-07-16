#ifndef DATAHOLDERSHAREDMEMORYOBJECT_H
#define DATAHOLDERSHAREDMEMORYOBJECT_H

///[!] sharedmemory
#include "src/shared/sharedmemowriter.h"



#include "dataholdersharedobject.h"

class DataHolderSharedMemoryObject : public SharedMemoWriter
{
    Q_OBJECT
public:
    explicit DataHolderSharedMemoryObject(DataHolderSharedObject *dhData, const QString &sharedMemoName, const QString &semaName, const int &delay, const bool &verboseMode, QObject *parent = nullptr);

    DataHolderSharedObject *dhData;

    struct DHOneRestoredPollCodeData
    {
        QStringList devIDs;
        QStringList additionalDevIDs;
        QList<qint64> msecs;
        QList<QVariantHash> hashs;
        QStringList srcnames;
        DHOneRestoredPollCodeData() {}
    };


    DHOneRestoredPollCodeData fromDHDevId2data(const DHDevId2data &idsData);


signals:
    void addRestoredRecordsNI(quint16 pollCode, QStringList nis, QStringList sns, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames);

    void addRestoredRecordsSN(quint16 pollCode, QStringList sns, QStringList nis, QList<qint64> msecs, QList<QVariantHash> hashs, QStringList srcnames);

public slots:
    void onThreadStarted();

    void onTableChanged();

    void onAppIsGoinToQuite();

private slots:
    void sendMeTheTableNow();

    void onTableRestored(QByteArray arr);

    void onTableRestoredNI(const DHDataTable &dataTableNI);

    void onTableRestoredSN(const DHDataTable &dataTableSN);

private:
    QByteArray getTableAsAnArray();

};

#endif // DATAHOLDERSHAREDMEMORYOBJECT_H
