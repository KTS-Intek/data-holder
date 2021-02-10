#include <QCoreApplication>
#include "data-holder-src/dataholdermanager.h"

/*
it
receives data from poll applications (zbyrator-bbb, firefly-bbb or any other
stores that data in RAM




*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DataHolderManager manager;
    manager.createObjectsLater();

    const int r = a.exec();

    manager.saveAllYourData();
    QThread::sleep(3);//save shared memory

    return r;
}
