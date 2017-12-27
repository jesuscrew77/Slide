#include <catalog.h>
#include <QDebug>
using namespace std;
void CatalogData::openCatalog(const QString& filename, bool& status, QString& error)
{

    data_star generalCat;
    sector secCat;
    QVector <sector> secVec;
    QVector <data_star> dataStarVec;
    QString bufFilename;
    QString filenameAdd;
    numbers number;

    if(sizeof(generalCat) != structSize)
    {
        error = "Размер структуры не соответствует заданному. Обратитесь к разработчику.";
        status = false;
        return;
    }

    std::ifstream in(filename.toLocal8Bit().constData(),ios::binary);
    if(in.is_open())
    {
        while(in.read((char*)&generalCat,sizeof(generalCat)))// считываем каталог в вектор структур
        {
            dataStarVec.append(generalCat);
        }
        in.close();
    }
    else
    {
        error = "Ошибка открытия файла";
        status = false;
        return;
    }

    for (QVector<data_star>::iterator it = dataStarVec.begin();it != dataStarVec.end();it ++)// расшифровываем считанный каталог
    {
        alphaAngles.append((it->alpha) * div * (transToGrad));
        betaAngles.append((it->beta) * div * (transToGrad));
        mv.append(((it->mv) - 20));
    }
    dataStarVec.clear();

    for(int i = 0;i < mv.size();i ++)
    {
        mv[i] = mv[i] / 10;
    }


    bufFilename = filename;
    filenameAdd = bufFilename.remove(filename.lastIndexOf("."),filename.end() - filename.begin());
    filenameAdd.append("_SEC.CAT");
    in.open(filenameAdd.toLocal8Bit().constData(),ios::binary);
    if(in.is_open())
    {
        while(in.read((char*)&secCat,sizeof(sector)))// считываем каталог в вектор структур
        {
            secVec.append(secCat);
        }
        in.close();
    }
    else
    {
        error = "Ошибка открытия файла";
        status = false;
        return;
    }

    for (QVector<sector>::iterator it = secVec.begin();it != secVec.end();it ++)
    {
        alphaAnglesSec.append((it->alpha_c)*(transToGrad));
        betaAnglesSec.append((it->beta_c)*(transToGrad));
        countSec.append(it->count_in_sector);
        shift.append(it->shift);
    }
    secVec.clear();


    filenameAdd = bufFilename.remove(filename.lastIndexOf("."),filename.end()-filename.begin());
    filenameAdd.append("_NUM.CAT");
    in.open(filenameAdd.toLocal8Bit().constData(),ios::binary);
    if(in.is_open())
    {
        while(in.read((char*)&number.num,sizeof(number.num)))// считываем каталог в вектор
        {
            newNumbers.append(number.num);
        }
        in.close();
    }
    else
    {
        error = "Ошибка открытия файла";
        status = false;
        return;
    }

    for(QVector<short>::iterator it = newNumbers.begin();it != newNumbers.end();it++)
    {
        if(*it < 0)
        {
            *it = *it*(-1);// избавляемся от отрицательных значений
        }
    }
    for(int i = 0;i <= newNumbers.size() - 1;i ++)
    {
        newNumbers[i] = newNumbers[i] - 1;
    }
    status = true;

//    int ii = 0;
//    for(int i = 0; i < alphaVec().size(); i ++)
//    {
//        if(betaVec()[i] > 31 &&  betaVec()[i] < 54 && alphaVec()[i] > 60 && alphaVec()[i] < 260
//                || betaVec()[i] > -33 &&  betaVec()[i] < 30  && alphaVec()[i] > 60 && alphaVec()[i] < 260
//                || betaVec()[i] > -51 &&  betaVec()[i] < 35  && alphaVec()[i] > 250 && alphaVec()[i] < 360
//                || betaVec()[i] > -51 &&  betaVec()[i] < 35  && alphaVec()[i] > 0 && alphaVec()[i] < 50
//                || betaVec()[i] > -49 &&  betaVec()[i] < -37 && alphaVec()[i] > 55 && alphaVec()[i] < 80
//                || betaVec()[i] > -49 &&  betaVec()[i] < 35  && alphaVec()[i] > 55 && alphaVec()[i] < 55)
//        {
//                ++ii;
//    }
//    }
//    qDebug() << ii;
    }
