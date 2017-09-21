#include <catalog.h>


using namespace std;
void Catalog::openCatalog(const QString& filename, bool& status, QString& error)
{
    clear();
    status = false;
    QString bufFilename;
    QString filenameAdd;

    if (sizeof(DataStar) != structSize)
    {
        error = "Размер структуры не соответствует заданному. Обратитесь к разработчику.";
        return;
    }

    std::ifstream in(filename.toLocal8Bit().constData(),ios::binary);
    QVector <DataStar> dataStarVec;
    if (in.is_open())
    {
        DataStar generalCat;
        while(in.read((char*)&generalCat,sizeof(generalCat)))// считываем каталог в вектор структур
        {
            dataStarVec.append(generalCat);
        }
        in.close();
    }
    else
    {
        error = "Ошибка открытия основного файла каталога";
        return;
    }

    for (QVector<DataStar>::iterator it = dataStarVec.begin();it != dataStarVec.end();it ++)// расшифровываем считанный каталог
    {
        alphaAngles.append((it->alpha) * div * (transToGrad));
        betaAngles.append((it->beta) * div * (transToGrad));
        mv.append(((it->mv) - 20));
    }
    dataStarVec.clear();

    for (int i = 0;i < mv.size();i ++)
    {
        mv[i] = mv[i] / 10;
    }

    bufFilename = filename;
    filenameAdd = bufFilename.remove(filename.lastIndexOf("."),filename.end() - filename.begin());
    filenameAdd.append("_SEC.CAT");
    in.open(filenameAdd.toLocal8Bit().constData(),ios::binary);

    QVector <Sectors> secVec;
    if (in.is_open())
    {
        Sectors secCat;
        while(in.read((char*)&secCat,sizeof(Sectors)))// считываем каталог в вектор структур
        {
            secVec.append(secCat);
        }
        in.close();
    }
    else
    {
        error = "Ошибка открытия файла секторов";
        return;
    }

    for (QVector<Sectors>::iterator it = secVec.begin();it != secVec.end();it ++)
    {
        alphaAnglesSec.append((it->alpha_c)*(transToGrad));
        betaAnglesSec.append((it->beta_c)*(transToGrad));
        countSec.append(it->count_in_sector);
        shift.append(it->shift);
    }
    secVec.clear();


    filenameAdd = bufFilename.remove(filename.lastIndexOf("."), filename.end() - filename.begin());
    filenameAdd.append("_NUM.CAT");
    in.open(filenameAdd.toLocal8Bit().constData(),ios::binary);
    if (in.is_open())
    {
        Numbers number;
        while(in.read((char*)&number.num,sizeof(number.num)))// считываем каталог в вектор
        {
            newNumbers.append(number.num);
        }
        in.close();
    }
    else
    {
        error = "Ошибка открытия файла номеров";
        return;
    }

    for (QVector<short>::iterator it = newNumbers.begin();it != newNumbers.end();it++)
    {
        if (*it < 0)
        {
            *it = *it*(-1);// избавляемся от отрицательных значений
        }
    }
    for (int i = 0;i < newNumbers.size(); i ++)
    {
        newNumbers[i] = newNumbers[i] - 1;
    }
    status = true;

}

void Catalog::clear()
{
    alphaAngles.clear();
    betaAngles.clear();
    mv.clear();
    alphaAnglesSec.clear();
    betaAnglesSec.clear();
    countSec.clear();
    shift.clear();
    newNumbers.clear();
}


