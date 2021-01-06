#ifndef VIRTUALPLAYER_H
#define VIRTUALPLAYER_H

#include <QVector>
#include <QSharedPointer>
#include <QDebug>

///
/// \brief The virtualPlayer class
///
/// Miguel Mota Jan 5 2021
///
///
class virtualPlayer
{
public:
    static QSharedPointer<virtualPlayer> singleInstance();

    virtualPlayer(){ qDebug() << "virtualPlayer constructor!"; };
    ~virtualPlayer() { qDebug() << "virtualPlayer destroyed!"; };

    void makeYourNextMove(QVector<QVector<int>> &board);

protected:
    static QSharedPointer<virtualPlayer> mInstance;

    static double multiplyVectors(const QVector<double> &v1, const QVector<double> &v2);
    double g(const double z) const { return (1/(qExp(-1*z)+1)); }; // Sigmoid function!
    QVector<double> getLayer(const QVector<double> input,
                             const QVector<QVector<double>> t,
                             const bool addBias) const;
};

#endif // VIRTUALPLAYER_H
