#ifndef MATILDACONNECTIONSOCKET_H
#define MATILDACONNECTIONSOCKET_H

///[!] ipc
#include "localsockets/regularlocalsocket.h"

#include <QtCore>


class MatildaConnectionSocket : public RegularLocalSocket
{
    Q_OBJECT
public:
    explicit MatildaConnectionSocket(bool verboseMode, QObject *parent = nullptr);

    bool hasTmrLater;

    void decodeReadData(const QVariant &dataVar, const quint16 &command);

    int gimmeAppId4pollCode(const quint16 &pollCode);

signals:
    void startTmrArgs();

    void append2log(QString message);


    //to dh processor
    void testThisRule(QString ruleName, QVariantHash oneRule);

    void resetThisRules(QStringList ruleNames);

    //from IPC to messageSender
    void smartPingTheseHostsResult(QString messagetag, bool ok, QString message);

    void sendTestMessage(QString profName, QVariantHash oneProf);

    void smartSystemEvent(QString who, QString evntType, QVariantHash payload);//who app name, evntType logIn,logOut,authFail,appRestart,gsmMoney...



public slots:
    //for client side

    void sendFireflyTempPowerCommand(bool remove, QString args);


    void sendAnIPCMessageDevMap(QVariantMap mapArgs);

    void sendCommand2pollDevStr(quint16 pollCode, QString args);

    void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);



        //from messageSender
     void smartPingTheseHosts(QStringList hosts, QString messagetag);//ask matilda-bbb iface manager to ping , in case of error restart eth0




    void sendCachedArgs();



private:
    bool sendCommand2appHashPwrCommand(const bool &remove,  const QString &args, const bool &isCached);


    bool sendCommand2appHash(quint16 pollCode, QVariantHash hash, bool isCached);

    void checkSendLater();

    void sendItLater(quint16 pollCode, QVariantHash hash);

    QList<quint16> lcodes;
    QList<QVariantHash> lhashs;

    struct LastFireflyTempPwrCommand
    {
        bool remove;
        QString args;
        LastFireflyTempPwrCommand() : remove(false) {}
        LastFireflyTempPwrCommand(const bool &remove, const QString &args) : remove(remove), args(args) {}
    } lPwrCommand;

};

#endif // MATILDACONNECTIONSOCKET_H
