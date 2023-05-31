#ifndef DATAHOLDERMESSAGESENDER_H
#define DATAHOLDERMESSAGESENDER_H

#include <QObject>
#include <QtCore>

class DataHolderMessageSender : public QObject
{
    Q_OBJECT
public:
    explicit DataHolderMessageSender(const bool &verboseMode, QObject *parent = nullptr);

    struct LastSmartPing
    {
        QString messagetag;
        bool lastOk;
        QString message;

        LastSmartPing() : lastOk(false) {}
        LastSmartPing(const QString &messagetag, const bool &lastOk, const QString &message) : messagetag(messagetag), lastOk(lastOk), message(message) {}
    };

    QReadWriteLock myLock;
    bool verboseMode;

    bool getPingAnswerIsWaiting();
    LastSmartPing getLastPingAnswer();

signals:
    void onThisCommandFailed(QString ruleNameId, QString counterId);

    void append2log(QString message);

    void smartPingTheseHosts(QStringList hosts, QString messagetag);//ask matilda-bbb iface manager to ping , in case of error restart eth0


    //from messageSender


public slots:
    void onThreadStarted();


    void sendAMessageDevMap(QVariantMap mapArgs);

    void onRestartDhcp();

    //from IPC to messageSender
    void smartPingTheseHostsResult(QString messagetag, bool ok, QString message);//direct connection

private:
    bool sendThisDevMap(const QVariantMap &mapArgs, const bool &silent);


    void setLastReceived(QString messagetag, bool ok, QString message);

    void resetPingAnswerIsReceived();

    QByteArray readBashProc(const QString &app, const QStringList &args, const bool &fastRead);

    bool startIPCPingTest(const QString &host);

    void startPingTest();

    void sendEth0Info();


    void restartIface(const bool &hardMode);

    void restartDhcp();


    struct MessageSenderState
    {

        quint16 evntMessangerFailedSendCounter;

        //multi thread access
        bool isWaiting4pingAnswer;
        LastSmartPing lastPing;

        quint16 ipcPingRoundCounter;
        MessageSenderState() : evntMessangerFailedSendCounter(0), isWaiting4pingAnswer(false), ipcPingRoundCounter(0) {}
    } myState;



};

#endif // DATAHOLDERMESSAGESENDER_H
