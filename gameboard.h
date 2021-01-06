#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QWidget>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

///
/// \brief The gameBoard class
///
/// Miguel Mota
///
/// Jan 5 2021
///
///
class gameBoard : public QWidget
{
    Q_OBJECT

public:
    static gameBoard *singleInstance(QWidget *parent);

    static void destroySingleton(){
                                    QMutexLocker ml(&mMutex);
                                    if(mInstance!=nullptr){
                                        delete mInstance;
                                        qDebug() << "game board singleton destroyed...";
                                    }
                                    mInstance=nullptr;
                                  };

protected:

    // Singleton instance, No smart pointer here
    // 'cause the window widget will destroy it
    // when closed...
    static gameBoard *mInstance;
    // Mutex used for protecting mInstance
    // object, used in timer...
    static QMutex mMutex;
    //Flag used to avoid
    // blinking timer to be duplicated
    // and get crazy
    bool mTimeToBlink;

    // Enums
    enum winnerOption {ROW, COL, DIAG_1, DIAG_2, BOARD_FULL, GAME_IN_PROGRESS};

    // Member variables
    int mRectWidth;
    int mRectHeight;
    winnerOption mWinningStatus;
    int mWinningIndex;
    bool mUserWon;
    bool mBlink;
    bool mThinking;
    QVector<QVector<int>> mBoardValues = { {-1 , -1, -1},
                                           {-1 , -1, -1},
                                           {-1 , -1, -1} };

    // Events
    void mousePressEvent (QMouseEvent *event) override;    

    // Constructors - Destructors
    gameBoard(QWidget *parent);
    ~gameBoard() { qDebug() << "game board destroyed!"; };

    // Drawing methods
    void paintEvent(QPaintEvent *event) override;
    void drawLines(QPainter &painter, const QRect &rect) const;
    void setDrawColors(QPainter &painter, const QRect &rect) const;
    void drawX(QPainter &painter, const int x, const int y) const;
    void drawO(QPainter &painter, const int x, const int y) const;
    void drawGame(QPainter &painter, const QRect &rect) const;    
    void markWiningMove(QPainter &painter) const;
    void showMessages(QPainter &painter);

    // logic methods
    void setUserMove(const int x, const int y);
    bool getGameStatus();

};

#endif // GAMEBOARD_H
