#ifndef DATAHOLDERAPPLOGS_H
#define DATAHOLDERAPPLOGS_H

///[!] sharedmemory
#include "src/shared/sharedmemowriterapplogbase.h"

//let it be, I will add something later

class DataHolderAppLogs : public SharedMemoWriterAppLogBase
{
    Q_OBJECT
public:
    explicit DataHolderAppLogs(const QString &sharedMemoName, const QString &semaName, const QString &write2fileName,
                               const int &delay, const int &delay2fileMsec, const bool &verboseMode, QObject *parent = nullptr);




signals:


public slots:
    void addThisDHEvent(QString ruleName, int cntr, QString ruleLine, QString devId, QString additioanlDevId);

};

#endif // DATAHOLDERAPPLOGS_H
