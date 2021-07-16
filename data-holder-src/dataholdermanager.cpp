#include "dataholdermanager.h"

#include <QTimer>

///[!] sharedmemory
#include "src/shared/sharedmemohelper.h"


///for test only
#include "dataholderlocalsocket.h"

DataHolderManager::DataHolderManager(QObject *parent) : QObject(parent)
{

}

DataHolderManager::~DataHolderManager()
{
   saveAllYourData();
}

void DataHolderManager::saveAllYourData()
{
    emit killAllAndExit();
}

void DataHolderManager::createObjects()
{
    verboseMode = qApp->arguments().contains("-vv");
    createSharedTableObject();
    createShareMemoryWriter();
    createLocalServerObject();
}

void DataHolderManager::createObjectsLater()
{
    QTimer::singleShot(111, this, SLOT(createObjects()));
}

void DataHolderManager::append2log(QString message)
{
    if(verboseMode)
        qDebug() << "DataHolderManager log " << message;
}

void DataHolderManager::reloadDataFromTheFile()
{
    //I need this method for test only, do not use it in production

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

void DataHolderManager::createSharedTableObject()
{
    dhData = new DataHolderSharedObject(this);

}

void DataHolderManager::createShareMemoryWriter()
{
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
