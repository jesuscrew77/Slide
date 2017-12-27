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

    QFile catFile (filename);
    QDataStream in;
    QVector <DataStar> dataStarVec;
    if (catFile.open(QIODevice::ReadOnly))
    {
        in.setDevice(&catFile);
        DataStar generalCat;
        while(in.readRawData((char*)&generalCat,sizeof(generalCat)))// считываем каталог в вектор структур
        {
            dataStarVec.append(generalCat);
        }
        catFile.close();
    }
    else
    {
        error = "Ошибка открытия основного файла каталога";
        return;
    }

    for (QVector<DataStar>::iterator it = dataStarVec.begin(); it != dataStarVec.end(); it++)// расшифровываем считанный каталог
    {
        alphaAngles.append(it->alpha * div * transToGrad);
        deltaAngles.append(it->delta * div * transToGrad);
        mv.append((it->mv - 20) / 10);
    }
    dataStarVec.clear();

    bufFilename = filename;
    filenameAdd = bufFilename.remove(filename.lastIndexOf("."), filename.end() - filename.begin());
    filenameAdd.append("_SEC.CAT");

    catFile.setFileName(filenameAdd);
    QVector <Sectors> secVec;
    if (catFile.open(QIODevice::ReadOnly))
    {
        Sectors secCat;
        while(in.readRawData((char*)&secCat,sizeof(Sectors)))// считываем каталог в вектор структур
        {
            secVec.append(secCat);
        }
        catFile.close();
    }
    else
    {
        error = "Ошибка открытия файла секторов";
        return;
    }

    for (QVector<Sectors>::iterator it = secVec.begin();it != secVec.end();it++)
    {
        alphaAnglesSec.append(it->alpha_c * transToGrad);
        deltaAnglesSec.append(it->delta_c * transToGrad);
        countSec.append(it->count_in_sector);
        shift.append(it->shift);
    }
    secVec.clear();

    filenameAdd = bufFilename.remove(filename.lastIndexOf("."), filename.end() - filename.begin());
    filenameAdd.append("_NUM.CAT");

    catFile.setFileName(filenameAdd);
    if (catFile.open(QIODevice::ReadOnly))
    {
        Numbers number;
        while(in.readRawData((char*)&number.num, sizeof(number.num)))// считываем каталог в вектор
        {
            newNumbers.append(number.num);
        }
        catFile.close();
    }
    else
    {
        error = "Ошибка открытия файла номеров";
        return;
    }

    for (QVector<qint16>::iterator it = newNumbers.begin();it != newNumbers.end(); it++)
    {
        if (*it < 0)
        {
            *it = *it * (-1);// избавляемся от отрицательных значений
        }
    }

    for (int i = 0;i < newNumbers.size(); i++)
    {
        newNumbers[i] = newNumbers[i] - 1;
    }
    status = true;

}

void Catalog::clear()
{
    alphaAngles.clear();
    deltaAngles.clear();
    mv.clear();
    alphaAnglesSec.clear();
    deltaAnglesSec.clear();
    countSec.clear();
    shift.clear();
    newNumbers.clear();
}


