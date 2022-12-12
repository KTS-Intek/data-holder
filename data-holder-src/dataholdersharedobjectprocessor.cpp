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

MyEventsRules DataHolderSharedObjectProcessor::fromHashMyEventsRules(const QVariantHash &h)
{
    //    ui->plainTextEdit_2->appendPlainText(tr("Out is - '%1'").arg(out));

    //    QVariantHash oneh;
    //    oneh.insert("r", oneRule.ruleLine);
    //    oneh.insert("c", oneRule.commands2execute);
    //    oneh.insert("pc", oneRule.pollCode);
    //    h.insert(ruleName, oneh);

    MyEventsRules out;
    const auto lk = h.keys();


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

        for(int j = 0, jmax = commands2execute.size(); j < jmax; j++){

            const QString line = commands2execute.at(j); //<poll code><space><arguments> or path to script, so check both

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
                const QString path2script = line.left(indxFrom);

                const QFileInfo fi(path2script);
                if(fi.exists() && fi.isExecutable()){
                    QJsonObject json;
                    json.insert("__path", path2script);
                    json.insert("__message",oneLineSett.line );


                    oneLineSett.line = QJsonDocument(json).toJson(QJsonDocument::Compact);

                    oneLineSett.isJson = true;
                    oneLineSett.command = 0xFFFF;//send message

                    oneRule.commands2execute.append(oneLineSett);




                    if(verboseMode){
                        qDebug() << "fromHashMyEventsRules script is found " << path2script <<  line.mid(indxFrom + 1);

//                        const QUrl url(line.mid(indxFrom + 1));
//                        const QString strArgs = url.toEncoded();

                        qDebug() << "fromHashMyEventsRules script is found " << QUrl::toPercentEncoding(line.mid(indxFrom + 1));

                    }

                }


                continue;

            }


            if(oneLineSett.line.contains("{") && oneLineSett.line.contains("}") && oneLineSett.line.contains(":")){
                const QJsonDocument jdoc = QJsonDocument::fromJson(oneLineSett.line.toUtf8());

                if(!jdoc.isNull()){
                    if(jdoc.isObject()){
                        const QJsonObject json = jdoc.object();
                        if(!json.isEmpty()){
                            oneLineSett.isJson = true;
                            oneRule.commands2execute.append(oneLineSett);
                        }
                        continue;

                    }
//                    if(jdoc.isArray()){     do I really need it?
//                        const QJsonArray array = jdoc.array();


//                    }
                }
            }
            oneRule.commands2execute.append(oneLineSett);
        }

        if(oneRule.commands2execute.isEmpty())
            continue;//bad settings, ignore them

        const quint16  pollCode = oneh.value("pc").toUInt();
        MyRuleSettingsList l = out.value(pollCode);
        l.append(oneRule);

        out.insert(pollCode, l);
    }

    return out;
}

//----------------------------------------------------------------------------------------

QHash<QString, QString> DataHolderSharedObjectProcessor::hdataFromOneRecord(const QString &devID, const DHMsecRecord &oneRecord)
{
    QHash<QString, QString> out;

    const auto lk = oneRecord.hash.keys();

    for(int i = 0, imax = lk.size(); i < imax; i++){
        out.insert(lk.at(i), oneRecord.hash.value(lk.at(i)).toString());
    }

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

QString DataHolderSharedObjectProcessor::getHRulesCounterKey(const MyRuleSettings &ruleSett)
{
    return QString("%1\n\n\n%2").arg(ruleSett.ruleName).arg(ruleSett.ruleLine);

}

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

void DataHolderSharedObjectProcessor::setEventManagerRules(QVariantHash hashRules)
{

    lastRules = fromHashMyEventsRules(hashRules);

    availableMethods = MyMathHelper::gimmeMethods();


}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::checkThisDevice(const quint16 &pollCode, const QString &devID, const DHMsecRecord &oneRecord)
{
    if(lastRules.isEmpty())
        return; //no rules , so take a rest

    if(!lastRules.contains(pollCode))
        return;//ignore this poll code

    const auto listOneCode = lastRules.value(pollCode);

//    QStringList removeBrokenRules;

    auto hdata = hdataFromOneRecord(devID, oneRecord);

    for(int i = 0, imax = listOneCode.size(); i < imax; i++){
        const auto oneRule = listOneCode.at(i);
        const QString ruleNameLineKey = getHRulesCounterKey(oneRule);

        auto ruleCounterHash = hRulesCounter.value(ruleNameLineKey) ;
        const QString ruleCounterKey = QString("%1\n%2\n%3").arg(int(pollCode)).arg(devID).arg(oneRecord.additionalID);


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
        ruleCounterHash.insert(ruleCounterKey, ruleCounter);

        hRulesCounter.insert(ruleNameLineKey, ruleCounterHash);

        emit append2log(tr("rule accepted counter %1, %2").arg(oneRule.ruleName).arg(ruleCounter));


        if(verboseMode)
            qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice ruleCounter" << ruleCounter << oneRule.limitExecutions << oneRule.ruleLine << oneRule.ruleName << ruleCounterKey;

        for(int j = 0, jmax = oneRule.commands2execute.size(); j < jmax; j++){
            auto oneLineSett = oneRule.commands2execute.at(j);

            if(oneLineSett.line.contains("$NI"))
                oneLineSett.line = oneLineSett.line.replace("$NI", devID);

            if(oneLineSett.isJson){
                QVariantMap map = QJsonDocument::fromJson(oneLineSett.line.toUtf8()).object().toVariantMap();
                if(map.isEmpty()){
                    if(verboseMode)
                        qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice " << oneLineSett.isJson << oneLineSett.line;
                    continue;
                }

                if(verboseMode)
                    qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice send map " << oneLineSett.command << map;

                map.insert("__ruleNameId", ruleNameLineKey);
                map.insert("__counterId", ruleCounterKey);
                //        emit onThisCommandFailed(mapArgs.value("__ruleNameId").toString(), mapArgs.value("__counterId").toString());

                emit sendCommand2pollDevMap(oneLineSett.command, map);
                continue;
            }

            if(verboseMode)
                qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice send str " << oneLineSett.command << oneLineSett.line << oneRule.ruleName << oneRule.ruleLine;

            emit sendCommand2pollDevStr(oneLineSett.command, oneLineSett.line);
        }
    }


}

//----------------------------------------------------------------------------------------

void DataHolderSharedObjectProcessor::onThisCommandFailed(QString ruleNameId, QString counterId)
{
//    const QString ruleNameLineKey = getHRulesCounterKey(oneRule);

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
