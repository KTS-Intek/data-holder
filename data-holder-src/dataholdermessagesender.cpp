#include "dataholdermessagesender.h"

#include <QUrl>


///[!] MatildaIO
#include "matilda-bbb-src/shared/runprocess.h"
//do not use it to reset eth0

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
    emit append2log(tr("Event Messanger client is ready"));
}



//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::sendAMessageDevMap(QVariantMap mapArgs)
{

    emit append2log(tr("sendAMessageDevMap %1").arg(mapArgs.value("__path").toString().split("/").last()));


//    if(messageClientName != "telegram")
//        return;
    const bool isTelegram = mapArgs.value("__path").toString().contains("telegram");
    if(!isTelegram)
        return;

    if(sendThisDevMap(mapArgs, true) ){
//        emit append2log(tr("DataHolderMessageSender::sendAMessageDevMap 103 askReset=%1, evntMessangerFailedSendCounter=%2").arg("-").arg(int(myState.evntMessangerFailedSendCounter)));

        myState.evntMessangerFailedSendCounter = 0;
        return;
    }

    if(myState.ifaceRestartCounter == myState.evntMessangerFailedSendCounter && myState.evntMessangerFailedSendCounter == 0){
        myState.evntMessangerFailedSendCounter = 2; //force to reset the interface
//        emit append2log(tr("DataHolderMessageSender::sendAMessageDevMap 69 ifaceRestartCounter=%1, evntMessangerFailedSendCounter=%2").arg(int(myState.ifaceRestartCounter)).arg(int(myState.evntMessangerFailedSendCounter)));

    }

    for(int i = 0; i < 9; i++){


        //        emit smartPingTheseHosts(QString("api.telegram.org").split(" "), "some tag here to accecpt the result"); wait 10-15 sec, and try again


        myState.evntMessangerFailedSendCounter++;

        //        if(myState.evntMessangerFailedSendCounter < 3){ //for testing, if it works, remove the counter


        quint8 askReset = 0;



        if(myState.evntMessangerFailedSendCounter > 1 && myState.evntMessangerFailedSendCounter < 10){


            switch(myState.evntMessangerFailedSendCounter){
            //                case 2: restartDhcp(); break;
            case 3: askReset = 1; break; // restartIface(false); break;
            case 5: {
                myState.evntMessangerFailedSendCounter = 0;//as many as it needs
                askReset = 2;
                //                    restartIface(true);
                break; }
            }
        }



        QElapsedTimer tmr;
        tmr.start();
        //use matilda bbb to reset the interface
        if( startIPCPingTest("api.telegram.org", askReset) && sendThisDevMap(mapArgs, false)){ //isTelegram
            emit append2log(tr("DataHolderMessageSender::sendAMessageDevMap 96 askReset=%1, evntMessangerFailedSendCounter=%2").arg(askReset).arg(int(myState.evntMessangerFailedSendCounter)));

            myState.evntMessangerFailedSendCounter = 0;
            return;
        }

        if(tmr.elapsed() < 15000)
            QThread::sleep(5);
        if(askReset > 0){
            myState.ifaceRestartCounter++;
            QThread::sleep(10); //wait a moment after reset
        }


        emit append2log(tr("DataHolderMessageSender::sendAMessageDevMap 102 i=%1, evntMessangerFailedSendCounter=%2").arg(i).arg(int(myState.evntMessangerFailedSendCounter)));





    }


}

//-----------------------------------------------------------------------------------------

void DataHolderMessageSender::smartPingTheseHostsResult(QString messagetag, bool ok, QString message)
{
    setLastReceived(messagetag, ok, message);
    //it is called from other thread
}


//-----------------------------------------------------------------------------------------

bool DataHolderMessageSender::sendThisDevMap(const QVariantMap &mapArgs, const bool &silent)
{
    //        QThread::sleep(5);
    //        const QUrl url(mapArgs.value("__message").toString());

//path 2 script    json.insert("__path", mapProfiles.value(ll.at(1)).fPath2script);
//script arguments    json.insert("__args", mapProfiles.value(ll.at(1)).args);
//message, html encoded    json.insert("__message", line.mid(indxFrom) );

    emit append2log(tr("DataHolderMessageSender::sendThisDevMap args=%1, silent=%2").arg(mapArgs.value("__args").toString()).arg(int(silent)));


    QStringList strArgs;
    if(!mapArgs.value("__args").toString().isEmpty())
        strArgs.append(mapArgs.value("__args").toString());

    if(!mapArgs.value("__message").toString().isEmpty())
        strArgs.append(QUrl::toPercentEncoding(mapArgs.value("__message").toString()));
    //        qDebug() <<  str << str.toHtmlEscaped() << url.toEncoded() ;

    //        QUrl url;

    emit append2log(tr("ruleId=%1,counterId=%2")
                    .arg(mapArgs.value("__ruleNameId").toString())
                    .arg(mapArgs.value("__counterId").toString()));


    if(strArgs.isEmpty()){
        if(verboseMode)
            qDebug() << "sendCommand2pollDevMap strArgs.isEmpty"  << mapArgs ;


        emit append2log(tr("DataHolderMessageSender::sendThisDevMap strArgs=%1, isEmpty=%2").arg(strArgs.join("\n")).arg(int(strArgs.isEmpty())));

        return true;
    }

    const QByteArray readArr = readBashProc(mapArgs.value("__path").toString(), strArgs, false);

    //        const bool r = QProcess::startDetached(mapArgs.value("__path").toString(), strArgs.split("\r\n"));

    if(verboseMode)
        qDebug() << "sendCommand2pollDevMap " << readArr << mapArgs <<  strArgs;

    if(!mapArgs.value("__path").toString().contains("telegram")){ //only for telegram sender

        emit append2log(tr("DataHolderMessageSender::sendThisDevMap path=%1, contains=%2")
                        .arg(mapArgs.value("__path").toString()).arg(int(mapArgs.value("__path").toString().contains("telegram"))));

        return true;
    }

    if(!readArr.isEmpty()){
        //telegram send check, add for others, later
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
            emit append2log(tr("messanger send ok, %1").arg(mapArgs.value("__message").toString().left(200)));
            return true;
        }
    }

    if(!silent){

        //tell to reset the counter
        //    void onThisCommandFailed(QString ruleNameId, QString counterId);

        emit onThisCommandFailed(mapArgs.value("__ruleNameId").toString(), mapArgs.value("__counterId").toString());

    }
    emit append2log(tr("messanger send err, %1").arg(mapArgs.value("__message").toString().left(200)));
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

    return RunProcess::runBashProcExt(QString("%1 %2").arg(app).arg(args.join(" ")).toUtf8(), verboseMode, fastRead ? 5000 : 15000, false);
}

//-----------------------------------------------------------------------------------------

bool DataHolderMessageSender::startIPCPingTest(const QString &host, const quint8 &askReset)
{
    resetPingAnswerIsReceived();

    emit append2log(tr("IPC ping %1, reset %2").arg(host).arg(int(askReset)));

    const QString messagetag = QString("DHMessageSender-%1-%2")
            .arg(QString::number(myState.ipcPingRoundCounter++))
            .arg(QDateTime::currentMSecsSinceEpoch());

    QElapsedTimer time;
    time.start();
    emit smartPingTheseHosts(host.split(" "), messagetag, askReset);

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
