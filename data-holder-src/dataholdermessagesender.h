#ifndef DATAHOLDERMESSAGESENDER_H
#define DATAHOLDERMESSAGESENDER_H

#include <QObject>
#include <QtCore>

class DataHolderMessageSender : public QObject
{
    Q_OBJECT
public:
    explicit DataHolderMessageSender(const bool &verboseMode, QObject *parent = nullptr);

    bool verboseMode;

signals:
    void onThisCommandFailed(QString ruleNameId, QString counterId);

    void append2log(QString message);

    void smartPingTheseHosts(QStringList hosts, QString messagetag);//ask matilda-bbb iface manager to ping , in case of error restart eth0, it will be added later


public slots:
    void onThreadStarted();

    void sendAMessageDevMap(QVariantMap mapArgs, QString messageClientName);

    void onRestartDhcp();


private:
    bool sendThis2telegramDevMap(const QVariantMap &mapArgs, const bool &silent);

    QByteArray readBashProc(const QString &app, const QStringList &args, const bool &fastRead);

    void startPingTest();

    void sendEth0Info();


    void restartIface(const bool &hardMode);

    void restartDhcp();

    quint16 telegramFailedSendCounter;

};

#endif // DATAHOLDERMESSAGESENDER_H
