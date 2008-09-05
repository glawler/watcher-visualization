#include "ui_watcher.h"
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow *window = new QMainWindow;
    Ui::MainWindow ui;
    ui.setupUi(window);

    ui.menuLayers->setTearOffEnabled(true);
    ui.menuView->setTearOffEnabled(true);
    ui.menuGoodwin->setTearOffEnabled(true);

    QObject::connect(ui.quitButton, SIGNAL(clicked()), &app, SLOT(quit()));

    ui.manetGLViewWindow->runLegacyWatcherMain(argc, argv);

    window->show();
    return app.exec();
}
