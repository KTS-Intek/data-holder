#include "matildaconnectionsocket.h"
#include <QUrl>

///[!] MatildaIO
#include "matilda-bbb-src/shared/runprocess.h"


#include "moji_defy.h"
#include "dbgaboutsourcetype.h"


#include "definedpollcodes.h"
#include "moji_defy.h"

//-------------------------------------------------------------------------------------

MatildaConnectionSocket::MatildaConnectionSocket(bool verboseMode, QObject *parent) :
    RegularLocalSocket(verboseMode, parent)
{
    // dont forget to     void initializeSocket(quint16 mtdExtName);
    telegramFailedSendCounter = 0;
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


    if(pollCode == 0xFFFF){

        if(!sendCommand2telegramDevMap(mapArgs) ){
            //add some network test
            startPingTest();
            telegramFailedSendCounter++;

            if(telegramFailedSendCounter > 1 && telegramFailedSendCounter < 10){


                switch(telegramFailedSendCounter){
                case 2: restartDhcp(); break;
                case 4: restartIface(false); break;
                case 6: {
                    telegramFailedSendCounter = 400; //do it only once;
                     restartIface(true);
                     break; }
                }


            }
        }else{
            telegramFailedSendCounter = 0;
        }
        //script mode


    }


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

void MatildaConnectionSocket::onRestartDhcp()
{
    restartDhcp();
}

bool MatildaConnectionSocket::sendCommand2telegramDevMap(const QVariantMap &mapArgs)
{

    //        QThread::sleep(5);
    //        const QUrl url(mapArgs.value("__message").toString());
    const QString strArgs = QUrl::toPercentEncoding(mapArgs.value("__message").toString());
    //        qDebug() <<  str << str.toHtmlEscaped() << url.toEncoded() ;

    //        QUrl url;

    emit append2log(tr("ruleId=%1,counterId=%2")
                    .arg(mapArgs.value("__ruleNameId").toString())
                    .arg(mapArgs.value("__counterId").toString()));


    if(strArgs.isEmpty()){
        if(verboseMode)
            qDebug() << "sendCommand2pollDevMap strArgs.isEmpty"  << mapArgs ;
        return true;
    }

    const QByteArray readArr = readBashProc(mapArgs.value("__path").toString(), strArgs.split("\r\n"), false);

    //        const bool r = QProcess::startDetached(mapArgs.value("__path").toString(), strArgs.split("\r\n"));

    if(verboseMode)
        qDebug() << "sendCommand2pollDevMap " << readArr << mapArgs <<  strArgs;

    if(!mapArgs.value("__path").toString().contains("telegram")) //only for telegram sender
        return true;

    if(!readArr.isEmpty()){
        const QJsonObject json = QJsonDocument::fromJson(QString(readArr).split("\r\n\r\n").last().toUtf8()).object();
        //{"ok":true,"result":{"message_id":34,"sender_chat":{"i
        /*
    "HTTP/1.1 200 OK\r\nServer: nginx/1.18.0\r\nDate: Fri, 09 Dec 2022 16:08:41 GMT\r\nContent-Type: application/json\r\nContent-Length: 504\r\nConnection: keep-alive\r\n
    Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\n
    Access-Control-Expose-Headers: Content-Length,Content-Type,Date,Server,Connection\r\n\r\n
    {\"ok\":true,\"result\":{\"message_id\":37,\"sender_chat\":{\"id\":-1001898948682,\"title\":\"AvrZolocheChannel\",\"username\":\"ZolocheAvrChannel\",\"type\":\"channel\"},
    \"chat\":{\"id\":-1001898948682,\"title\":\"AvrZolocheChannel\",\"username\":\"ZolocheAvrChannel\",\"type\":\"channel\"},\"date\":1670602121,
    \"text\":\"\\u041f\\u043b\\u0430\\u0432\\u0434\\u0430\\u0447\\u0456: \\u0416\\u0438\\u0432\\u043b\\u0435\\u043d\\u043d\\u044f \\u0432\\u0456\\u0434 \\u0433
    \\u0435\\u043d\\u0435\\u0440\\u0430\\u0442\\u043e\\u0440\\u0430 \\u0437\\u0430\\u0434\\u0456\\u044f\\u043d\\u043e\"}}
    */
        if(verboseMode)
            qDebug() << "sendCommand2pollDevMap json " << json.isEmpty() << json.value("ok");
        if(!json.isEmpty() && json.value("ok").toBool()){
            emit append2log(tr("telegram send ok, %1").arg(mapArgs.value("__message").toString().left(200)));
            return true;
        }
    }


    //tell to reset the counter
    //    void onThisCommandFailed(QString ruleNameId, QString counterId);

    emit onThisCommandFailed(mapArgs.value("__ruleNameId").toString(), mapArgs.value("__counterId").toString());

    emit append2log(tr("telegram send err, %1").arg(mapArgs.value("__message").toString().left(200)));
    emit append2log(tr("terr=%1").arg(QString(readArr)));// .left(800))));

    return false;
}

QByteArray MatildaConnectionSocket::readBashProc(const QString &app, const QStringList &args, const bool &fastRead)
{

//     return RunProcess::runProc(app, verboseMode, args, fastRead ? 15000 : 25000, "", false, mergedChannels);
    return RunProcess::runBashProcExt(QString("%1 %2").arg(app).arg(args.join(" ")).toUtf8(), verboseMode, fastRead ? 15000 : 25000, false);
}

void MatildaConnectionSocket::startPingTest()
{
    // ping 8.8.8.8 -w 5
    emit append2log(tr("ping=%1").arg(QString(readBashProc("ping",
                                                           QString("8.8.8.8 -w 5 -c 5").split(";"), false))));

}

void MatildaConnectionSocket::sendEth0Info()
{
    emit append2log(tr("iface=%1").arg(QString(readBashProc("ifconfig",
                                                           QString("eth0").split(";"), false))));
}

void MatildaConnectionSocket::restartIface(const bool &hardMode)
{
    sendEth0Info();

//    QString command = "ifdown eth0 && ifup eth0 && date"; it doesn't work as it should
    QString command = "ifconfig eth0 down && ifconfig eth0 up";
    if(hardMode){
        command.prepend("ip addr flush dev eth0 && ");


    }

    emit append2log(tr("reset,isHard=%1,%2").arg(int(hardMode))
                    .arg(QString(readBashProc(command, QStringList(), false).left(1000))));

    QThread::sleep(1);

    sendEth0Info();

}

void MatildaConnectionSocket::restartDhcp()
{
    emit append2log(tr("dhcpcd=%1")
                    .arg(QString(readBashProc("dhclient -r eth0 -v && date", QStringList(), false))));
}

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
