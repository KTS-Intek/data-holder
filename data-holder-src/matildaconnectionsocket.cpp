#include "matildaconnectionsocket.h"
#include <QUrl>


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

//    QTimer::singleShot(9999, this, SLOT(onRestartDhcp())); for test only

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

    case MTD_EXT_COMMAND_2_DHP_SYSEVNTS:{

        const QVariantHash h = dataVar.toHash();
        if(!h.isEmpty() && !h.value("who").toString().isEmpty())
            emit smartSystemEvent(h.value("who").toString(), h.value("evntType").toString(), h.value("payload").toHash());//who app name, evntType logIn,logOut,authFail,appRestart,gsmMoney...


        break;}

    case MTD_EXT_COMMAND_2_SMARTPING:{
        const QVariantHash h = dataVar.toHash();
//        h.insert("messagetag", messagetag);
//        h.insert("ok", ok);
//        h.insert("message", message);

        emit smartPingTheseHostsResult(h.value("messagetag").toString(), h.value("ok").toBool(), h.value("message").toString());

        break;}

    case MTD_EXT_CUSTOM_COMMAND_0:{
        //rule test
        const QVariantHash h = dataVar.toHash();
           //h.name - a rule name
           //h.rule - rule settings
        emit testThisRule(h.value("name").toString(), h.value("rule").toHash());
        break;}


    case MTD_EXT_CUSTOM_COMMAND_1:{
        //inData, rule reset
            const QVariantHash h = dataVar.toHash();
            //h.name - a rule name if empty all
        emit resetThisRules(h.value("names").toStringList());
        break;}


    case MTD_EXT_CUSTOM_COMMAND_2:{
        //inData, rule reset
            const QVariantHash h = dataVar.toHash();
            //h.name - a rule name if empty all

//            qDebug() << "MTD_EXT_CUSTOM_COMMAND_2 " << h;

            emit sendTestMessage(h.value("name").toString(), h.value("prof").toHash());
        break;}


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


    if(verboseMode)
        qDebug() << "sendCommand2pollDevMap pollCode" << pollCode  << mapArgs;


    QVariantHash pollargs;
    pollargs.insert("c", int(pollCode));
    pollargs.insert("pc", int(pollCode)); //commnad
    pollargs.insert("dmap", mapArgs); //args


    sendCommand2appHash(pollCode, pollargs, false);



}

//-------------------------------------------------------------------------------------

void MatildaConnectionSocket::smartPingTheseHosts(QStringList hosts, QString messagetag)
{
//    const QVariantHash h = dataVar.toHash();
//    QStringList hosts = h.value("hosts").toStringList();
//    QString messagetag = h.value("messagetag").toString();

//    emit smartPingTheseHosts(hosts, messagetag, mtdExtName);

    if(hosts.isEmpty() || messagetag.isEmpty()){
        emit smartPingTheseHostsResult(messagetag, false, "command is not sent: check hosts and messagetag");
        return;
    }


    QVariantHash h;
    h.insert("hosts", hosts);
    h.insert("messagetag", messagetag);

    mWrite2extension(h, MTD_EXT_COMMAND_2_SMARTPING);

}


//-------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------

bool MatildaConnectionSocket::sendCommand2appHash(quint16 pollCode, QVariantHash hash, bool isCached)
{
    const int appid = gimmeAppId4pollCode(pollCode);
    if(appid == MTD_EXT_NAME_ALL)
        return false;

    if(state() != QLocalSocket::ConnectedState){
        emit append2log(tr("send later, app=%1,code=%2")
                        .arg(appid)
                        .arg(hash.value("pc").toInt()));
        sendItLater(pollCode, hash);
        return false;
    }


    emit append2log(tr("send now app=%1,code=%2,args=%3")
                    .arg(appid)
                    .arg(hash.value("pc").toInt())
                    .arg((hash.contains("d") ? hash.value("d").toString() : QString("size is %1").arg(hash.value("dmap").toMap().size())))
                    );

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

//-------------------------------------------------------------------------------------

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
