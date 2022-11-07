#ifndef DATAHOLDERLOCALSOCKET_H
#define DATAHOLDERLOCALSOCKET_H

///[!] localsockets
#include "localsockets/regularserversocket.h"


#include "dataholdersharedobject.h"

class DataHolderLocalSocket : public RegularServerSocket
{
    Q_OBJECT
public:
    explicit DataHolderLocalSocket(DataHolderSharedObject *dhData, const bool &verboseMode, QObject *parent = nullptr);


    DataHolderSharedObject *dhData;//it stores data by NI and SN

    quint16 getZombieCommand();

    QVariantHash getOkMessage(const QVariant &messagetag, const QVariant &objecttag);

    QVariantHash getErrorMessage(const QString &message, const QVariant &messagetag, const QVariant &objecttag);

signals:



public slots:


    void configureZombieKiller();

    void decodeReadData(const QVariant &dataVar, const quint16 &command);


    void addDataFromTheFile(const QVariantHash &hash);

private:
    QVariantHash onDATAHOLDER_ADD_POLLDATA(const QVariantHash &hash);

    QVariantHash onDATAHOLDER_GET_POLLDATA(const QVariantHash &hash);

    QVariantHash onDATAHOLDER_GET_POLLDATA_EXT(const QVariantHash &hash);

    QVariantHash getHashRecord(const quint16 &pollCode, const QString &devID, const bool &useSn4devID, const DHMsecRecord &pollCodeData);


    void appendData2file(const QVariantHash &hash);

    QJsonArray getFileContent();


};

#endif // DATAHOLDERLOCALSOCKET_H
