#include "dataholdersharedobjectprocessor.h"

#include <QJsonDocument>
#include <QJsonObject>


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

    for(int i = 0, imax = lk.size(); i < imax; i++){
        const QString ruleName = lk.at(i);
        const auto oneh = h.value(ruleName).toHash();

        MyRuleSettings oneRule;

        oneRule.ruleName = ruleName;
        oneRule.ruleLine = oneh.value("r").toString();

        const auto commands2execute = oneh.value("c").toStringList();


        for(int j = 0, jmax = commands2execute.size(); j < jmax; j++){

            const QString line = commands2execute.at(j); //<poll code><space><arguments>
            const int indxFrom = line.indexOf(" ");

            if(indxFrom < 0)
                continue;

            MyExecuteLine oneLineSett;
            oneLineSett.line = line.mid(indxFrom + 1);
            oneLineSett.command = line.left(indxFrom).toUInt();

            if(oneLineSett.command < POLL_CODE_FF_READ_LAMP || oneLineSett.line.isEmpty())
                continue;

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


void DataHolderSharedObjectProcessor::createLinesIterator()
{
    iterator = new MyLinesInterpretator(verboseMode);
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

    const auto hdata = hdataFromOneRecord(devID, oneRecord);

    for(int i = 0, imax = listOneCode.size(); i < imax; i++){
        const auto oneRule = listOneCode.at(i);


        QStringList errorList;
        const QString out = iterator->gimmeTheFunctionResult(oneRule.ruleLine, availableMethods, hdata, errorList);

        if(out.isEmpty()){
            if(verboseMode)
                qDebug() << "DataHolderSharedObjectProcessor::checkThisDevice " << out << errorList << oneRule.ruleLine;
//            removeBrokenRules.append(oneRule.ruleLine);
            continue;
        }



        //ok if everything is fine

        if(out.toInt() == 0)
            continue; //

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
                emit sendCommand2pollDevMap(oneLineSett.command, map);
                continue;
            }
            emit sendCommand2pollDevStr(oneLineSett.command, oneLineSett.line);
        }
    }


}

//----------------------------------------------------------------------------------------
