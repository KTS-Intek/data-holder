#ifndef DATAHOLDERMANAGER_H
#define DATAHOLDERMANAGER_H

#include <QObject>
#include <QtCore>

#include "dataholdersharedmemoryobject.h"
#include "dataholderlocalserver.h"

class DataHolderManager : public QObject
{
    Q_OBJECT
public:
    explicit DataHolderManager(QObject *parent = nullptr);

    ~DataHolderManager();

    bool verboseMode;
    DataHolderSharedObject *dhData;//it stores data by NI and SN



signals:
    void killAllAndExit();


public slots:
    void saveAllYourData();

    void createObjects();

    void createObjectsLater();

    void append2log(QString message);//it adds date time automatically


    void reloadDataFromTheFile();

private:

    void createSharedTableObject();

    void createShareMemoryWriter();

    void createLocalServerObject();

};

#endif // DATAHOLDERMANAGER_H
