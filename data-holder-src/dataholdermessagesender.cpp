#include "dataholdermessagesender.h"

#include <QUrl>


///[!] MatildaIO
#include "matilda-bbb-src/shared/runprocess.h"

//-----------------------------------------------------------------------------------------

DataHolderMessageSender::DataHolderMessageSender(const bool &verboseMode, QObject *parent)
    : QObject(parent), verboseMode(verboseMode)
{

}

//-----------------------------------------------------------------------------------------

bool DataHolderMessageSender::getPingAnswerIsWaiting()
{

    QReadLocker locker(&myLock);
    const auto r = myState.isWaiting4pingAnswer;
    return r;
}

//-----------------------------------------------------------------------------------------

DataHolderMessageSender::LastSmartPing DataHolderMessageSender::getLastPingAnswer()
{
    QReadLocker locker(&myLock);
    const auto r = myState.lastPing;
    return r;
}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::onThreadStarted()
{
    emit append2log(tr("Telegram client is ready"));
}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::sendAMessageDevMap(QVariantMap mapArgs, QString messageClientName)
{

    emit append2log(tr("sendAMessageDevMap %1").arg(messageClientName));


    if(messageClientName != "telegram")
        return;

    if(!sendThis2telegramDevMap(mapArgs, true) ){
        //        emit smartPingTheseHosts(QString("api.telegram.org").split(" "), "some tag here to accecpt the result"); wait 10-15 sec, and try again


        myState.telegramFailedSendCounter++;

        if(myState.telegramFailedSendCounter < 3){ //for testing, if it works, remove the counter
            //use matilda bbb to reset the interface
            if(startIPCPingTest("api.telegram.org") && sendThis2telegramDevMap(mapArgs, false)){
                myState.telegramFailedSendCounter = 0;
                return;
            }



        }else{

            //it shouldn't be here, it is only for emergency


//            //add some network test
//            startPingTest();

//            if(myState.telegramFailedSendCounter == 1){
//                restartIface(false);
//                startPingTest();
//                if(sendThis2telegramDevMap(mapArgs, false) ){
//                    telegramFailedSendCounter = 0;
//                    return;
//                }
//            }


            if(myState.telegramFailedSendCounter > 1 && myState.telegramFailedSendCounter < 10){


                switch(myState.telegramFailedSendCounter){
                //                case 2: restartDhcp(); break;
                case 3: restartIface(false); break;
                case 5: {
                    myState.telegramFailedSendCounter = 400; //do it only once;
                    restartIface(true);
                    break; }
                }


            }


        }

    }else{
        myState.telegramFailedSendCounter = 0;
    }

}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::onRestartDhcp()
{
    restartDhcp();

}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::smartPingTheseHostsResult(QString messagetag, bool ok, QString message)
{
    setLastReceived(messagetag, ok, message);
    //it is called from other thread
}


//-----------------------------------------------------------------------------------------

bool DataHolderMessageSender::sendThis2telegramDevMap(const QVariantMap &mapArgs, const bool &silent)
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

    if(!silent){

        //tell to reset the counter
        //    void onThisCommandFailed(QString ruleNameId, QString counterId);

        emit onThisCommandFailed(mapArgs.value("__ruleNameId").toString(), mapArgs.value("__counterId").toString());

    }
    emit append2log(tr("telegram send err, %1").arg(mapArgs.value("__message").toString().left(200)));
    emit append2log(tr("terr=%1").arg(QString(readArr)));// .left(800))));

    return false;
}


//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::setLastReceived(QString messagetag, bool ok, QString message)
{
    //I remember something like that  didn't work without this shit
    if(true){
        QWriteLocker locker(&myLock);
        myState.lastPing = LastSmartPing(messagetag, ok, message);
        myState.isWaiting4pingAnswer = false;
    }
}


//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::resetPingAnswerIsReceived()
{
    if(true){
        QWriteLocker locker(&myLock);
        myState.isWaiting4pingAnswer = true;
        myState.lastPing = LastSmartPing();
    }
}

//-----------------------------------------------------------------------------------------

QByteArray DataHolderMessageSender::readBashProc(const QString &app, const QStringList &args, const bool &fastRead)
{

    //     return RunProcess::runProc(app, verboseMode, args, fastRead ? 15000 : 25000, "", false, mergedChannels);
    return RunProcess::runBashProcExt(QString("%1 %2").arg(app).arg(args.join(" ")).toUtf8(), verboseMode, fastRead ? 5000 : 15000, false);
}

//-----------------------------------------------------------------------------------------

bool DataHolderMessageSender::startIPCPingTest(const QString &host)
{
    resetPingAnswerIsReceived();

    emit append2log(tr("IPC ping %1").arg(host));

    const QString messagetag = QString("DHMessageSender-%1-%2")
            .arg(QString::number(myState.ipcPingRoundCounter++))
            .arg(QDateTime::currentMSecsSinceEpoch());

    QElapsedTimer time;
    time.start();
    emit smartPingTheseHosts(host.split(" "), messagetag);

    for(int i = 0; i < 1000 && time.elapsed() < 60000; i++){
        QThread::msleep(100);
        if(!getPingAnswerIsWaiting() && getLastPingAnswer().messagetag == messagetag)
            break;
    }

    const auto lastPing = getLastPingAnswer();

    if(lastPing.messagetag != messagetag){
        emit append2log(tr("IPC ping, bad message tag is received").arg(lastPing.messagetag));
    }

    if(lastPing.lastOk){
        emit append2log(tr("IPC ping, success: %1").arg(lastPing.message));
    }else{
        emit append2log(tr("IPC ping, failed: %1").arg(lastPing.message));
    }

    return lastPing.lastOk;

}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::startPingTest()
{
    // ping 8.8.8.8 -w 5
    //    emit append2log(tr("ping=%1,%2").arg(QString::number(telegramFailedSendCounter)).arg(QString(readBashProc("ping",
    //                                                           QString("8.8.8.8 -w 5 -c 5").split(";"), true))));
    //ping telegram, dns check
    emit append2log(tr("ping=%1,%2")
                    .arg(QString::number(myState.telegramFailedSendCounter))
                    .arg(QString(readBashProc("ping",
                                              QString("api.telegram.org -w 5 -c 5").split(";"), true))));

}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::sendEth0Info()
{
    emit append2log(tr("iface=%1").arg(QString(readBashProc("ifconfig",
                                                            QString("eth0").split(";"), false))));
}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::restartIface(const bool &hardMode)
{
    sendEth0Info();

    //    QString command = "ifdown eth0 && ifup eth0 && date"; it doesn't work as it should
    QString command = "ifconfig eth0 down && ifconfig eth0 up";
    if(hardMode){
        command.prepend("ip addr flush dev eth0 && ");


    }

    emit append2log(tr("reset,isHard=%1,%2").arg(int(hardMode))
                    .arg(QString(readBashProc(command, QStringList(), false).left(1000))));

    QThread::sleep(3);

    sendEth0Info();
}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::restartDhcp()
{
    emit append2log(tr("dhcpcd=%1")
                    .arg(QString(readBashProc("dhclient -r eth0 -v && date", QStringList(), false))));
}

//-----------------------------------------------------------------------------------------

