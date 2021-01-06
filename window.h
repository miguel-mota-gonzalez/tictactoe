#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QCloseEvent>
#include <QDebug>

///
/// \brief The main window class
///
/// Miguel Mota
/// Jan 5 2021
///
class window : public QWidget
{
    Q_OBJECT

public:
    static QSharedPointer<window> singleInstance();
    ~window() { qDebug() << "window destroyed!"; };

protected:
    static QSharedPointer<window> mInstance;

    window();
    void closeEvent(QCloseEvent *event) override;
};

#endif
