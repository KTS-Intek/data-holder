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

    DataHolderSharedObject *dhData;

    QString getPath2server();


public slots:
    void onThrdStarted();



protected:
    void incomingConnection(quintptr socketDsk);

};

#endif // DATAHOLDERLOCALSERVER_H
