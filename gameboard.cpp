#include "gameboard.h"
#include "virtualplayer.h"

#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QDebug>

const int MAGIC_BOARD_SIZE_X = 333;
const int MAGIC_BOARD_SIZE_Y = 333;
const int TOTAL_LINES        = 3;
const int ITEM_SIZE          = 50;
const int X_LINE_WIDTH       = 10;
const int BOARD_LINE_WIDTH   = 12;
const int HALF_SQ_ANGLE      = 45;
const int TOTAL_ROWS         = 3;
const int TOTAL_COLS         = 3;
const int BLINK_TIME         = 400;
const int O_VALUE            = 0;
const int X_VALUE            = 1;
const int USER_MOVE          = X_VALUE;
const int THINKING_TIME      = 1000;
const QString FONT_TYPE      = QStringLiteral("Times");
const QString YOU_WIN        = QStringLiteral("YOU WIN!!!");
const QString YOU_LOSE       = QStringLiteral("YOU LOSE!!!");
const QString NO_WINNER      = QStringLiteral("NO WINNER!!!");
const QString CLICK_TO_PLAY  = QStringLiteral("Click the board to play again...");

gameBoard *gameBoard::mInstance = nullptr;
QMutex gameBoard::mMutex;

///
/// \brief gameBoard::singleInstance Returns the instance of the gameboard singleton
///
/// Miguel Mota Jan 5 2021
///
/// \param parent parent window
/// \return pointer to the singleton instance
///
gameBoard *gameBoard::singleInstance(QWidget *parent)
{
    if(mInstance == nullptr){
        mInstance = new gameBoard(parent);
        qDebug() << "game board singleton instance created...";
    }
    return mInstance;
}

///
/// \brief gameBoard::gameBoard Main constructor
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param parent Parent window
///
gameBoard::gameBoard(QWidget *parent) : QWidget(parent),
    mTimeToBlink(false), mRectWidth(0), mRectHeight(0),
    mWinningStatus(gameBoard::winnerOption::GAME_IN_PROGRESS),
    mWinningIndex(-1), mUserWon(false), mBlink(true),mThinking(false)
{
    qDebug() << "game board constructor!";
    setFixedSize(MAGIC_BOARD_SIZE_X , MAGIC_BOARD_SIZE_Y);
}

///
/// \brief gameBoard::setDrawColors Sets the colors for board items
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param painter current painter object
/// \param rect widget area
///
void gameBoard::setDrawColors(QPainter &painter, const QRect &rect) const
{
    QLinearGradient gradient(QPointF(1, 1), QPointF(rect.width(), rect.height()));
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(1.0, Qt::yellow);

    painter.setBrush(QBrush(gradient));
    painter.setPen(QPen(Qt::black));

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect,  QBrush(Qt::darkBlue));
}

///
/// \brief gameBoard::drawLines Draws the board lines
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param painter Current painter object
/// \param rect widget area
///
void gameBoard::drawLines(QPainter &painter, const QRect &rect) const
{
    for (int i = 1; i<TOTAL_LINES; i++){
        painter.drawRect((rect.width()/TOTAL_LINES)*i, 1, BOARD_LINE_WIDTH, rect.height());
        painter.drawRect(1,(rect.height()/TOTAL_LINES)*i, rect.width(), BOARD_LINE_WIDTH);
    }
}

///
/// \brief gameBoard::drawX Draws a big X in the specified position
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param painter Current painter
/// \param x Desired X position
/// \param y Desired Y position
///
void gameBoard::drawX(QPainter &painter, const int x, const int y) const
{
    painter.translate(x+8, y-5);
    painter.rotate(HALF_SQ_ANGLE);
    painter.drawRect(1,1,ITEM_SIZE, X_LINE_WIDTH);

    painter.rotate(-1*HALF_SQ_ANGLE);
    painter.translate(-1*(x+8), -1*(y-5));

    painter.translate(x, y+32);
    painter.rotate(-1*HALF_SQ_ANGLE);
    painter.drawRect(1,1,ITEM_SIZE, X_LINE_WIDTH);

    painter.rotate(HALF_SQ_ANGLE);
    painter.translate(-1*(x), -1*(y+32));
}

///
/// \brief gameBoard::drawO Draws a big O in the specified position
/// \param painter Current painter
/// \param x Desired X position
/// \param y Desired Y position
///
void gameBoard::drawO(QPainter &painter, const int x, const int y) const
{
    painter.drawEllipse(QRectF(x,y,ITEM_SIZE, ITEM_SIZE));
}

///
/// \brief gameBoard::drawGame Draws the current elements of the game in the board.
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param painter Current painter
/// \param rect widget area
///
void gameBoard::drawGame(QPainter &painter, const QRect &rect) const
{    
    int adjustX = ((rect.width()/TOTAL_LINES) - ITEM_SIZE)/2;
    int adjustY = ((rect.height()/TOTAL_LINES) - ITEM_SIZE)/2;

    // What shall we draw?
    std::function<void(const int, const int, const int)> f = [&](const int val, const int x, const int y) -> void{
        if(val==O_VALUE){
            drawO(painter, x, y);
        }
        else if (val==X_VALUE){
            drawX(painter, x, y);
        }
    };

    // For all the items in the board
    int count = 0;
    for(const QVector<int> &line : mBoardValues){
        // Draw each line item...
        f(line[0], adjustX, (rect.height()/TOTAL_LINES)*count + adjustY);
        f(line[1], ((rect.width()/TOTAL_LINES))+adjustX, (rect.height()/3)*count + adjustY);
        f(line[2], ((rect.width()/TOTAL_LINES)*2)+adjustX, (rect.height()/3)*count + adjustY);

        count++;
    }
}

///
/// \brief gameBoard::paintEvent Paint event handler
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param event
///
void gameBoard::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);

    setDrawColors(painter, event->rect());
    drawLines(painter, event->rect());
    drawGame(painter, event->rect());

    //Save last rect size
    mRectWidth = event->rect().width();
    mRectHeight = event->rect().height();

    markWiningMove(painter);

    // shall we draw the blinking message?
    if(mTimeToBlink){
        qDebug() << "Blink!";
        mTimeToBlink = false;
        showMessages(painter);
    }
    else if(mThinking){

        qDebug() << "Thinking...";
        QFont smallTextFont(FONT_TYPE, 10, QFont::Normal);
        QFont gameOverFont(FONT_TYPE, 20, QFont::Bold);

        // let the user know we are thinking...
        painter.setFont(gameOverFont);
        painter.setPen(QPen(Qt::cyan));
        painter.drawText(mRectWidth/2 - 40,  mRectHeight/2 - 25, "Thinking ...");
    }
    painter.end();
}

///
/// \brief gameBoard::setUserMove Sets in the board a movement made by the user.
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param x (3 x 3) X postion set by the user in the board
/// \param y (3 x 3) Y postion set by the user in the board
///
void gameBoard::setUserMove(const int x, const int y)
{
    if(mBoardValues[y][x]==-1){
        mBoardValues[y][x]=USER_MOVE;

        // virtual user move code...
        std::function<void()> makeUserMove = [&]{

            // not thinking anymore...
            mThinking = false;

            if(mInstance!=nullptr){
                QWidget* w = dynamic_cast<QWidget*>(mInstance);
                if(w!=nullptr){

                    virtualPlayer::singleInstance()->makeYourNextMove(mBoardValues);

                    // check status after virtual user moves
                    if(!getGameStatus()){
                        qDebug() << "Game Over!!! *";

                        // Dow we have a winner?
                        if(mWinningStatus == winnerOption::COL ||
                           mWinningStatus == winnerOption::ROW ||
                           mWinningStatus == winnerOption::DIAG_1 ||
                           mWinningStatus == winnerOption::DIAG_2){
                            // Virtual user won!
                            mUserWon=false;
                        }
                    }

                    w->update();
                }
            }
        };

        // Now we have to check the game status...
        if(getGameStatus()){

            // lets pretend we need time to think about our next move...
            mThinking = true;
            QTimer::singleShot(THINKING_TIME, makeUserMove);
        }
        else{
            // Game over!
            qDebug() << "Game Over!!!";

            // Dow we have a winner?
            if(mWinningStatus == winnerOption::COL ||
               mWinningStatus == winnerOption::ROW ||
               mWinningStatus == winnerOption::DIAG_1 ||
               mWinningStatus == winnerOption::DIAG_2){
                // User won!
                mUserWon=true;
            }
        }

        update();
    }
    else{
        qDebug() << "Already filled!";
    }
}

///
/// \brief gameBoard::markWiningMove Draws a red line marking the winning game
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param painter Current painter
///
void gameBoard::markWiningMove(QPainter &painter) const
{
    painter.setBrush(QBrush(Qt::red));
    painter.setPen(QPen(Qt::black));

    switch(mWinningStatus){
        case winnerOption::COL:
            painter.drawRect((mRectWidth/TOTAL_LINES)*mWinningIndex + (((mRectWidth/TOTAL_LINES)/2)), 1, 5, mRectHeight);
            break;
        case winnerOption::ROW:
            painter.drawRect(1, (mRectWidth/TOTAL_LINES)*mWinningIndex + (((mRectWidth/TOTAL_LINES)/2)), mRectWidth, 5);
            break;
        case winnerOption::DIAG_1:
            painter.rotate(HALF_SQ_ANGLE);
            painter.drawRect(1, 1, mRectWidth*2, 5);
            painter.rotate(-1*HALF_SQ_ANGLE);
            break;
        case winnerOption::DIAG_2:
            painter.translate(1, mRectHeight);
            painter.rotate(-1*HALF_SQ_ANGLE);
            painter.drawRect(1, 1, mRectWidth*2, 5);
            painter.rotate(HALF_SQ_ANGLE);
            painter.translate(-1, -1*mRectHeight);
            break;
        default:
            break;
    };
}

///
/// \brief gameBoard::showMessages Show a blinking message with the game result
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param painter Current painter
///
void gameBoard::showMessages(QPainter &painter)
{    
    // Blinking code...
    std::function<void()> blink = [&]{
        QMutexLocker ml(&mMutex);
        if(mInstance!=nullptr){
            QWidget* w = dynamic_cast<QWidget*>(mInstance);
            if(w!=nullptr){
                mTimeToBlink=true;
                w->update();
            }
        }
    };

    switch(mWinningStatus){
        case winnerOption::COL:
        case winnerOption::ROW:
        case winnerOption::DIAG_1:
        case winnerOption::DIAG_2:
            if(mBlink){
                QFont smallTextFont(FONT_TYPE, 10, QFont::Normal);
                QFont gameOverFont(FONT_TYPE, 20, QFont::Bold);

                // Display Wining status and game over!
                painter.setBrush(QBrush(Qt::black));
                painter.drawRect(1, mRectHeight/2 - 30, mRectWidth, 60);

                painter.setFont(gameOverFont);
                if(mUserWon){
                   painter.setPen(QPen(Qt::green));
                   painter.drawText(mRectWidth/2 - 70,  mRectHeight/2, YOU_WIN);
                }
                else{
                    painter.setPen(QPen(Qt::red));
                    painter.drawText(mRectWidth/2 - 70,  mRectHeight/2, YOU_LOSE);
                }

                painter.setPen(QPen(Qt::white));
                painter.setFont(smallTextFont);
                painter.drawText(mRectWidth/2 - 80,  mRectHeight/2 + 27, CLICK_TO_PLAY);
            }

            mBlink = !mBlink;
            QTimer::singleShot(BLINK_TIME, blink);
            break;
        case winnerOption::BOARD_FULL:
            if(mBlink){
                QFont smallTextFont(FONT_TYPE, 10, QFont::Normal);
                QFont gameOverFont(FONT_TYPE, 20, QFont::Bold);

                // Display Wining status and game over!
                painter.setBrush(QBrush(Qt::black));
                painter.drawRect(1, mRectHeight/2 - 30, mRectWidth, 60);

                painter.setFont(gameOverFont);
                painter.setPen(QPen(Qt::blue));
                painter.drawText(mRectWidth/2 - 90,  mRectHeight/2, NO_WINNER);

                painter.setPen(QPen(Qt::white));
                painter.setFont(smallTextFont);
                painter.drawText(mRectWidth/2 - 80,  mRectHeight/2 + 27, CLICK_TO_PLAY);
            }

            mBlink = !mBlink;
            QTimer::singleShot(BLINK_TIME, blink);
            break;
        case winnerOption::GAME_IN_PROGRESS:
            // Game in progress
            break;
    };
}

///
/// \brief gameBoard::getGameStatus Checks if some user has won, or the board is full, or if game shall continue...
///
/// Miguel Mota
/// Jan 5 2021
///
/// \return true : The game continues, false : the game has ended.
///
bool gameBoard::getGameStatus()
{
    bool boardFull = true;
    bool diag1Winner = true;
    bool diag2Winner = true;

    // Lambda for win verification...
    std::function<bool(const winnerOption, const bool, const int index)> checkIfWin = [&](const winnerOption wo,
                                                                          const bool winnerFlag,
                                                                          const int index) -> bool{
        if(winnerFlag){
            mWinningStatus = wo;
            mWinningIndex = index;
            return true;
        }
        return false;
    };

    for(int i=0; i<TOTAL_ROWS; i++){

        bool rowWinner = true;
        bool colWinner = true;

        for(int j=0; j<TOTAL_COLS; j++){

            // check if the board is full
            if(mBoardValues[i][j]==-1){
                boardFull=false;
            }

            // check all the col values for row i
            // check all the row values for col i
            if(j>0){
                if(mBoardValues[i][j]==-1 || mBoardValues[i][j]!=mBoardValues[i][j-1]){
                    rowWinner=false;
                }

                if(mBoardValues[j][i]==-1 || mBoardValues[j][i]!=mBoardValues[j-1][i]){
                    colWinner=false;
                }
            }

            // Check 2 diagonals
            if(i>0 && i==j){
                if(mBoardValues[i][j]==-1 || mBoardValues[i][j]!=mBoardValues[i-1][j-1]){
                    diag1Winner=false;
                }

                if(mBoardValues[i][(TOTAL_COLS-1)-j]==-1 ||
                   mBoardValues[i][(TOTAL_COLS-1)-j]!=mBoardValues[i-1][((TOTAL_COLS-1)-(j-1))]){
                    diag2Winner=false;
                }
            }
        }

        if(checkIfWin(winnerOption::ROW, rowWinner, i) ||
           checkIfWin(winnerOption::COL, colWinner, i)){
            mTimeToBlink = true;
            return false;
        }
    }

    if(checkIfWin(winnerOption::DIAG_1, diag1Winner, -1) ||
       checkIfWin(winnerOption::DIAG_2, diag2Winner, -1) ||
       checkIfWin(winnerOption::BOARD_FULL, boardFull, -1)
            ){
        mTimeToBlink = true;
        return false;
    }

    mWinningStatus = winnerOption::GAME_IN_PROGRESS;
    mWinningIndex = -1;

    return true;
}

///
/// \brief gameBoard::mousePressEvent Mouse clicked event handler
///
/// Miguel Mota
/// Jan 5 2021
///
/// \param event Event data
///
void gameBoard::mousePressEvent (QMouseEvent *event)
{
    if(mThinking){
        qDebug() << "let me think please...";
        return;
    }

    // Dow we have a winner?
    if(mWinningStatus == winnerOption::COL ||
       mWinningStatus == winnerOption::ROW ||
       mWinningStatus == winnerOption::DIAG_1 ||
       mWinningStatus == winnerOption::DIAG_2 ||
       mWinningStatus == winnerOption::BOARD_FULL){

        // Clear the board
        for(QVector<int> &row : mBoardValues){
            for(int &value : row){
                value = -1;
            }
        }

        // just for some nanoseconds user won if virtual user didn't!
        if(mWinningStatus == winnerOption::BOARD_FULL){
            mUserWon=true;
        }

        // Restart the game
        mWinningStatus = winnerOption::GAME_IN_PROGRESS;
        mWinningIndex = -1;
        mTimeToBlink = false;

        // If the Virtual user won or nobody won, he starts!
        // No need to check the status of the game
        // since the bard is empty an it is no possible to
        // win with a single move...
        if(!mUserWon){
            virtualPlayer::singleInstance()->makeYourNextMove(mBoardValues);
        }

        mUserWon = false;
        mBlink = true;

        update();
        qDebug() << "New game!!";
        return;
    }

    int ii=-1,jj=-1;
    for (int i = 0; i<TOTAL_LINES; i++){
        if(event->pos().x()>(mRectWidth/TOTAL_LINES)*i){
            ii = i;
        }
        if(event->pos().y()>(mRectHeight/TOTAL_LINES)*i){
            jj = i;
        }
    }

    setUserMove(ii, jj);
    update();

    qDebug() << "clicked : " << ii << ", " << jj;
}
