#ifndef DATAHOLDERLOCALSERVER_H
#define DATAHOLDERLOCALSERVER_H

///[!] localsockets
#include "localservers/regularlocalserver.h"

#include "dataholdersharedobject.h"

class DataHolderLocalServer : public RegularLocalServer
{
    Q_OBJECT
public:
    explicit DataHolderLocalServer(DataHolderSharedObject *dhData, const bool &verboseMode, QObject *parent = nullptr);

    DataHolderSharedObject *dhData;//it stores data by NI and SN

    QString getPath2server();

signals:


public slots:
    void onThrdStarted();



protected:
    void incomingConnection(quintptr socketDsk);

};

#endif // DATAHOLDERLOCALSERVER_H
