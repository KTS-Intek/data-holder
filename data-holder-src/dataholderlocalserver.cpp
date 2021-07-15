#include "dataholderlocalserver.h"


#include "dataholderlocalsocket.h"



///[!] MatildaIO
#include "matilda-bbb-src/shared/pathsresolver.h"

//--------------------------------------------------------------------------------------

DataHolderLocalServer::DataHolderLocalServer(DataHolderSharedObject *dhDataNI, DataHolderSharedObject *dhDataSN, const bool &verboseMode, QObject *parent) :
    RegularLocalServer(verboseMode, parent)
{
    this->dhDataNI = dhDataNI;
    this->dhDataSN = dhDataSN;

}

//--------------------------------------------------------------------------------------

QString DataHolderLocalServer::getPath2server()
{
    return PathsResolver::localServerNameModbusBBB();

}

//--------------------------------------------------------------------------------------

void DataHolderLocalServer::onThrdStarted()
{
    initLocalServer();
    startServerLater(5);

}

//--------------------------------------------------------------------------------------

void DataHolderLocalServer::incomingConnection(quintptr socketDsk)
{
    DataHolderLocalSocket *socket  = new DataHolderLocalSocket(dhDataNI, dhDataSN, verboseMode, this);
    if(!socket->setSocketDescriptor(socketDsk)){
        socket->close();
        socket->deleteLater();
        return;
    }

    connect(socket, SIGNAL(append2log(QString)), this, SIGNAL(append2log(QString)) );
    socket->configureZombieKiller();

    if(verboseMode)
        qDebug() << "DataHolderLocalServer incomingConnection " << socket->serverName();
}

//--------------------------------------------------------------------------------------
