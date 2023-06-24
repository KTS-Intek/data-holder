#include "dataholdermanager.h"

#include <QTimer>

///[!] sharedmemory
#include "src/shared/sharedmemohelper.h"


///[!] MatildaIO
#include "matilda-bbb-src/shared/pathsresolver.h"


///[!] matilda-bbb-settings
#include "src/matilda/settloader4matilda.h"
#include "src/matilda/settloader4matildadefaults.h"


#include "dataholderapplogs.h"

///[!] firefly-settings
#include "firefly-v2-src/fireflyfilesetthelper.h"


///for test only
#include "dataholderlocalsocket.h"

#include "moji_defy.h"


//---------------------------------------------------------------------------------------

DataHolderManager::DataHolderManager(QObject *parent) : QObject(parent)
{

}

//---------------------------------------------------------------------------------------

DataHolderManager::~DataHolderManager()
{
    saveAllYourData();
}

//---------------------------------------------------------------------------------------

void DataHolderManager::saveAllYourData()
{
    emit killAllAndExit();
}

//---------------------------------------------------------------------------------------

void DataHolderManager::createObjects()
{
    verboseMode = qApp->arguments().contains("-vv");
    createShareMemoryWriterAppLogs(); //it must be the first
    createSharedTableObject();
    createShareMemoryWriter();
    createLocalServerObject();
    createMatildaLocalSocket();
    createMessageSender();

    reloadAllSettings();
}

//---------------------------------------------------------------------------------------

void DataHolderManager::createObjectsLater()
{
    QTimer::singleShot(111, this, SLOT(createObjects()));
}

//---------------------------------------------------------------------------------------

void DataHolderManager::append2log(QString message)
{
    emit add2systemLogEvent(message);
    if(verboseMode)
        qDebug() << "DataHolderManager log " << message;
}

//---------------------------------------------------------------------------------------

void DataHolderManager::onConfigChanged(quint16 command, QVariant datavar)
{
    Q_UNUSED(datavar);

    switch(command){
    case MTD_EXT_COMMAND_RELOAD_SETT:{
        reloadAllSettings();
        break; }

    default:{
        if(verboseMode)
            qDebug() << "DataHolderManager::onConfigChanged " << command << datavar;
        break;}
    }
}

//---------------------------------------------------------------------------------------

void DataHolderManager::reloadAllSettings()
{
    append2log(tr("Reload settings"));
    QVariantHash hashRules = SettLoader4matilda().loadOneSett(SETT_DATAHOLDR_EVNTMNGR_RLS).toHash();
    QVariantHash hashProfiles = SettLoader4matilda().loadOneSett(SETT_DATAHOLDR_MSSNGR_PRFLS).toHash();



    emit setEventManagerRules(hashRules, hashProfiles, FireflyFileSettHelper::getDevPos());


}

//---------------------------------------------------------------------------------------

void DataHolderManager::reloadDataFromTheFile()
{
    //I need this method for test only, do not use it in production

    append2log(tr("Reloading data from file, test feature, do not use it in production"));
    QFile file;
    file.setFileName("dataholder.data");
    if(file.open(QFile::ReadOnly)){
        const QByteArray readarr = file.readAll();
        QJsonArray jsonarr = QJsonDocument::fromJson(readarr).array();

        if(jsonarr.isEmpty()){
            qDebug() << "DataHolderManager jsonarr.isEmpty ";

        }else{

            DataHolderLocalSocket *socket = new DataHolderLocalSocket(dhData, true, this);
            socket->mtdExtNameTxt = "zbyrator-bbb2";

            for(int i = 0, imax = jsonarr.size(); i < imax; i++){


                QJsonObject json = jsonarr.at(i).toObject();
                QVariantHash hash = json.toVariantHash();
                hash.insert("data", json.value("data").toObject().toVariantHash());

                socket->addDataFromTheFile(hash);

                qDebug() << "DataHolderManager added items  " << i << hash.value("NI") << hash.value("pollCode");

            }
            socket->deleteLater();

        }

    }
}

//---------------------------------------------------------------------------------------

void DataHolderManager::createShareMemoryWriterAppLogs()
{
    QThread *writerthred = new QThread;
    writerthred->setObjectName("DHAppLogs");

    DataHolderAppLogs *writer = new DataHolderAppLogs(
                SharedMemoHelper::defDataHolderAppLogMemoName(),
                SharedMemoHelper::defDataHolderAppLogSemaName(),
                PathsResolver::path2config() + "/tmp/data-holderapp.log", 2222, 60000, verboseMode);


//    PathsResolver::path2configTmp()
    writer->mymaximums.write2ram = 120;
    writer->mymaximums.write2file = 250;

    writer->moveToThread(writerthred);
    connect(writer, SIGNAL(destroyed(QObject*)), writerthred, SLOT(quit()));
    connect(writerthred, SIGNAL(finished()), writerthred, SLOT(deleteLater()));
    connect(writerthred, SIGNAL(started()), writer, SLOT(initObjectLtr()));

    connect(this, &DataHolderManager::killAllAndExit, writer, &SharedMemoWriter::flushAllNowAndDie);

//    connect(this, &HTTPResourceManager::add2systemLogError, writer, &DataHolderAppLogs::add2systemLogError);
    connect(this, &DataHolderManager::add2systemLogEvent, writer, &DataHolderAppLogs::add2systemLogEvent);
//    connect(this, &HTTPResourceManager::add2systemLogWarn , writer, &DataHolderAppLogs::add2systemLogWarn);

//    connect(this, SIGNAL(goGoGo()), writerthred, SLOT(start()));
    connect(this, &DataHolderManager::addThisDHEvent, writer, &DataHolderAppLogs::addThisDHEvent);


    writerthred->start();
//    return writer;
}

//---------------------------------------------------------------------------------------

void DataHolderManager::createSharedTableObject()
{
    dhData = new DataHolderSharedObject(verboseMode, this);
    connect(this, &DataHolderManager::setEventManagerRules, dhData, &DataHolderSharedObject::setEventManagerRules);


//    createDataProcessor must be called before the connections
    connect(dhData, &DataHolderSharedObject::sendCommand2pollDevMap , this, &DataHolderManager::sendCommand2pollDevMap);
    connect(dhData, &DataHolderSharedObject::sendCommand2pollDevStr , this, &DataHolderManager::sendCommand2pollDevStr);
    connect(dhData, &DataHolderSharedObject::sendAMessageDevMap     , this, &DataHolderManager::sendAMessageDevMap);
    connect(dhData, &DataHolderSharedObject::sendAnIPCMessageDevMap , this, &DataHolderManager::sendAnIPCMessageDevMap);



    connect(dhData, &DataHolderSharedObject::append2log, this, &DataHolderManager::append2log);
    connect(dhData, &DataHolderSharedObject::addThisDHEvent, this, &DataHolderManager::addThisDHEvent);

    connect(this, &DataHolderManager::sendTestMessage           , dhData, &DataHolderSharedObject::sendTestMessage);
}

//---------------------------------------------------------------------------------------

void DataHolderManager::createShareMemoryWriter()
{
    //it is data table
    QThread *shmemwriterThread = new QThread;
    shmemwriterThread->setObjectName("DataHolderSharedMemoryObject");

    DataHolderSharedMemoryObject *shmemwriter = new DataHolderSharedMemoryObject(
                dhData,
                SharedMemoHelper::defDataHolderMemoName(),
                SharedMemoHelper::defDataHolderSemaName(),
                2222,
                verboseMode);

    shmemwriter->moveToThread(shmemwriterThread);

    connect(shmemwriter, SIGNAL(destroyed(QObject*)), shmemwriterThread, SLOT(quit()));
    connect(shmemwriterThread, SIGNAL(finished()), shmemwriterThread, SLOT(deleteLater()));
    connect(shmemwriterThread, SIGNAL(started()), shmemwriter, SLOT(onThreadStarted()));

    connect(this, SIGNAL(killAllAndExit()), shmemwriter, SLOT(onAppIsGoinToQuite()));

    shmemwriterThread->start();

}

//---------------------------------------------------------------------------------------

void DataHolderManager::createLocalServerObject()
{
//    QTimer::singleShot(2444, this, SLOT(reloadDataFromTheFile()));//for test only, do not use it in production


    QThread *t = new QThread;
    t->setObjectName("DataHolderLocalServer");
    DataHolderLocalServer *localServer = new DataHolderLocalServer(dhData, verboseMode);
    localServer->moveToThread(t);

    connect(this, &DataHolderManager::killAllAndExit, localServer, &DataHolderLocalServer::kickOffLocalServer);
    connect(localServer, SIGNAL(destroyed(QObject*)), t, SLOT(quit()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    connect(t, SIGNAL(started()), localServer, SLOT(onThrdStarted()) );

    connect(localServer, SIGNAL(append2log(QString)), this, SLOT(append2log(QString)) );


    QTimer::singleShot(44, t, SLOT(start()));

}

//---------------------------------------------------------------------------------------

void DataHolderManager::createMatildaLocalSocket()
{


    MatildaConnectionSocket *extSocket = new MatildaConnectionSocket(verboseMode);
//    extSocket->activeDbgMessages = activeDbgMessages;

    extSocket->initializeSocket(MTD_EXT_NAME_DATAHOLDER);
    QThread *extSocketThrd = new QThread(this); //QT2
    extSocketThrd->setObjectName("MatildaConnectionSocket");
    extSocket->moveToThread(extSocketThrd);
#ifdef ENABLE_VERBOSE_SERVER
    connect(extSocket, &MatildaConnectionSocket::appendDbgExtData, this, &ZbyratorManager::appendDbgExtData );
#endif
    connect(extSocketThrd, &QThread::started, extSocket, &MatildaConnectionSocket::onThreadStarted);

    connect(extSocket, &MatildaConnectionSocket::onConfigChanged , this, &DataHolderManager::onConfigChanged  );


    connect(this, &DataHolderManager::killAllAndExit, extSocket, &MatildaConnectionSocket::killAllObjects);


    connect(this, &DataHolderManager::sendCommand2pollDevMap, extSocket, &MatildaConnectionSocket::sendCommand2pollDevMap);
    connect(this, &DataHolderManager::sendCommand2pollDevStr, extSocket, &MatildaConnectionSocket::sendCommand2pollDevStr);
    connect(this, &DataHolderManager::sendAnIPCMessageDevMap, extSocket, &MatildaConnectionSocket::sendAnIPCMessageDevMap);


    connect(extSocket, &MatildaConnectionSocket::append2log, this, &DataHolderManager::append2log);



    //from IPC to messageSender
    connect(extSocket, &MatildaConnectionSocket::smartPingTheseHostsResult  , this, &DataHolderManager::smartPingTheseHostsResult);
    connect(extSocket, &MatildaConnectionSocket::sendTestMessage            , this, &DataHolderManager::sendTestMessage);

//    void smartPingTheseHostsResult(QString messagetag, bool ok, QString message);
    //from messageSender
    connect(this, &DataHolderManager::smartPingTheseHosts, extSocket, &MatildaConnectionSocket::smartPingTheseHosts);
//    void smartPingTheseHosts(QStringList hosts, QString messagetag);//ask matilda-bbb iface manager to ping , in case of error restart eth0


    connect(extSocket, &MatildaConnectionSocket::testThisRule       , dhData, &DataHolderSharedObject::testThisRule     );
    connect(extSocket, &MatildaConnectionSocket::resetThisRules     , dhData, &DataHolderSharedObject::resetThisRules   );
    connect(extSocket, &MatildaConnectionSocket::smartSystemEvent   , dhData, &DataHolderSharedObject::smartSystemEvent );
//    connect(extSocket, &MatildaConnectionSocket::resetThisRules     , dhData, &DataHolderSharedObject::resetThisRules   );


    extSocketThrd->start();

}

//---------------------------------------------------------------------------------------

void DataHolderManager::createMessageSender()
{
    DataHolderMessageSender *messanger = new DataHolderMessageSender(verboseMode);
    QThread *t = new QThread;
    t->setObjectName("DHMessanger");
    messanger->moveToThread(t);

    connect(t, SIGNAL(started()), messanger, SLOT(onThreadStarted()) );
    connect(messanger, SIGNAL(destroyed(QObject*)), t, SLOT(quit()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    connect(messanger, &DataHolderMessageSender::append2log         , this, &DataHolderManager::append2log          );
    connect(messanger, &DataHolderMessageSender::onThisCommandFailed, dhData, &DataHolderSharedObject::onThisCommandFailed );


    connect(this, &DataHolderManager::sendAMessageDevMap    , messanger, &DataHolderMessageSender::sendAMessageDevMap);



    //from IPC to messageSender
    connect(this, &DataHolderManager::smartPingTheseHostsResult , messanger, &DataHolderMessageSender::smartPingTheseHostsResult, Qt::DirectConnection);
//    connect(this, &DataHolderManager::sendTestMessage           , messanger, &DataHolderMessageSender::sendTestMessage);

    //from messageSender
    connect(messanger, &DataHolderMessageSender::smartPingTheseHosts, this, &DataHolderManager::smartPingTheseHosts);


    t->start();

}

//---------------------------------------------------------------------------------------
