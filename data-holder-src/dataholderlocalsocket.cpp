#include "dataholderlocalsocket.h"

#include "dataholderlocalservercommands.h"


//--------------------------------------------------------------------------------------

DataHolderLocalSocket::DataHolderLocalSocket(DataHolderSharedObject *dhData, const bool &verboseMode, QObject *parent) :
    RegularServerSocket(verboseMode, parent)
{
    this->dhData = dhData;
}

//--------------------------------------------------------------------------------------

quint16 DataHolderLocalSocket::getZombieCommand()
{
    return DATAHOLDER_PING;
}

//--------------------------------------------------------------------------------------

QVariantHash DataHolderLocalSocket::getOkMessage(const QVariant &messagetag, const QVariant &objecttag)
{
    QVariantHash h;
    h.insert("result", DATAHOLER_RESULT_OK);
    h.insert("message", "ok");
    h.insert("messagetag", messagetag);
    h.insert("objecttag", objecttag);
    return h;
}

//--------------------------------------------------------------------------------------

QVariantHash DataHolderLocalSocket::getErrorMessage(const QString &message, const QVariant &messagetag, const QVariant &objecttag)
{
    QVariantHash h;
    h.insert("result", DATAHOLER_RESULT_ERROR);
    h.insert("message", message);
    h.insert("messagetag", messagetag);
    h.insert("objecttag", objecttag);

    return h;
}

//--------------------------------------------------------------------------------------

void DataHolderLocalSocket::configureZombieKiller()
{
    connect(this, &DataHolderLocalSocket::onReadData, this, &DataHolderLocalSocket::decodeReadData);
    initObjects();
    mWrite2extension(QByteArray("h"), DATAHOLDER_GET_INFO);//отримати назву пристрою
}

//--------------------------------------------------------------------------------------

void DataHolderLocalSocket::decodeReadData(const QVariant &dataVar, const quint16 &command)
{
    if(command != DATAHOLDER_PING_2_SERV && verboseMode)
        qDebug() << "read " << command << dataVar << dataVar.isValid() << mtdExtNameTxt;

    if(!dataVar.isValid())
        return;

//#define DATAHOLDER_GET_INFO        1 //GET A CLIENT NAME

//#define DATAHOLDER_PING             9 //FROM CLIENT TO SERVER
//#define DATAHOLDER_PING_2_SERV      16 //FROM SERVER TO CLIENT


//#define DATAHOLDER_ADD_POLLDATA     30 //A CLIENT SENDS DATA
//#define DATAHOLDER_GET_POLLDATA     31 //A CLIENT ASKS FOR DATA

    QVariantHash writeHash;

    switch(command){
    case DATAHOLDER_GET_INFO: {
        if(mtdExtNameTxt.isEmpty()){
            mtdExtNameTxt = dataVar.toString();
        }else{
            close();
        }
        break; }

    case DATAHOLDER_PING: {  onPingReceived(); break;}
    case DATAHOLDER_PING_2_SERV: { onPing2serverReceived(); break;}


    case DATAHOLDER_ADD_POLLDATA: {
        writeHash = onDATAHOLDER_ADD_POLLDATA(dataVar.toHash());
        break;}
    case DATAHOLDER_GET_POLLDATA: {
        writeHash = onDATAHOLDER_GET_POLLDATA(dataVar.toHash());
        break;}
    }

    if(!writeHash.isEmpty())
        mWrite2extensionLater(writeHash, command);

}

void DataHolderLocalSocket::addDataFromTheFile(const QVariantHash &hash)
{
    //for test only, do not use it in production
    onDATAHOLDER_ADD_POLLDATA(hash);
}

//--------------------------------------------------------------------------------------

QVariantHash DataHolderLocalSocket::onDATAHOLDER_ADD_POLLDATA(const QVariantHash &hash)
{
    const QVariant messagetag = hash.value("messagetag");
    const QVariant objecttag = hash.value("objecttag");

    if(mtdExtNameTxt.isEmpty())
        return getErrorMessage("you have to register first", messagetag, objecttag); //it is not registered

/* there are 4 keys
 * pollCode
 * NI
 * msec
 * data
*/

    const quint16 pollCode = hash.value("pollCode").toUInt();
    const QString ni = hash.value("NI").toString();
    const qint64 msec = hash.value("msec").toLongLong();
    const QVariantHash h = hash.value("data").toHash();

    if(verboseMode)
        qDebug() << "onDATAHOLDER_ADD_POLLDATA " << mtdExtNameTxt << ni
                 << pollCode
                 << msec
                 << QDateTime::fromMSecsSinceEpoch(msec).toString("yyyy-MM-dd hh:mm:ss.zzz");



    if(pollCode > 0 && !ni.isEmpty() && msec > 0 && !h.isEmpty()){
        dhData->addRecord(pollCode, ni, msec, h, mtdExtNameTxt);

//for test only, to add values to the file
//#ifndef __x86_64
//        appendData2file(hash);
//#endif
        return getOkMessage(messagetag, objecttag);
    }

    return getErrorMessage(
                QString("your data is bad, pollCode='%1', ni='%2', msec='%3', h.isEmpty='%4'")
                .arg(pollCode).arg(ni).arg(msec).arg(QString::number(h.isEmpty())), messagetag, objecttag);
}

//--------------------------------------------------------------------------------------

QVariantHash DataHolderLocalSocket::onDATAHOLDER_GET_POLLDATA(const QVariantHash &hash)
{
//    if(mtdExtNameTxt.isEmpty())
//        return getErrorMessage("you have to register first"); //it is not registered
    const QVariant messagetag = hash.value("messagetag");
    const QVariant objecttag = hash.value("objecttag");


    bool hasPollCode;
    const quint16 pollCode = hash.value("pollCode").toUInt(&hasPollCode);


    const QString ni = hash.value("NI").toString();
    const bool hasNI = !ni.isEmpty();


    if(verboseMode)
        qDebug() << "onDATAHOLDER_GET_POLLDATA " << mtdExtNameTxt << messagetag << objecttag << pollCode << ni;

//    const qint64 msec = hash.value("msec").toLongLong();
//    const QVariantHash h = hash.value("data").toHash();
    if(pollCode > 0){
        QVariantList varlist;
        if(hasNI){
            const DHMsecRecord pollCodeData = dhData->getLastRecord(pollCode, ni);
            varlist.append(getHashRecord(pollCode, ni, pollCodeData));
        }else{
            const DHNI2data pollCodeData = dhData->getPollCodeData(pollCode);
            QList<QString> lk = pollCodeData.keys();
            std::sort(lk.begin(), lk.end());

            for(int i = 0, imax = lk.size(); i < imax; i++)
                varlist.append(getHashRecord(pollCode, ni, pollCodeData.value(ni)));

        }



        QVariantHash writehash = getOkMessage(messagetag, objecttag);
        writehash.insert("varlist", varlist);

        return writehash;
    }

    return getErrorMessage(
                QString("your data is bad, pollCode='%1', ni='%2', hasPollCode='%3', hasNI='%4'")
                .arg(pollCode).arg(ni).arg(QString::number(hasPollCode))
                .arg(QString::number(hasNI)), messagetag, objecttag);
}

//--------------------------------------------------------------------------------------

QVariantHash DataHolderLocalSocket::getHashRecord(const quint16 &pollCode, const QString &ni, const DHMsecRecord &pollCodeData)
{
    QVariantHash h;
    //main
    h.insert("pollCode", pollCode);
    h.insert("NI", ni);
    h.insert("msec", pollCodeData.msec);
    h.insert("data", pollCodeData.hash);
    //optional
    h.insert("src", pollCodeData.srcname);
    h.insert("restore", pollCodeData.wasRestored);
    return h;
}

//--------------------------------------------------------------------------------------

void DataHolderLocalSocket::appendData2file(const QVariantHash &hash)
{
    QJsonObject json = QJsonObject::fromVariantHash(hash);
    json.insert("srcname", mtdExtNameTxt);

    QJsonArray arr = getFileContent();
    arr.append(json);

    qDebug() << "DataHolderLocalSocket file add " << json;



    QSaveFile sfile;
    sfile.setFileName("dataholder.data");
    if(sfile.open(QSaveFile::WriteOnly)){
        sfile.write(QJsonDocument(arr).toJson());
        const bool r = sfile.commit();
        qDebug() << "DataHolderLocalSocket file saved " << r << arr.size();
    }

}

//--------------------------------------------------------------------------------------

QJsonArray DataHolderLocalSocket::getFileContent()
{
    QFile file;
    file.setFileName("dataholder.data");
    if(file.open(QFile::ReadOnly)){
        const QByteArray readarr = file.readAll();
        QJsonArray jsonarr = QJsonDocument::fromJson(readarr).array();
        return jsonarr;
    }
    return QJsonArray();
}

//--------------------------------------------------------------------------------------
