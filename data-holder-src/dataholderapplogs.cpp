#include "dataholderapplogs.h"

DataHolderAppLogs::DataHolderAppLogs(const QString &sharedMemoName, const QString &semaName, const QString &write2fileName, const int &delay,
                                     const int &delay2fileMsec, const bool &verboseMode, QObject *parent) :
    SharedMemoWriterAppLogBase(sharedMemoName, semaName, write2fileName, delay, delay2fileMsec, verboseMode, parent)
{

}
