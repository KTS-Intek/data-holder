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

    MyEventsRules lastRules;

    QStringList listRulesWithErrors;

    QHash<QString, QHash<QString, quint32> > hRulesCounter; //<ruleName>\n<ruleLine>  to <devId> <counter>


    MyEventsRules fromHashMyEventsRules(const QVariantHash &h);

     QHash<QString,QString> hdataFromOneRecord(const QString &devID, const DHMsecRecord &oneRecord);

     QString getHRulesCounterKey(const MyRuleSettings &ruleSett);

signals:
     void sendCommand2pollDevStr(quint16 pollCode, QString args);

     void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);

     void append2log(QString message);

signals:
     //from iterator
    void gimmeThisDevIDData(QString devID, quint16 pollCode, QString dataKey);

    void gimmeThisAdditionalDevIDData(QString devID, QString additionalDevID, quint16 pollCode, QString dataKey);

    //to iterator
    void setThisDevIDData(QString devID, quint16 pollCode, QString dataKey, QString value);

    void setThisAdditionalDevIDData(QString devID, QString additionalDevID, quint16 pollCode, QString dataKey, QString value);


public slots:
    void createLinesIterator();

    void setEventManagerRules(QVariantHash hashRules);

    void checkThisDevice(const quint16 &pollCode, const QString &devID, const DHMsecRecord &oneRecord);


    void onThisCommandFailed(QString ruleNameId, QString counterId);

};

#endif // DATAHOLDERSHAREDOBJECTPROCESSOR_H
