
#include "window.h"
#include "gameboard.h"

#include <QGridLayout>
#include <QSharedPointer>
#include <QDebug>

QSharedPointer<window> window::mInstance;

///
/// \brief window::singleInstance Returns a pointer to this class singleton instance
///
/// Miguel Mota
/// Jan 5 2021
///
/// \return
///
QSharedPointer<window> window::singleInstance()
{
    if(mInstance.isNull()){
        mInstance = QSharedPointer<window>(new window());
        qDebug() << "window singleton instance created...";
    }

    return mInstance;
}

///
/// \brief window::window Default constructor
///
/// Miguel Mota
/// Jan 5 2021
///
///
window::window()
{
    qDebug() << "window constructor!";

    setWindowTitle(tr("My TicTacToe in QT!"));

    // Create the gameboard...
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(gameBoard::singleInstance(this), 0, 0);
    setLayout(layout);
}

///
/// \brief window::closeEvent Window closed - Event handler
///
/// Miguel Mota
/// Jan 5 2021
///
///
void window::closeEvent(QCloseEvent *)
{
    qDebug("The close button was clicked");
    gameBoard::destroySingleton();
}

