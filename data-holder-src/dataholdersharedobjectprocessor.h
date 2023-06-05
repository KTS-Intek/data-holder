#ifndef DATAHOLDERSHAREDOBJECTPROCESSOR_H
#define DATAHOLDERSHAREDOBJECTPROCESSOR_H

#include <QObject>

///[!] MatildaIO
#include "conditions-checker/mylinesinterpretator.h"


#include "dataholdertypes.h"


class DataHolderSharedObjectProcessor : public QObject
{
    Q_OBJECT
public:
    explicit DataHolderSharedObjectProcessor(const bool &verboseMode, QObject *parent = nullptr);

    bool verboseMode;
    MyLinesInterpretator *iterator;

    MyMethodsParamsList availableMethods;

    MyEventsRules lastPollRules; //pollcodes != 0

    MyEventsRules lastSystemRules; //pollcode = 0


    QStringList listRulesWithErrors;

    SendMessageProfileMap lastMapProfiles;

    QHash<QString, QHash<QString, quint32> > hRulesCounter; //<ruleName>\n<ruleLine>  to <devId> <counter>


    MyEventsRules fromHashMyEventsRules(const QVariantHash &h, const QVariantHash &hashProfiles);

    SendMessageProfileMap fromSendMessageProfileMap(const QVariantHash &hashProfiles);


    QList<MyExecuteLine> fromStringList(const QStringList &commands2executeStrList, const SendMessageProfileMap &mapProfiles);

    QHash<QString,QString> hdataFromVarHash(const QVariantHash &hash);

     QHash<QString,QString> hdataFromOneRecord(const QString &devID, const DHMsecRecord &oneRecord);

     QHash<QString,QString> hdataFromOnePayload(const QString &who, const QString &evntType, const QVariantHash &payload);


     QString getHRulesCounterKey(const MyRuleSettings &ruleSett);

     QString getHRuleNameFromTheKey(const QString &key);

//     QJsonObject getTelegramJsonSett(const QString &line);

//     MyExecuteLine getExecuteTelegramSett(const QString &line);

signals:
     void sendCommand2pollDevStr(quint16 pollCode, QString args);

     void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);

     void sendAMessageDevMap(QVariantMap mapArgs);

     void addThisDHEvent(QString ruleName, int cntr, quint16 pollCode, QString ruleLine, QString devId, QString additioanlDevId);



     void append2log(QString message);

     //from iterator
    void gimmeThisDevIDData(QString devID, quint16 pollCode, QString dataKey);

    void gimmeThisAdditionalDevIDData(QString devID, QString additionalDevID, quint16 pollCode, QString dataKey);

    //to iterator
    void setThisDevIDData(QString devID, quint16 pollCode, QString dataKey, QString value);

    void setThisAdditionalDevIDData(QString devID, QString additionalDevID, quint16 pollCode, QString dataKey, QString value);


public slots:
    void createLinesIterator();

    void setEventManagerRules(QVariantHash hashRules, QVariantHash hashProfiles);

    void checkThisDeviceNoDataOrModem(const quint16 &pollCode, const QString &devID, const DHMsecRecord &oneRecord, const bool &modemFail);

    void checkThisDevice(const quint16 &pollCode, const QString &devID, const DHMsecRecord &oneRecord);


    void onThisCommandFailed(QString ruleNameId, QString counterId);


    //to dh processor from IPC
    void testThisRule(QString ruleName, QVariantHash oneRuleH);

    void resetThisRules(QStringList ruleNames);

    void smartSystemEvent(QString who, QString evntType, QVariantHash payload);//who app name, evntType logIn,logOut,authFail,appRestart,gsmMoney...

    void sendTestMessage(QString profName, QVariantHash oneProf);

    void smartEvntProcessor(const QString &devIdWho, const QString &additionalIdEvntType, const quint16 &pollCode, const MyRuleSettingsList &listOneCode, QHash<QString,QString> &hdata);


private:
    int executeLines(const QList<MyExecuteLine> &commands2execute, const QString &devID, const QString &ruleNameLineKey, const QString &ruleCounterKey, const QHash<QString, QString> &hdata);

};

#endif // DATAHOLDERSHAREDOBJECTPROCESSOR_H
