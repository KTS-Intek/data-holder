#include "dataholderapplogs.h"

DataHolderAppLogs::DataHolderAppLogs(const QString &sharedMemoName, const QString &semaName, const QString &write2fileName, const int &delay,
                                     const int &delay2fileMsec, const bool &verboseMode, QObject *parent) :
    SharedMemoWriterAppLogBase(sharedMemoName, semaName, write2fileName, delay, delay2fileMsec, verboseMode, parent)
{

}

void DataHolderAppLogs::addThisDHEvent(QString ruleName, int cntr, QString ruleLine, QString devId, QString additioanlDevId)
{
    QStringList l;
    l.append(ruleName);
    l.append(QString::number(cntr));
    l.append(ruleLine);
    l.append(devId);
    l.append(additioanlDevId);

    add2systemLogWarn(l.join("\t"));

}
