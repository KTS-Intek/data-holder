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

    MyEventsRules fromHashMyEventsRules(const QVariantHash &h);

     QHash<QString,QString> hdataFromOneRecord(const QString &devID, const DHMsecRecord &oneRecord);

signals:
     void sendCommand2pollDevStr(quint16 pollCode, QString args);

     void sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs);


public slots:
    void createLinesIterator();

    void setEventManagerRules(QVariantHash hashRules);

    void checkThisDevice(const quint16 &pollCode, const QString &devID, const DHMsecRecord &oneRecord);

};

#endif // DATAHOLDERSHAREDOBJECTPROCESSOR_H
