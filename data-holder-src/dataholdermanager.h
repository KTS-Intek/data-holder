#ifndef DATAHOLDERMANAGER_H
#define DATAHOLDERMANAGER_H

#include <QObject>
#include <QtCore>

#include "dataholdersharedmemoryobject.h"
#include "dataholderlocalserver.h"
#include "matildaconnectionsocket.h"
#include "dataholdermessagesender.h"

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

    void setEventManagerRules(QVariantHash hashRules);


    //to external apps
    void sendCommand2pollDevStr(quint16 pollCode, QString args);

    void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);

    void sendAMessageDevMap(QVariantMap mapArgs, QString messageClientName);

    //status of a messaga
    void onThisCommandFailed(QString ruleNameId, QString counterId);


    void smartPingTheseHosts(QStringList hosts);


    void add2systemLogEvent(QString evnt);

public slots:
    void saveAllYourData();

    void createObjects();

    void createObjectsLater();

    void append2log(QString message);//it adds date time automatically

    void onConfigChanged(quint16 command, QVariant datavar);

    void reloadAllSettings();

    void reloadDataFromTheFile();

private:

    void createShareMemoryWriterAppLogs();

    void createSharedTableObject();

    void createShareMemoryWriter();

    void createLocalServerObject();

    void createMatildaLocalSocket();

    void createMessageSender();
};

#endif // DATAHOLDERMANAGER_H
