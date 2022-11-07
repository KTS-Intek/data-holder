#include "matildaconnectionsocket.h"
#include "moji_defy.h"
#include "dbgaboutsourcetype.h"


#include "definedpollcodes.h"
#include "moji_defy.h"

//-------------------------------------------------------------------------------------

MatildaConnectionSocket::MatildaConnectionSocket(bool verboseMode, QObject *parent) :
    RegularLocalSocket(verboseMode, parent)
{
    // dont forget to     void initializeSocket(quint16 mtdExtName);

    hasTmrLater = false;
}
//void ZbyratorSocket::relayStatusChanged(QVariantMap map)
//{
//    mWrite2extension(map, MTD_EXT_ABOUT_LAST_RELAY);
//}

//-------------------------------------------------------------------------------------

void MatildaConnectionSocket::decodeReadData(const QVariant &dataVar, const quint16 &command)
{
    //only commands for zbyrator-bbb
    switch(command){
    //       case MTD_EXT_CUSTOM_COMMAND_0: {
    //           if(verboseMode) qDebug() << "ext " << mtdExtName << dataVar;
    //    #ifdef ENABLE_VERBOSE_SERVER
    //           if(activeDbgMessages)
    //               emit appendDbgExtData(DBGEXT_THELOCALSOCKET, QString("command r: %1, data=%2").arg(command).arg(dataVar.toHash().value("d").toString()));
    //    #endif
    //           emit command4dev(dataVar.toHash().value("c").toUInt(), dataVar.toHash().value("d").toString());
    //           break;}



    default: {
        if(verboseMode)
            qDebug() << "default ext " << command << mtdExtName << dataVar;
        emit onConfigChanged(command,dataVar);
        break;}
    }
}

//-------------------------------------------------------------------------------------

int MatildaConnectionSocket::gimmeAppId4pollCode(const quint16 &pollCode)
{
    if(pollCode >= POLL_CODE_SLEEP_2_METERS && pollCode < POLL_CODE_EXCHANGE_HL8518){
        return MTD_EXT_NAME_ZBYRATOR;
    }

    if(pollCode >= POLL_CODE_FF_READ_LAMP && pollCode < POLL_CODE_SLEEP_2_METERS){
        return MTD_EXT_NAME_FIREFLY_MAIN;
    }
    return MTD_EXT_NAME_ALL;
}

//-------------------------------------------------------------------------------------

void MatildaConnectionSocket::sendCommand2pollDevStr(quint16 pollCode, QString args)
{



    QVariantHash pollargs;
    pollargs.insert("c", int(pollCode));
    pollargs.insert("pc", int(pollCode)); //commnad
    pollargs.insert("d", args); //args

    sendCommand2appHash(pollCode, pollargs, false);

}

//-------------------------------------------------------------------------------------

void MatildaConnectionSocket::sendCommand2pollDevMap(quint16 pollCode, QVariantMap mapArgs)
{

    QVariantHash pollargs;
    pollargs.insert("c", int(pollCode));
    pollargs.insert("pc", int(pollCode)); //commnad
    pollargs.insert("dmap", mapArgs); //args


    sendCommand2appHash(pollCode, pollargs, false);



}

void MatildaConnectionSocket::sendCachedArgs()
{
    if(state() != QLocalSocket::ConnectedState){
        return;
    }
    const QList<quint16> lcodesL = lcodes;
    const QList<QVariantHash> lhashsL = lhashs;

    lcodes.clear();
    lhashs.clear();


    for(int i = 0, imax = lcodesL.size(); i < imax; i++){
        sendCommand2appHash(lcodesL.at(i), lhashsL.at(i), true);
    }
}

bool MatildaConnectionSocket::sendCommand2appHash(quint16 pollCode, QVariantHash hash, bool isCached)
{
    const int appid = gimmeAppId4pollCode(pollCode);
    if(appid == MTD_EXT_NAME_ALL)
        return false;

    if(state() != QLocalSocket::ConnectedState){
        sendItLater(pollCode, hash);
        return false;
    }


    QVariantHash h;
    h.insert("e", appid);//destination
    h.insert("c", MTD_EXT_CUSTOM_COMMAND_0);//command to process on the destination side
    h.insert("d", hash);//poll arguments

    const qint64 r = mWrite2extensionF(QVariant(h), MTD_EXT_COMMAND_2_OTHER_APP);



    if(r > 0){
        return true;
    }
    if(!isCached) //allow to resend only once
        sendItLater(pollCode, hash);

    return false;


}

void MatildaConnectionSocket::sendItLater(quint16 pollCode, QVariantHash hash)
{
    if(!hasTmrLater){
        hasTmrLater = true;
        connect(this, &MatildaConnectionSocket::connected, this, &MatildaConnectionSocket::sendCachedArgs);//coz connection down

        QTimer *tmr = new QTimer(this); //coz write error
        tmr->setInterval(111);

        connect(this, SIGNAL(startTmrArgs()), tmr, SLOT(start()));
        connect(tmr, &QTimer::timeout, this, &MatildaConnectionSocket::sendCachedArgs);


    }

    emit startTmrArgs();

    lcodes.append(pollCode);
    lhashs.append(hash);
}

//-------------------------------------------------------------------------------------