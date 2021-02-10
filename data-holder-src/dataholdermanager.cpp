#include "dataholdermanager.h"

#include <QTimer>

///[!] sharedmemory
#include "src/shared/sharedmemohelper.h"


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

}
