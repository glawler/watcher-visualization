#include <QApplication>
#include <QMainWindow>
#include "qtBuildFiles/ui/ui_ogreWatcher.h"

int main(int argc, char **argv) 
{
    QApplication app(argc, argv);
    QMainWindow window;
    Ui_MainWindow *ui = new Ui_MainWindow;
    ui->setupUi(&window);

    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    QObject::connect(ui->quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
    QObject::connect(ui->action_Quit, SIGNAL(activated()), &app, SLOT(quit()));

    window.show();
    return app.exec();
}
