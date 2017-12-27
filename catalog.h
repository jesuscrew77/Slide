#ifndef CATALOG_H
#define CATALOG_H

#include <QtGlobal>
#include <QVector>
#include <fstream>
#include <iostream>

class Catalog /*структура, содержащая звездный каталог*/
{
public:
    void openCatalog(const QString& filename,  bool& status, QString& error);

    const QVector<double>& alphaVec() const noexcept {
        return alphaAngles;
    }
    void setAlphaVec(const QVector <double>& vec) noexcept
    {
        alphaAngles = vec;
    }

    const QVector<double>& betaVec() const noexcept {
        return betaAngles;
    }

    void setBetaVec(const QVector <double>& vec) noexcept
    {
        betaAngles = vec;
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


    const QVector<double>& betaVecSec() const noexcept {
        return betaAnglesSec;
    }

    void setBetaVecSec(const QVector <double>& vec) noexcept
    {
        betaAnglesSec = vec;
    }

    const QVector<long>& countVecSec() const noexcept {

        return countSec;
    }

    void setCountVecSec(const QVector <long>& vec) noexcept
    {
        countSec = vec;
    }

    const QVector<long>& shiftVec() const noexcept {
        return shift;
    }

    void setshiftVec(const QVector <long>& vec) noexcept
    {
        shift = vec;
    }

    const QVector<short>& newNumn() const noexcept {
        return newNumbers;
    }

    void setNewNumn(const QVector <short>& vec) noexcept
    {
        newNumbers = vec;
    }




private:
    void clear();

    constexpr  static double transToGrad = 57.29577957855229;
    constexpr  static double div = 0.00000001;
    constexpr  static int structSize = 18;

    QVector <double> alphaAngles;
    QVector <double> betaAngles;
    QVector <float> mv;
    QVector <double> alphaAnglesSec;
    QVector <double> betaAnglesSec;
    QVector <long> countSec;
    QVector <long> shift;
    QVector <short> newNumbers;
};

#pragma pack(push,1)
struct Sectors // каталог секторов
{
    float alpha_c;
    float beta_c;
    qint16 count_in_sector;
    int shift;
};
#pragma pack(pop)



#pragma pack(push,1)
struct DataStar // основной каталог/бортовой каталог
{
    qint32  NSAO;
    qint32 alpha;
    qint32 beta;
    qint16 ualpha;
    qint16 ubeta;
    unsigned char mv;
    char sp;
};
#pragma pack(pop)



#pragma pack(push,1)
struct Numbers // основной каталог/бортовой каталог
{
    qint16 num;
};
#pragma pack(pop)

#endif // CATALOG_H
