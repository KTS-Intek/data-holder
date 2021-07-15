#ifndef DATAHOLDERLOCALSERVER_H
#define DATAHOLDERLOCALSERVER_H

///[!] localsockets
#include "localservers/regularlocalserver.h"

#include "dataholdersharedobject.h"

class DataHolderLocalServer : public RegularLocalServer
{
    Q_OBJECT
public:
    explicit DataHolderLocalServer(DataHolderSharedObject *dhDataNI, DataHolderSharedObject *dhDataSN, const bool &verboseMode, QObject *parent = nullptr);

    DataHolderSharedObject *dhDataNI;//it stores data by NI
    DataHolderSharedObject *dhDataSN;//it stores data by SN

    QString getPath2server();


public slots:
    void onThrdStarted();



protected:
    void incomingConnection(quintptr socketDsk);

};

#endif // DATAHOLDERLOCALSERVER_H
