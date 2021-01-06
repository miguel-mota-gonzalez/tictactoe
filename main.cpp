
#include "window.h"

#include <QApplication>
#include <QDebug>

/*
 * This program was written for QT company
 *
 * Miguel Mota
 * Jan 5 2021
*/

///
/// \brief Main application class
///
/// Miguel Mota
/// Jan 5 2021
///
class ticTacToeApp: public QApplication {
public:
   ticTacToeApp(int argc, char *argv[]) : QApplication(argc, argv){
   }

    ~ticTacToeApp() {
       qDebug() << "App finished!";
    }
};

///
/// \brief main program function
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param argc parameter counter
/// \param argv parameters
/// \return Error Code
///
int main(int argc, char *argv[])
{
    ticTacToeApp app(argc, argv);

    qDebug() << "App started!";

    window::singleInstance()->show();

    return app.exec();
}
