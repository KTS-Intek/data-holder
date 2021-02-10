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

signals:
    void addRestoredRecords(quint16 pollCode, QStringList nis, QList<qint64> msecs, QList<QVariantHash> hashs);

public slots:
    void onThreadStarted();

    void onTableChanged();

    void onAppIsGoinToQuite();

private slots:
    void sendMeTheTableNow();

    void onTableRestored(QByteArray arr);


private:
    QByteArray getTableAsAnArray();

};

#endif // DATAHOLDERSHAREDMEMORYOBJECT_H
