#ifndef DATAHOLDERTYPES_H
#define DATAHOLDERTYPES_H

#include <QVariantHash>


//<poll code> - it must have only unique NIs
//        <NI> - it can contain only one record
//            <DT msec> <data QVH>

struct DHMsecRecord
{
    qint64 msec;
    QVariantHash hash;
    QString srcname;
    bool wasRestored;

    DHMsecRecord() : msec(0), wasRestored(false) {}

    DHMsecRecord(const qint64 &msec, const QVariantHash &hash, const QString &srcname, const bool &wasRestored)
        : msec(msec), hash(hash), srcname(srcname), wasRestored(wasRestored) {}
};


typedef QHash<QString, DHMsecRecord> DHNI2data; //NI to msec and data

typedef QHash<quint16, DHNI2data> DHDataTable;//poll code to NIs with their data




//typedef QHash<qint64, QVariantHash> DHMsecRecord ; //<DT msec> <data QVH>



#endif // DATAHOLDERTYPES_H
