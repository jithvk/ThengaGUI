#include "mainwindow.h"
#include <QApplication>
#include "QSplashScreen"

extern "C" {
#include "sysdeps.h"
#include "adb.h"
#include "adb_auth.h"
}

int gui_main(int argc, char *argv[])
{


    QApplication a(argc, argv);
    QSplashScreen splash(QPixmap(":/icons/res/debug_big.png"));
    splash.show();
    a.processEvents();
    MainWindow w;
    w.show();
    splash.finish(&w);
    a.setApplicationVersion(APP_VERSION);
    return a.exec();
}


int main(int argc, char *argv[])
{


    adb_sysdeps_init();
    adb_trace_init();

    if(argc > 1)
    {
        return adb_commandline(argc - 1, argv + 1);
    }
    else
        gui_main(argc,argv);
}
