#ifndef CATALOG_H
#define CATALOG_H

#include <QtGlobal>
#include <QVector>
#include <QFile>
#include <QDataStream>

class Catalog /*структура, содержащая звездный каталог*/
{
public:
    void openCatalog(const QString& filename,  bool& status, QString& error);

    const QVector <double> & alphaVec() const noexcept {
        return alphaAngles;
    }
    void setAlphaVec(const QVector <double>& vec) noexcept
    {
        alphaAngles = vec;
    }

    const QVector<double>& deltaVec() const noexcept {
        return deltaAngles;
    }

    void setBetaVec(const QVector <double>& vec) noexcept
    {
        deltaAngles = vec;
    }

    const QVector <float> & mvVec() const noexcept {
        return mv;
    }

    void setMvVec(const QVector <float>& vec) noexcept
    {
        mv = vec;
    }

    const QVector<double>& alphaVecSec() const noexcept {
        return alphaAnglesSec;
    }

    void setAlphaVecSec(const QVector <double>& vec) noexcept
    {
        alphaAnglesSec = vec;
    }


    const QVector<double>& deltaVecSec() const noexcept {
        return deltaAnglesSec;
    }

    void setBetaVecSec(const QVector <double>& vec) noexcept
    {
        deltaAnglesSec = vec;
    }

    const QVector<long>& countVecSec() const noexcept {

        return countSec;
    }

    void setCountVecSec(const QVector <long>& vec) noexcept
    {
        countSec = vec;
    }

    const QVector <long>& shiftVec() const noexcept {
        return shift;
    }

    void setshiftVec(const QVector <long>& vec) noexcept
    {
        shift = vec;
    }

    const QVector <qint16>& newNumn() const noexcept {
        return newNumbers;
    }

    void setNewNumn(const QVector <qint16>& vec) noexcept
    {
        newNumbers = vec;
    }




private:
    void clear();

    constexpr static double transToGrad = 57.29577957855229;
    constexpr static double div = 0.00000001;
    constexpr static int structSize = 18;

    QVector <double> alphaAngles;
    QVector <double> deltaAngles;
    QVector <float> mv;
    QVector <double> alphaAnglesSec;
    QVector <double> deltaAnglesSec;
    QVector <long> countSec;
    QVector <long> shift;
    QVector <qint16> newNumbers;
};

#pragma pack(push,1)
struct Sectors // каталог секторов
{
    float alpha_c;
    float delta_c;
    qint16 count_in_sector; //32 для гайа
    int shift;
};
#pragma pack(pop)



#pragma pack(push,1)
struct DataStar // основной каталог/бортовой каталог
{
    qint32  NSAO;
    qint32 alpha;
    qint32 delta;
    qint16 ualpha;
    qint16 udelta;
    unsigned char mv;
    char sp;
};
#pragma pack(pop)



#pragma pack(push,1)
struct Numbers // основной каталог/бортовой каталог
{
    qint16 num; //32 для гайа
};
#pragma pack(pop)

#endif // CATALOG_H
