#ifndef DATAHOLDERLOCALSERVER_H
#define DATAHOLDERLOCALSERVER_H

#include <QObject>

class DataHolderLocalServer : public QObject
{
    Q_OBJECT
public:
    explicit DataHolderLocalServer(QObject *parent = nullptr);

signals:

};

#endif // DATAHOLDERLOCALSERVER_H
