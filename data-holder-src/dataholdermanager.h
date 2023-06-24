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

    void setEventManagerRules(QVariantHash hashRules, QVariantHash hashProfiles, QPointF devCoordinate);


    //to external apps
    void sendCommand2pollDevStr(quint16 pollCode, QString args);

    void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);

    void sendAMessageDevMap(QVariantMap mapArgs);

    void sendAnIPCMessageDevMap(QVariantMap mapArgs);


    void addThisDHEvent(QString ruleName, int cntr, quint16 pollCode, QString ruleLine, QString devId, QString additioanlDevId);


    void add2systemLogEvent(QString evnt);


    //from IPC to messageSender
    void sendTestMessage(QString profName, QVariantHash oneProf);

    void smartPingTheseHostsResult(QString messagetag, bool ok, QString message);
    //from messageSender
    void smartPingTheseHosts(QStringList hosts, QString messagetag, quint8 askReset);//ask matilda-bbb iface manager to ping , in case of error restart eth0

    void smartSystemEvent(QString who, QString evntType, QVariantHash payload);//who app name, evntType logIn,logOut,authFail,appRestart,gsmMoney...


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
