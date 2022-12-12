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

    void onThisCommandFailed(QString ruleNameId, QString counterId);

    void append2log(QString message);

public slots:
    //for client side



    void sendCommand2pollDevStr(quint16 pollCode, QString args);

    void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);



    void sendCachedArgs();


    void onRestartDhcp();

private:
    bool sendCommand2telegramDevMap(const QVariantMap &mapArgs);

    QByteArray readBashProc(const QString &app, const QStringList &args, const bool &fastRead);

    void startPingTest();

    void sendEth0Info();


    void restartIface(const bool &hardMode);

    void restartDhcp();


    bool sendCommand2appHash(quint16 pollCode, QVariantHash hash, bool isCached);

    void sendItLater(quint16 pollCode, QVariantHash hash);

    QList<quint16> lcodes;
    QList<QVariantHash> lhashs;

    quint16 telegramFailedSendCounter;
};

#endif // MATILDACONNECTIONSOCKET_H
