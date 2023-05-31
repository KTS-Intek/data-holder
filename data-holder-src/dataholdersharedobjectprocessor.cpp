#include "dataholdersharedobjectprocessor.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>

#include "definedpollcodes.h"

//----------------------------------------------------------------------------------------

DataHolderSharedObjectProcessor::DataHolderSharedObjectProcessor(const bool &verboseMode, QObject *parent) :
    QObject(parent), verboseMode(verboseMode)
{

}

//----------------------------------------------------------------------------------------

MyEventsRules DataHolderSharedObjectProcessor::fromHashMyEventsRules(const QVariantHash &h, const QVariantHash &hashProfiles)
{
    //    ui->plainTextEdit_2->appendPlainText(tr("Out is - '%1'").arg(out));

    //    QVariantHash oneh;
    //    oneh.insert("r", oneRule.ruleLine);
    //    oneh.insert("c", oneRule.commands2execute);
    //    oneh.insert("pc", oneRule.pollCode);
    //    h.insert(ruleName, oneh);

    lastMapProfiles = fromSendMessageProfileMap(hashProfiles);

    MyEventsRules out;
    auto lk = h.keys();

    std::sort(lk.begin(), lk.end());


    auto hRulesCounterL = hRulesCounter;
    hRulesCounter.clear();//to remove missing rules

    for(int i = 0, imax = lk.size(); i < imax; i++){
        const QString ruleName = lk.at(i);
        const auto oneh = h.value(ruleName).toHash();

        MyRuleSettings oneRule;

        oneRule.ruleName = ruleName;
        oneRule.ruleLine = oneh.value("r").toString();

        oneRule.limitExecutions = oneh.value("cntr", 0).toUInt();
        oneRule.disable = oneh.value("dsbl", false).toBool();

        if(oneRule.disable)
            continue;//rule is disabled, so ignore it

        const auto commands2execute = oneh.value("c").toStringList();

        if(hRulesCounterL.contains(getHRulesCounterKey(oneRule)))
            hRulesCounter.insert(getHRulesCounterKey(oneRule), hRulesCounterL.value(getHRulesCounterKey(oneRule)));

        oneRule.commands2execute = fromStringList(commands2execute, lastMapProfiles);


        if(oneRule.commands2execute.isEmpty())
            continue;//bad settings, ignore them

        const quint16  pollCode = oneh.value("pc").toUInt();
        MyRuleSettingsList l = out.value(pollCode);
        l.append(oneRule);

        out.insert(pollCode, l);//0 - system events
    }

    return out;
}

//----------------------------------------------------------------------------------------

SendMessageProfileMap DataHolderSharedObjectProcessor::fromSendMessageProfileMap(const QVariantHash &hashProfiles)
{
    SendMessageProfileMap out;
    const auto lk = hashProfiles.keys();
    for(int i = 0, imax = lk.size(); i < imax; i++){
        const QString name = lk.at(i);
        const QVariantHash oneProf = hashProfiles.value(name).toHash();
        if(oneProf.isEmpty())
            continue;

        SendMessageProfile onesett;
        const quint16 type = oneProf.value("t").toUInt();

        switch(type){
        case 1: onesett.fPath2script = "/opt/matilda/script/telegram.sh"; break; //telegram
        }

        if(onesett.fPath2script.isEmpty())
            continue;
        onesett.args = oneProf.value("a").toString();
        out.insert(name, onesett);
    }
    return out;
}

//----------------------------------------------------------------------------------------

QList<MyExecuteLine> DataHolderSharedObjectProcessor::fromStringList(const QStringList &commands2executeStrList, const SendMessageProfileMap &mapProfiles)
{
    QList<MyExecuteLine> commands2execute;
    for(int j = 0, jmax = commands2executeStrList.size(); j < jmax; j++){

        const QString line = commands2executeStrList.at(j); //<poll code><space><arguments> or path to script, so check both

        const int indxFrom = line.indexOf(" ");

        if(indxFrom < 0)
            continue;

        MyExecuteLine oneLineSett;
        oneLineSett.line = line.mid(indxFrom + 1);
        oneLineSett.command = line.left(indxFrom).toUInt();


        //arguments are necessary
        if(oneLineSett.line.isEmpty())
            continue;


        if(oneLineSett.command < POLL_CODE_FF_READ_LAMP ){
            //<sendMessage><space><send prof name><space><message>
            const QStringList ll = line.split(" ", QString::SkipEmptyParts);
            if(ll.size() > 2 && ll.at(0) == "sendMessage" && mapProfiles.contains(ll.at(1))){
                //ll.at(0) - sendMessage
//                indxFrom = line.indexOf(ll.at(1)) + ll.at(1).length() + 1; //indx + len + spacelen
                QJsonObject json;
                json.insert("__path", mapProfiles.value(ll.at(1)).fPath2script);
                json.insert("__args", mapProfiles.value(ll.at(1)).args);
                json.insert("__message", line.mid(line.indexOf(ll.at(1)) + ll.at(1).length() + 1) );


                oneLineSett.line = QJsonDocument(json).toJson(QJsonDocument::Compact);
                oneLineSett.isJson = true;
                oneLineSett.command = 0xFFFF;//send message

                commands2execute.append(oneLineSett);

                continue;
            }
            emit append2log(tr("Bad rule %1").arg(line));

//            //script mode, obsolete mode
//            const QString path2script = line.left(indxFrom);

//            const QFileInfo fi(path2script);
//            if(fi.exists() && fi.isExecutable()){
//                QJsonObject json;
//                json.insert("__path", path2script);
//                json.insert("__message",oneLineSett.line );


//                oneLineSett.line = QJsonDocument(json).toJson(QJsonDocument::Compact);

//                oneLineSett.isJson = true;
//                oneLineSett.command = 0xFFFF;//send message

//                commands2execute.append(oneLineSett);




//                if(verboseMode){
//                    qDebug() << "fromHashMyEventsRules script is found " << path2script <<  line.mid(indxFrom + 1);

//                    //                        const QUrl url(line.mid(indxFrom + 1));
//                    //                        const QString strArgs = url.toEncoded();

//                    qDebug() << "fromHashMyEventsRules script is found " << QUrl::toPercentEncoding(line.mid(indxFrom + 1));

//                }
//                continue;

//            }


//            //this is the future for messages
//            if(path2script == "telegram"){ //message mode name
//                oneLineSett = getExecuteTelegramSett(line);
//                if(oneLineSett.command > 0)
//                    commands2execute.append(oneLineSett);

//            }

            continue;

        }


        if(oneLineSett.line.contains("{") && oneLineSett.line.contains("}") && oneLineSett.line.contains(":")){
            const QJsonDocument jdoc = QJsonDocument::fromJson(oneLineSett.line.toUtf8());

            if(!jdoc.isNull()){
                if(jdoc.isObject()){
                    const QJsonObject json = jdoc.object();
                    if(!json.isEmpty()){
                        oneLineSett.isJson = true;
                        commands2execute.append(oneLineSett);
                    }
                    continue;

                }
                //                    if(jdoc.isArray()){     do I really need it?
                //                        const QJsonArray array = jdoc.array();


                //                    }
            }
        }
        commands2execute.append(oneLineSett);
    }
    return commands2execute;
}

//----------------------------------------------------------------------------------------

QHash<QString, QString> DataHolderSharedObjectProcessor::hdataFromVarHash(const QVariantHash &hash)
{
    QHash<QString, QString> out;
    const auto lk = hash.keys();
    for(int i = 0, imax = lk.size(); i < imax; i++){
        out.insert(lk.at(i), hash.value(lk.at(i)).toString());
    }
    return out;
}

//----------------------------------------------------------------------------------------

QHash<QString, QString> DataHolderSharedObjectProcessor::hdataFromOneRecord(const QString &devID, const DHMsecRecord &oneRecord)
{
    QHash<QString, QString> out = hdataFromVarHash(oneRecord.hash);


    out.insert("NI", devID);
    out.insert("SN", oneRecord.additionalID);
    return out;

    //    struct DHMsecRecord
    //    {
    //        qint64 msec;
    //        QString additionalID;

    //        QVariantHash hash;
    //        QString srcname;
    //        bool wasRestored;

    //        DHMsecRecord() : msec(0), wasRestored(false) {}

    //        DHMsecRecord(const qint64 &msec, const QString &additionalID, const QVariantHash &hash, const QString &srcname, const bool &wasRestored)
    //            : msec(msec), additionalID(additionalID), hash(hash), srcname(srcname), wasRestored(wasRestored) {}
    //    };
}

//----------------------------------------------------------------------------------------

QHash<QString, QString> DataHolderSharedObjectProcessor::hdataFromOnePayload(const QString &who, const QString &evntType, const QVariantHash &payload)
{
    QHash<QString, QString> out = hdataFromVarHash(payload);
    out.insert("who", who);
    out.insert("evntType", evntType);
    return out;
}

//----------------------------------------------------------------------------------------

QString DataHolderSharedObjectProcessor::getHRulesCounterKey(const MyRuleSettings &ruleSett)
{
    return QString("%1\n\n\n%2").arg(ruleSett.ruleName).arg(ruleSett.ruleLine);

}

//----------------------------------------------------------------------------------------

QString DataHolderSharedObjectProcessor::getHRuleNameFromTheKey(const QString &key)
{
    return key.split("\n\n\n").first();
}

//----------------------------------------------------------------------------------------


//QJsonObject DataHolderSharedObjectProcessor::getTelegramJsonSett(const QString &line)
//{
//    //obsolete
//    QJsonObject json;
//    //telegram botToken chId message
//    const QStringList sett = line.split(" ");
//    if(sett.length() > 3){
//        //sett.at(0) - telegram
//        const QString botToken = sett.at(1);
//        const QString chId = sett.at(2);
//        const QString message = sett.mid(3).join(" ");

//        if(botToken.isEmpty() || chId.isEmpty()){
//            emit append2log(tr("Rule %1, bad settings, token=%2, ch=%3")
//                            .arg(sett.at(0))
//                            .arg(int(botToken.isEmpty()))
//                            .arg(int(chId.isEmpty()))
//                            );
//            return json;
//        }

//        json.insert("__path", "/opt/matilda/script/telegram");
//        json.insert("__botToken", botToken);
//        json.insert("__chId", chId);
//        json.insert("__message", message );

//    }

//    return json;
//}

//----------------------------------------------------------------------------------------


//MyExecuteLine DataHolderSharedObjectProcessor::getExecuteTelegramSett(const QString &line)
//{
//    MyExecuteLine oneLineSett;
//    const auto json = getTelegramJsonSett(line);

//    if(!json.isEmpty()){

//        oneLineSett.line = QJsonDocument(json).toJson(QJsonDocument::Compact);

//        oneLineSett.isJson = true;
//        oneLineSett.command = 0xFFFF;//send message

//    }
//    return oneLineSett;
//}

//----------------------------------------------------------------------------------------


void DataHolderSharedObjectProcessor::createLinesIterator()
{
    iterator = new MyLinesInterpretator(verboseMode);

    //from iterator

    connect(iterator, &MyLinesInterpretator::gimmeThisDevIDData             , this, &DataHolderSharedObjectProcessor::gimmeThisDevIDData            );
    connect(iterator, &MyLinesInterpretator::gimmeThisAdditionalDevIDData   , this, &DataHolderSharedObjectProcessor::gimmeThisAdditionalDevIDData  );


    //to iterator
    connect(this, &DataHolderSharedObjectProcessor::setThisDevIDData            , iterator, &MyLinesInterpretator::setThisDevIDData             );
    connect(this, &DataHolderSharedObjectProcessor::setThisAdditionalDevIDData  , iterator, &MyLinesInterpretator::setThisAdditionalDevIDData   );


}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::setEventManagerRules(QVariantHash hashRules, QVariantHash hashProfiles)
{

    MyEventsRules lastRules = fromHashMyEventsRules(hashRules, hashProfiles);

    lastPollRules.clear();
    lastSystemRules.clear();

    if(lastRules.contains(0)){
        lastSystemRules.insert(0, lastRules.take(0));

        //start system checker
    }
    lastPollRules = lastRules;



    availableMethods = MyMathHelper::gimmeMethods();


}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::checkThisDeviceNoData(const quint16 &pollCode, const QString &devID, const DHMsecRecord &oneRecord)
{
    //when there is no data for long time, max 7 days

    //it checks only devices after answer,
    if(lastPollRules.isEmpty())
        return; //no rules , so take a rest

    if(!lastPollRules.contains(pollCode))
        return;//ignore this poll code
    auto hdata = hdataFromOneRecord(devID, oneRecord);

    smartEvntProcessor(devID, "", pollCode, lastPollRules.value(pollCode), hdata);

}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::checkThisDevice(const quint16 &pollCode, const QString &devID, const DHMsecRecord &oneRecord)
{
    //it checks only devices after answer,
    if(lastPollRules.isEmpty())
        return; //no rules , so take a rest

    if(!lastPollRules.contains(pollCode))
        return;//ignore this poll code

//    const auto listOneCode = lastPollRules.value(pollCode);

    //    QStringList removeBrokenRules;

    auto hdata = hdataFromOneRecord(devID, oneRecord);

//    smartEvntProcessor(who, evntType, pollCode, lastPollRules.value(pollCode), hdata);
    smartEvntProcessor(devID, "", pollCode, lastPollRules.value(pollCode), hdata);



}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::onThisCommandFailed(QString ruleNameId, QString counterId)
{
    //    const QString ruleNameLineKey = getHRulesCounterKey(oneRule);

    if(ruleNameId.isEmpty()){

        return;//test conditions have all necessary ids
    }

    auto ruleCounterHash = hRulesCounter.value(ruleNameId) ;
    //    const QString ruleCounterKey = QString("%1\n%2\n%3").arg(int(pollCode)).arg(devID).arg(oneRecord.additionalID);


    auto ruleCounter = ruleCounterHash.value(counterId, 0);


    if(verboseMode)
        qDebug() << "DataHolderSharedObjectProcessor::onThisCommandFailed " << ruleNameId << counterId << ruleCounter;



    if(ruleCounter > 0){
        ruleCounter--;
        ruleCounterHash.insert(counterId, ruleCounter);
        hRulesCounter.insert(ruleNameId, ruleCounterHash);
    }


}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::testThisRule(QString ruleName, QVariantHash oneRuleH)
{
    //execute the rule command

    const auto commands2execute = fromStringList(oneRuleH.value("c").toStringList(), lastMapProfiles);

    if(commands2execute.isEmpty()){
        emit append2log(tr("Nothing to execute"));
        return;
    }
    const auto variables = oneRuleH.value("v").toHash();

    const QString devID = variables.value("NI").toString(); //who or IN
    const QString additionalID = variables.value("SN").toString(); //evntType or SN
    const quint16 pollCode = variables.value("code", 0).toUInt(); //pollCode or 0 - system event


    MyRuleSettings oneRule;
    oneRule.ruleName = ruleName;
    oneRule.ruleLine = "test";

    //        void addThisDHEvent(QString ruleName, int cntr, QString ruleLine, QString devId, QString additioanlDevId);



    emit addThisDHEvent(oneRule.ruleName, 0, pollCode, "test", devID, additionalID);



    const QString ruleNameLineKey = getHRulesCounterKey(oneRule);
    const QString ruleCounterKey = QString("%1\n%2\n%3").arg("0").arg(devID).arg(additionalID);

    const int cntr = executeLines(commands2execute, devID, ruleNameLineKey, ruleCounterKey, QHash<QString, QString>());

    if(verboseMode)
        qDebug() << "DataHolderSharedObjectProcessor::testThisRule send str " << cntr << oneRule.ruleName << oneRule.ruleLine;

}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::resetThisRules(QStringList ruleNames)
{
    if(ruleNames.isEmpty() || hRulesCounter.isEmpty())
        return;


    for(int i = 0, imax = ruleNames.size(); i < imax; i++){
        const auto ruleName = ruleNames.at(i);

        if(ruleName.isEmpty()){
            hRulesCounter.clear();
            return;
        }

        const auto lk = hRulesCounter.keys();
        for(int i = 0, imax = lk.size(); i < imax; i++){
            if(getHRuleNameFromTheKey(lk.at(i)) == ruleName){
                hRulesCounter.remove(lk.at(i));
            }
        }
    }

}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::smartSystemEvent(QString who, QString evntType, QVariantHash payload)
{
    if(lastSystemRules.isEmpty())
        return;

//    const auto listOneCode = lastSystemRules.value(0);//keep this compatibility

    auto hdata = hdataFromOnePayload(who, evntType, payload);
    smartEvntProcessor(who, evntType, 0, lastSystemRules.value(0), hdata);


}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::sendTestMessage(QString profName, QVariantHash oneProf)
{
    QVariantHash h;
    h.insert(profName, oneProf);

    const auto mapProfile = fromSendMessageProfileMap(h);


    if(mapProfile.isEmpty()){
        emit append2log(tr("Nothing to test, profile"));
        if(verboseMode)
            qDebug() << "sendTestMessage " << profName << oneProf;
        return;
    }
    QStringList l;
    l.append(QString("sendMessage %1 Profile: %1, Test message.").arg(profName));
    const auto commands2execute = fromStringList(l, mapProfile);

    if(mapProfile.isEmpty()){
        emit append2log(tr("Nothing to test, commands"));
        if(verboseMode)
            qDebug() << "sendTestMessage " << profName << oneProf << l;
        return;
    }

    QVariantMap map = QJsonDocument::fromJson(commands2execute.constFirst().line.toUtf8()).object().toVariantMap();


    map.insert("__ruleNameId", "test");
    map.insert("__counterId", "test");

    //final
     emit sendAMessageDevMap(map);

}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::smartEvntProcessor(const QString &devIdWho, const QString &additionalIdEvntType, const quint16 &pollCode, const MyRuleSettingsList &listOneCode, QHash<QString, QString> &hdata)
{

    /*
     * variables
     * when systemEvent, pollCode = 0
     * who
     *  - matilda-bbb
     *  - zbyrator-bbb
     *  - peredavator-bbb
     *  - firefly-bbb
     *  - matilda-uart
     *  - clock: dtEvnt
     *
     *  evntType
     *      - logIn : msec, usr, ip, lvl
     *      - logOut: msec, usr, ip, lvl
     *      - authFail: msec, usr, ip
     *      - appStart: msec
     *      - dtEvnt: msec
     *      - gsmMoney - msec, USSD answer when money was checked
     *      - embInfo - msec, ch: main or others,
     *
     *  when pollCode != 0
     *  exchagne event
     *  devIdWho - NI
     *  additionalIdEvntType - empty
     *
     */


    for(int i = 0, imax = listOneCode.size(); i < imax; i++){
        const auto oneRule = listOneCode.at(i);
        const QString ruleNameLineKey = getHRulesCounterKey(oneRule);

        auto ruleCounterHash = hRulesCounter.value(ruleNameLineKey) ;
        const QString ruleCounterKey = QString("%1\n%2\n%3").arg(int(pollCode)).arg(devIdWho).arg(additionalIdEvntType);


        auto ruleCounter = ruleCounterHash.value(ruleCounterKey, 0);

        hdata.insert("counter", QString::number(ruleCounter));

        QStringList errorList;
        const QString out = iterator->gimmeTheFunctionResult(oneRule.ruleLine, availableMethods, hdata, errorList);

        if(out.isEmpty()){
            if(verboseMode)
                qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice " << out << errorList << oneRule.ruleLine;
            //            removeBrokenRules.append(oneRule.ruleLine);

            emit append2log(tr("Bad rule %1, %2, %3").arg(oneRule.ruleName).arg(oneRule.ruleLine).arg(errorList.join("\n").left(800)));
            continue;
        }



        //ok if everything is fine

        if(out.toInt() == 0){
            if(verboseMode)
                qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice ruleCounter reset" << ruleCounter << oneRule.limitExecutions << oneRule.ruleLine << oneRule.ruleName << ruleCounterKey;

            if(ruleCounterHash.value(ruleCounterKey, 0) > 0){
                emit append2log(tr("rule reset counter %1, %2").arg(oneRule.ruleName).arg(ruleCounter));

                ruleCounterHash.insert(ruleCounterKey, 0);
                hRulesCounter.insert(ruleNameLineKey, ruleCounterHash);//reset the counter
            }

            continue; //

        }

        ruleCounter++;

        if(oneRule.limitExecutions > 0 && oneRule.limitExecutions < ruleCounter){
            continue;//stop execution of the rule
        }

        //        void addThisDHEvent(QString ruleName, int cntr, QString ruleLine, QString devId, QString additioanlDevId);
        emit addThisDHEvent(oneRule.ruleName, ruleCounter, pollCode, oneRule.ruleLine, devIdWho, additionalIdEvntType);


        ruleCounterHash.insert(ruleCounterKey, ruleCounter);

        hRulesCounter.insert(ruleNameLineKey, ruleCounterHash);

        emit append2log(tr("rule accepted counter %1, %2").arg(oneRule.ruleName).arg(ruleCounter));


        if(verboseMode)
            qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice ruleCounter" << ruleCounter << oneRule.limitExecutions << oneRule.ruleLine << oneRule.ruleName << ruleCounterKey;


        const int cntr = executeLines(oneRule.commands2execute, devIdWho, ruleNameLineKey, ruleCounterKey, hdata);


        if(verboseMode)
            qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice send str " << cntr << oneRule.ruleName << oneRule.ruleLine;
    }

}

//----------------------------------------------------------------------------------------

int DataHolderSharedObjectProcessor::executeLines(const QList<MyExecuteLine> &commands2execute, const QString &devID, const QString &ruleNameLineKey, const QString &ruleCounterKey, const QHash<QString, QString> &hdata)
{
    int cntr = 0;
    for(int j = 0, jmax = commands2execute.size(); j < jmax; j++){
        auto oneLineSett = commands2execute.at(j);

        if(oneLineSett.line.contains("$NI"))
            oneLineSett.line = oneLineSett.line.replace("$NI", devID);

        if(oneLineSett.isJson){
            QVariantMap map = QJsonDocument::fromJson(oneLineSett.line.toUtf8()).object().toVariantMap();
            if(map.isEmpty()){
                if(verboseMode)
                    qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice " << oneLineSett.isJson << oneLineSett.line;
                continue;
            }
            //find in __message keys for data
            if(map.value("__message").toString().contains("$")){
                const auto lk = hdata.keys();
                QString message = map.value("__message").toString();
                for(int ii = 0, iimax = lk.size(); ii < iimax; ii++){
                    if(message.contains(lk.at(ii))){
                        message.replace( QString("$%1").arg(lk.at(ii)), hdata.value(lk.at(ii)));
                        if(!message.contains("$"))
                            break;
                    }
                }
                map.insert("__message", message);
            }
//            fromStringList()
//            json.insert("__message", line.mid(line.indexOf(ll.at(1)) + ll.at(1).length() + 1) );


            if(verboseMode)
                qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice send map " << oneLineSett.command << map;

            map.insert("__ruleNameId", ruleNameLineKey);
            map.insert("__counterId", ruleCounterKey);
            //        emit onThisCommandFailed(mapArgs.value("__ruleNameId").toString(), mapArgs.value("__counterId").toString());

            if(oneLineSett.command == 0xFFFF){
                //currently only telegram is supported,
                //check __path

                emit sendAMessageDevMap(map);
            }else
                emit sendCommand2pollDevMap(oneLineSett.command, map);

            continue;
        }

        if(verboseMode)
            qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice send str " << oneLineSett.command << oneLineSett.line ;

        emit sendCommand2pollDevStr(oneLineSett.command, oneLineSett.line);
    }
    return cntr;
}

//----------------------------------------------------------------------------------------
