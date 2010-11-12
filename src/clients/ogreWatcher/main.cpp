#include <QApplication>
#include <QMainWindow>
#include "qtBuildFiles/ui/ui_ogreWatcher.h"
#include "qtBuildFiles/ui/ui_QMessageStreamPlaybackWidget.h"

int main(int argc, char **argv) 
{
    QApplication app(argc, argv);
    QMainWindow window;
    Ui_OgreWatcherMainWindow *ui = new Ui_OgreWatcherMainWindow;
    ui->setupUi(&window);

    Ui_messageStreamPlaybackForm *pbForm = new Ui_messageStreamPlaybackForm;
    pbForm->setupUi(ui->messageStreamPlaybackWidget); 

    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    QObject::connect(ui->quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
    QObject::connect(ui->action_Quit, SIGNAL(activated()), &app, SLOT(quit()));

    window.show();
    return app.exec();
}
