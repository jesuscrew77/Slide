#include "slide.h"
#include <qDebug>
#include <QFile>
#include <QTextStream>

QVector <QRgb> createColorTable ()
{
    QVector <QRgb> vector;
    for (int i = 0; i < 256; ++i)
    {
        vector.append(qRgb(i, i, i));
    }
    return vector;
}

QVector <QRgb> SlideCreator::colorTable = createColorTable();


SlideCreator::SlideCreator()
    :angleData(AngularDistanceOptions{}),
      slideData(StarSlideData{}),
      catalogData(Catalog{}),
      slideType(SLIDE_TYPE::INVALID_TYPE)
{

}

/*Расчет скорректированной по полиному дисторсии координаты пикселя*/
double SlideCreator::calculateDistorsio(double coordF, double coordS, const QList <double>& distCoef)
{
    double delta = distCoef[0] + distCoef[1] * coordF + distCoef[2] * coordS
            +distCoef[3] * pow(coordF, 2) + distCoef[4] * coordF * coordS
            +distCoef[5] * pow(coordS, 2)+ distCoef[6] * pow(coordF, 3)
            +distCoef[7] * pow(coordF, 2) * coordS + distCoef[8]* coordF * pow(coordS, 2)
            +distCoef[9] * pow(coordS, 3) + distCoef[10] * pow(coordF, 4)
            +distCoef[11] * pow(coordF, 3) * coordS + distCoef[12] * pow(coordS, 2) * pow(coordF, 2)
            +distCoef[13] * coordF * pow(coordS, 3) + distCoef[14] * pow(coordS, 4)
            +distCoef[15] * pow(coordF, 5) + distCoef[16] * pow(coordF, 4) * coordS
            +distCoef[17] * pow(coordF, 3) * pow(coordS, 2)
            +distCoef[18] * pow(coordF, 2) * pow(coordS, 3)
            +distCoef[19] * coordF * pow(coordS, 4)
            +distCoef[20] * pow(coordS, 5);
    return delta;
}
/*Рассчитывает переходную матрицу для пересчета угловых координат звезд на координаты слайда*/
QVector< QVector<double> > SlideCreator::calcTransitionMatrix(double pointAlpha, double pointBeta, double pointAzimut)
{

    QVector< QVector<double> > trMat(3);
    double PS, PC, QS, QC, RS, RC;
    for (int i = 0; i < trMat.size(); i++) trMat[i].resize(3);

    PS = sin(pointAzimut * transToRad); PC= cos(pointAzimut * transToRad);
    QS = sin(pointBeta * transToRad); QC = cos(pointBeta * transToRad);
    RS = sin(pointAlpha * transToRad); RC = cos(pointAlpha * transToRad);
    trMat[0][0] = -PC * RS - PS * RC * QS;
    trMat[0][1] = PC * RC - PS * RS * QS;
    trMat[0][2] = PS * QC;
    trMat[1][0] = PS * RS - PC * RC * QS;
    trMat[1][1] = -PS * RC - PC * RS * QS;
    trMat[1][2] = PC * QC;
    trMat[2][0] = QC * RC;
    trMat[2][1] = QC * RS;
    trMat[2][2] = QS;

    return trMat;

}

int SlideCreator::getStarSize(int defaultSize, int starPos)
{
    return starPos - (defaultSize / 2) < 0 ? defaultSize - abs(starPos - (defaultSize / 2)) : defaultSize;
}

int SlideCreator::getStarPos(int defaultSize, int starPos)
{
    return starPos - (defaultSize / 2) < 0 ? 0: starPos - (defaultSize / 2);
}

/*Проверяет, выходит ли текущий пиксель, составляющий звезду за границу изображение*/
/*startPos-одномерная координата левого верхнего пикселя звезды*/
/*x и y - координаты текущего пикселя*/
/*xSize и ySize - размер изображения*/
/*centralXc - x-координата центра звезды*/

bool SlideCreator::outOfImage(int startPos, int xSize, int ySize, int centralXc, int x, int y, int pixelPerStar)
{
    const int oneDimensionalPixPos = startPos + x + y * xSize;
    const int maxOneDimensionalPixPos = xSize * ySize;
    const int xCoordPixPosition = centralXc - (pixelPerStar / 2) + x;

    return (oneDimensionalPixPos <= 0
            || oneDimensionalPixPos >= maxOneDimensionalPixPos
            || xCoordPixPosition < 0
            || xCoordPixPosition >= xSize);
}


double SlideCreator::calcScalarProduct(double lOz, double lSt, double mOz, double mSt, double nOz, double nSt)
{
    double scalarProduct = lOz * lSt + mOz * mSt + nOz * nSt;

    if (scalarProduct > 1.0)// проверка на выход значения косинуса за диапазон [-1, 1]
    {
        scalarProduct = 1;
    }
    else if (scalarProduct < -1.0)
    {
        scalarProduct = -1;
    }
    return scalarProduct;
}

void SlideCreator::calcAngularDistancesWithSectors()
{
    // вычисляем направляющие косинусы точки проектирования
    double lOz = cos(slideData.pointBeta * transToRad) * cos(slideData.pointAlpha * transToRad);
    double mOz = cos(slideData.pointBeta * transToRad) * sin(slideData.pointAlpha * transToRad);
    double nOz = sin(slideData.pointBeta * transToRad);

    // Для входа в сектор

    QVector <double> lOzSec;
    lOzSec.reserve(catalogData.alphaVecSec().size());
    QVector <double> mOzSec;
    mOzSec.reserve(catalogData.alphaVecSec().size());
    QVector <double> nOzSec;
    nOzSec.reserve(catalogData.alphaVecSec().size());

    for (int i = 0; i < catalogData.alphaVecSec().size(); i++)// направляющие косинусы центров секторов
    {
        double cosD = cos(catalogData.betaVecSec()[i] * transToRad);
        double cosA = cos(catalogData.alphaVecSec()[i] * transToRad);
        double sinD = sin(catalogData.betaVecSec()[i] * transToRad);
        double sinA = sin(catalogData.alphaVecSec()[i] * transToRad);
        lOzSec.append(cosD * cosA);
        mOzSec.append(cosD * sinA);
        nOzSec.append(sinD);
    }

    // считаем косинусы между центром проектирования и центрами секторов, находим ближайший
    QVector <double> angleCosSec;
    angleCosSec.reserve(lOzSec.size());

    for (int i = 0; i < lOzSec.size(); i++)
    {
        auto scalarProduct = calcScalarProduct(lOz, lOzSec[i], mOz , mOzSec[i] , nOz, nOzSec[i]);
        angleCosSec.append(scalarProduct);
    }

    QVector<double>::iterator itSec;
    itSec = std::max_element(angleCosSec.begin(), angleCosSec.end());// вычисляем максимальной близкий к точке проецирования сектор
    int pos = itSec-angleCosSec.begin();

    QVector <int> starsInSector;
    for (int i = catalogData.shiftVec()[pos];i < (catalogData.shiftVec()[pos] + catalogData.countVecSec()[pos]); i++)
    {
        starsInSector.append(catalogData.newNumn()[i]);// формируем вектор, содержащий номера звезд в секторе
    }

    const auto countOfStars = starsInSector.size();
    angleData.resultBeta.reserve(countOfStars);
    angleData.resultAlpha.reserve(countOfStars);

    for (int i = 0; i < countOfStars; i++)
    {
        angleData.resultAlpha.append(catalogData.alphaVec()[starsInSector[i]]);
        angleData.resultBeta.append(catalogData.betaVec()[starsInSector[i]]);
    }


    QVector<double> lSt;
    lSt.reserve(countOfStars);
    QVector<double> mSt;
    mSt.reserve(countOfStars);
    QVector<double> nSt;
    nSt.reserve(countOfStars);

    for (int i = 0; i < countOfStars; i++)//направляющие косинусы звезд в секторе
    {
        double cosD = cos(angleData.resultBeta[i] * transToRad);
        double cosA = cos(angleData.resultAlpha[i] * transToRad);
        double sinD = sin(angleData.resultBeta[i] * transToRad);
        double sinA = sin(angleData.resultAlpha[i] * transToRad);
        lSt.append(cosD * cosA);
        mSt.append(cosD * sinA);
        nSt.append(sinD);
    }

    angleData.angleCos.reserve(countOfStars);

    for (int i = 0; i < countOfStars; i++)// косинусы между точкой проектирования и звездами в секторе
    {
        auto scalarProduct = calcScalarProduct(lOz, lSt[i], mOz, mSt[i], nOz, nSt[i]);
        angleData.angleCos.append(scalarProduct);
    }

}
void SlideCreator::calcAngularDistancesNoSectors()
{
    // вычисляем направляющие косинусы точки проектирования
    double lOz = cos(slideData.pointBeta * transToRad) * cos(slideData.pointAlpha * transToRad);
    double mOz = cos(slideData.pointBeta * transToRad) * sin(slideData.pointAlpha * transToRad);
    double nOz = sin(slideData.pointBeta * transToRad);

    const auto countOfStars = catalogData.alphaVec().size();
    QVector <double> lSt;
    lSt.reserve(countOfStars);
    QVector <double> mSt;
    mSt.reserve(countOfStars);
    QVector <double> nSt;
    nSt.reserve(countOfStars);

    for (int i = 0;i < countOfStars; i++)
    {
        // вычисляем направляющие косинусы всех звезд
        double cosD = cos(catalogData.betaVec()[i] * transToRad);
        double cosA = cos(catalogData.alphaVec()[i] * transToRad);
        double sinD = sin(catalogData.betaVec()[i] * transToRad);
        double sinA = sin(catalogData.alphaVec()[i]  * transToRad);
        lSt.append(cosD * cosA);
        mSt.append(cosD * sinA);
        nSt.append(sinD);
    }

    angleData.angleCos.reserve(countOfStars);
    for (int i = 0;i < countOfStars; i++)
    {
        // вычисляем косинус угла между точкой проецирования и координатами звезд
        auto scalarProduct = calcScalarProduct(lOz, lSt[i], mOz, mSt[i], nOz, nSt[i]);
        angleData.angleCos.append(scalarProduct);
    }

    angleData.resultAlpha = catalogData.alphaVec();
    angleData.resultBeta = catalogData.betaVec();
}

void SlideCreator::calcViewAngle(double& viewAngleX, double& viewAngleY, double& viewAngle)
{
    // считаем угол зрения в зависимости от фокусного расстояния и размерности матрицы
    viewAngleX = atan(((slideData.slideSizeX / 2 * slideData.pix) / slideData.focStart)) * 2 * transToGrad;
    viewAngleY = atan(((slideData.slideSizeY / 2 * slideData.pix) / slideData.focStart)) * 2 * transToGrad;
    // угол зрения - описанная окружность
    viewAngle = sqrt(pow(viewAngleX, 2) + pow(viewAngleY, 2));
}


/*Рассчитывает угловые расстояние между точкой проецирования и звездами для звездного слайда, а так же переходную матрицу на координаты слайда*/
void SlideCreator::calculateAngularDistOptions(const StarSlideData& _slideData, const Catalog& _catData, bool checkSector)
{

    starSlideDataPrepare = false;
    slideData = _slideData;
    catalogData = _catData;
    if (checkSector) calcAngularDistancesWithSectors();
    else calcAngularDistancesNoSectors();
    auto trMat = calcTransitionMatrix(slideData.pointAlpha, slideData.pointBeta, slideData.pointAzimut);
    angleData.trMat = trMat;
    starSlideDataPrepare = true;

}


/*возвращает вспомогательные слайды в виде сетки*/
QVector <StarParameters> SlideCreator::createGridSlide(const GridSlideData& gridD, bool checkDistorsio, const DistorsioData& distData)
{

    QVector <double> xv;
    QVector <double> yv;
    QVector <double> dxv;
    QVector <double> dyv;
    slideType = SLIDE_TYPE::INVALID_TYPE;
    QScopedArrayPointer <unsigned char> dataImage(new unsigned char [gridD.slideSizeX * gridD.slideSizeY]);
    QVector <StarParameters> coordsOfStars;
    if (checkDistorsio)
    {
        for (int y = gridD.gridDistance; y < (gridD.slideSizeY - gridD.gridDistance); y += (gridD.gridDistance + gridD.pixelPerStar))
        {
            for (int x = gridD.gridDistance; x < (gridD.slideSizeX - gridD.gridDistance); x += (gridD.gridDistance + gridD.pixelPerStar))
            {

                double yMM = (y - gridD.slideSizeY / 2) * gridD.pix;
                double xMM = (x - gridD.slideSizeX / 2) * gridD.pix;
               // xv.append(xMM); yv.append(yMM);
                xMM += calculateDistorsio(xMM, yMM, distData.xDistorsioVector);
                yMM += calculateDistorsio(xMM, yMM, distData.yDistorsioVector);
                //dxv.append(calculateDistorsio(xMM, yMM, distData.xDistorsioVector));
                //dyv.append(calculateDistorsio(xMM, yMM, distData.yDistorsioVector));
                int yPix = ((yMM/gridD.pix) + 0.5) + gridD.slideSizeY / 2;
                int xPix = ((xMM/gridD.pix) + 0.5) + gridD.slideSizeX / 2;

                StarParameters starParameters;
                starParameters.x = getStarPos(gridD.pixelPerStar, xPix);
                starParameters.y = getStarPos(gridD.pixelPerStar, yPix);
                starParameters.sizeX = getStarSize(gridD.pixelPerStar, xPix);
                starParameters.sizeY = getStarSize(gridD.pixelPerStar, yPix);
                coordsOfStars.append(starParameters);

                int startPos = xPix + yPix * gridD.slideSizeX;

                for (int i = 0;i < gridD.pixelPerStar; i++)/*i-Y, j-X*/
                {
                    for (int j = 0;j < gridD.pixelPerStar; j++)
                    {
                        if (outOfImage(startPos, gridD.slideSizeX, gridD.slideSizeY, xPix, j , i, gridD.pixelPerStar))
                        {
                            continue;
                        }
                        dataImage[startPos + j + i * gridD.slideSizeX] = 255;
                    }

                }
            }
        }
//        QFile test("xydxdy.txt");
//        if (test.open(QIODevice::WriteOnly))
//        {
//            QTextStream out(&test);
//            for(int i = 0; i < xv.size(); i++)
//            {
//                out << QString("%1 %2 %3 %4\n").arg(xv[i] + dxv[i]).arg(yv[i] + dyv[i]).arg(dxv[i]).arg(dyv[i]);
//            }
//        }

    }

    else
    {
        for (int y = gridD.gridDistance; y < (gridD.slideSizeY-gridD.gridDistance); y += (gridD.gridDistance+gridD.pixelPerStar))
        {
            for (int x = gridD.gridDistance; x < (gridD.slideSizeX-gridD.gridDistance); x += (gridD.gridDistance+gridD.pixelPerStar))
            {
                StarParameters starParameters;
                starParameters.x = getStarPos(gridD.pixelPerStar, x);
                starParameters.y = getStarPos(gridD.pixelPerStar, y);
                starParameters.sizeX = getStarSize(gridD.pixelPerStar, x);
                starParameters.sizeY = getStarSize(gridD.pixelPerStar, y);
                coordsOfStars.append(starParameters);
                int startPos = x + y * gridD.slideSizeX;
                for (int i = 0; i < gridD.pixelPerStar; i++)
                {
                    for (int j = 0;j < gridD.pixelPerStar; j++)
                    {
                        if (outOfImage(startPos, gridD.slideSizeX, gridD.slideSizeY, x, i, j, gridD.pixelPerStar))
                        {
                            continue;
                        }
                        dataImage[startPos + j + i * gridD.slideSizeX] = 255;
                    }

                }
            }
        }
    }

    QImage img(dataImage.data(), gridD.slideSizeX, gridD.slideSizeY, gridD.slideSizeX, QImage::Format_Grayscale8);
    img.setColorTable(colorTable);
    optimalImage.reset(new QImage(img.convertToFormat(QImage::Format_RGB32)));
    slideType = SLIDE_TYPE::GRID_TYPE;
    return coordsOfStars;
}


/*Перед выполнением этой функции должны быть рассчитаны угловые расстояния между точкой проецирования и звездами, т.е выполнена функция calculateAngularDistOptions  */
SlideParameters SlideCreator::createStarSlide(double focus, bool checkSector, bool checkDistorsio, const DistorsioData& distData = DistorsioData {})
{
    slideType = SLIDE_TYPE::INVALID_TYPE;
    if (!starSlideDataPrepare) return SlideParameters {};

    double viewAngleX, viewAngleY, viewAngle;
    calcViewAngle(viewAngleX, viewAngleY, viewAngle);


    QVector <double> resultAlpha;
    QVector <double> resultBeta;

    for (int i = 0; i < angleData.angleCos.size(); i++)
    {
        if (checkSector)// если учитываем сектора
        {
            double fieldOfViewCos = cos(viewAngle * transToRad);
            if ((angleData.angleCos[i]) >= fieldOfViewCos
                    && catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                resultAlpha.append(angleData.resultAlpha[i]);
                resultBeta.append(angleData.resultBeta[i]);

            }
        }
        else // если не учитываем
        {
            double insideDiamOfViewCos = cos(slideData.insideViewAngle / 2 * transToRad);
            if (angleData.angleCos[i] >= insideDiamOfViewCos
                    && catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                resultAlpha.append(angleData.resultAlpha[i]);
                resultBeta.append(angleData.resultBeta[i]);
            }
        }
    }
    QVector <double> filteredLSt;
    QVector <double> filteredMSt;
    QVector <double> filteredNSt;

    for (int i = 0;i < resultBeta.size(); i++)
    {
        double cosD = cos(resultBeta[i] * transToRad);
        double cosA = cos(resultAlpha[i] * transToRad);
        double sinD = sin(resultBeta[i] * transToRad);
        double sinA = sin(resultAlpha[i] * transToRad);

        filteredLSt.append(cosD * cosA);
        filteredMSt.append(cosD * sinA);
        filteredNSt.append(sinD);
    }


    QScopedArrayPointer <unsigned char> dataImage;
    if (!onlyParameters)
        dataImage.reset(new unsigned char [slideData.slideSizeX * slideData.slideSizeY]);

    QVector <StarParameters> coordsOfStars;

    double xCoordMM, yCoordMM;
    int xCoord, yCoord;
    double CC;
    int countOfStars = 0;//для подсчёта числа звезд, попавших на слайд
    for (int i = 0;i < filteredLSt.size(); i++)
    {

        CC = angleData.trMat[2][0] * filteredLSt[i]
                +
                angleData.trMat[2][1] * filteredMSt[i]
                +
                angleData.trMat[2][2] * filteredNSt[i];

        if (checkDistorsio)
        {

            xCoordMM = (-focus * (angleData.trMat[0][0] * filteredLSt[i]
                    + angleData.trMat[0][1] * filteredMSt[i]
                    + angleData.trMat[0][2] * filteredNSt[i]) / CC);
            yCoordMM = (-focus * (angleData.trMat[1][0] * filteredLSt[i]
                    + angleData.trMat[1][1] * filteredMSt[i]
                    + angleData.trMat[1][2] * filteredNSt[i]) / CC);
            xCoordMM += calculateDistorsio (xCoordMM, yCoordMM, distData.xDistorsioVector);
            yCoordMM += calculateDistorsio (xCoordMM, yCoordMM, distData.yDistorsioVector);

            xCoord = xCoordMM / slideData.pix + 0.5;
            yCoord = yCoordMM / slideData.pix + 0.5;

        }

        else
        {
            xCoord = (-focus * (angleData.trMat[0][0] * filteredLSt[i]
                    + angleData.trMat[0][1] * filteredMSt[i]
                    + angleData.trMat[0][2]*filteredNSt[i]) / CC) / slideData.pix + 0.5;
            yCoord = (-focus * (angleData.trMat[1][0] * filteredLSt[i]
                    + angleData.trMat[1][1] * filteredMSt[i]
                    + angleData.trMat[1][2] * filteredNSt[i]) / CC) / slideData.pix + 0.5;

        }

        xCoord += slideData.slideSizeX / 2; yCoord += slideData.slideSizeY / 2;// x_coord, slideSizeX- ось X, y_coord, slideSizeY - ось Y

        if (xCoord > 0 && xCoord < slideData.slideSizeX && yCoord > 0 && yCoord < slideData.slideSizeY)
        {
            StarParameters starParameters;
            starParameters.x = getStarPos(slideData.pixelPerStar, xCoord);
            starParameters.y = getStarPos(slideData.pixelPerStar, yCoord);
            starParameters.sizeX = getStarSize(slideData.pixelPerStar, xCoord);
            starParameters.sizeY = getStarSize(slideData.pixelPerStar, yCoord);
            coordsOfStars.append(starParameters);


            int posPix = yCoord * slideData.slideSizeX + xCoord;
            int startPos = posPix - (slideData.pixelPerStar/2) * slideData.slideSizeX - (slideData.pixelPerStar/2);

            for (int y = 0; y < slideData.pixelPerStar; y++)
            {
                for (int x = 0; x < slideData.pixelPerStar; x++)
                {

                    if (outOfImage(startPos, slideData.slideSizeX, slideData.slideSizeY, xCoord, x, y, slideData.pixelPerStar))
                    {
                        continue;
                    }
                    if (!onlyParameters)
                        dataImage[startPos + x + y * slideData.slideSizeX] = 255;
                }

            }
            ++ countOfStars;// подсчёт числа звезд в кадре
        }

    }

    if(!onlyParameters)
    {
        QImage img(dataImage.data(), slideData.slideSizeX, slideData.slideSizeY, slideData.slideSizeX, QImage::Format_Grayscale8);
        img.setColorTable(colorTable);
        optimalImage.reset(new QImage(img.convertToFormat(QImage::Format_Mono)));
    }

    SlideParameters imageData;
    imageData.countOfStars = countOfStars;
    imageData.viewAngleX = viewAngleX;
    imageData.viewAngleY = viewAngleY;
    imageData.coordsOfStars = coordsOfStars;

    slideType = SLIDE_TYPE::STAR_TYPE;
    return imageData;

}
/* Выдает предварительную информацию о размерах слайда, числе звезд на слайде.*/
/* Если слайдов несколько выдает так же информацию об размере общего изображения*/
TestSlideParameters SlideCreator::testStarSlide (bool checkSector, bool checkDistorsio, const DistorsioData &distData)
{
    if (!starSlideDataPrepare)
        return TestSlideParameters {};
    int countOfStars = 0;//для подсчёта числа звезд, попавших на слайд

    double viewAngleX, viewAngleY, viewAngle;
    calcViewAngle(viewAngleX, viewAngleY, viewAngle);

    QVector <double> resultAlpha;
    QVector <double> resultBeta;

    for (int i = 0;i < angleData.angleCos.size();i ++)
    {
        if (checkSector)
        {
            double fieldOfViewCos = cos(viewAngle * transToRad);
            if (angleData.angleCos[i] >= fieldOfViewCos
                    && catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                resultAlpha.append(angleData.resultAlpha[i]);
                resultBeta.append(angleData.resultBeta[i]);

            }
        }
        else
        {
            double insideFieldOfViewCos = cos(slideData.insideViewAngle / 2 * transToRad);
            if (angleData.angleCos[i] >= insideFieldOfViewCos
                    &&  catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                resultAlpha.append(angleData.resultAlpha[i]);
                resultBeta.append(angleData.resultBeta[i]);
            }
        }
    }


    QVector <double> filteredLSt;
    QVector <double> filteredMSt;
    QVector <double> filteredNSt;

    for (int i = 0; i < resultBeta.size(); i++)
    {
        double cosD = cos(resultBeta[i] * transToRad);
        double cosA = cos(resultAlpha[i] * transToRad);
        double sinD = sin(resultBeta[i] * transToRad);
        double sinA = sin(resultAlpha[i] * transToRad);

        filteredLSt.append(cosD * cosA);
        filteredMSt.append(cosD * sinA);
        filteredNSt.append(sinD);
    }

    double xCoordMM, yCoordMM;
    int xCoord, yCoord;
    double CC;

    for (int i = 0; i < filteredLSt.size(); i++)
    {
        CC = angleData.trMat[2][0] * filteredLSt[i]
                +
                angleData.trMat[2][1] * filteredMSt[i]
                +
                angleData.trMat[2][2] * filteredNSt[i];

        if (checkDistorsio)
        {

            xCoordMM = (-slideData.focStart * (angleData.trMat[0][0] * filteredLSt[i]
                    + angleData.trMat[0][1] * filteredMSt[i]
                    + angleData.trMat[0][2] * filteredNSt[i]) / CC);
            yCoordMM = (-slideData.focStart * (angleData.trMat[1][0] * filteredLSt[i]
                    + angleData.trMat[1][1] * filteredMSt[i]
                    + angleData.trMat[1][2] * filteredNSt[i]) / CC);
            xCoordMM += calculateDistorsio (xCoordMM, yCoordMM, distData.xDistorsioVector);
            yCoordMM += calculateDistorsio (xCoordMM, yCoordMM, distData.yDistorsioVector);
            xCoord = xCoordMM / slideData.pix + 0.5;
            yCoord = yCoordMM / slideData.pix + 0.5;
        }

        else
        {
            xCoord = (-slideData.focStart * (angleData.trMat[0][0] * filteredLSt[i]
                    + angleData.trMat[0][1] * filteredMSt[i]
                    + angleData.trMat[0][2] * filteredNSt[i]) / CC) / slideData.pix + 0.5;
            yCoord = (-slideData.focStart * (angleData.trMat[1][0] * filteredLSt[i]
                    + angleData.trMat[1][1] * filteredMSt[i]
                    + angleData.trMat[1][2] * filteredNSt[i]) / CC) / slideData.pix + 0.5;

        }

        xCoord += slideData.slideSizeX / 2; yCoord += slideData.slideSizeY / 2;// x_coord, slideSizeX- ось X, y_coord, slideSizeY - ось Y

        if ( xCoord > 0
             && xCoord < slideData.slideSizeX
             && yCoord > 0
             && yCoord < slideData.slideSizeY)
        {
            ++countOfStars; // подсчёт числа звезд
        }

    }

    TestSlideParameters testData;
    testData.viewAngleX = viewAngleX;
    testData.viewAngleY = viewAngleY;
    testData.countOfStars = countOfStars;
    return testData;

}

QSharedPointer <QImage> SlideCreator::getSlidePointer() noexcept
{
    return optimalImage;
}


QDomDocument SlideCreator::createFullSvg(QVector <QVector <StarParameters>> coordsOfStars,
                                         const int imageWidth, const int imageHeight,
                                         const GroupImgParams& gParams,
                                         const InscriptParams& iParams,
                                         QVector <QString> setableText)
{
    int sizeX = gParams.countX * imageWidth + gParams.countX * gParams.space;
    int sizeY = gParams.countY * imageHeight + gParams.countX * gParams.space;
    QDomDocument doc("pic");
    QDomElement svg = doc.createElement("svg");
    svg.setAttribute("viewBox", QString("0 ")+"0 "+ QString::number(sizeX) + " " + QString::number(sizeY));
    svg.setAttribute("xmlns", "http://www.w3.org/2000/svg");
    svg.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    svg.setAttribute("encoding", "UTF-8");
    svg.setAttribute("width", QString::number(sizeX));
    svg.setAttribute("height", QString::number(sizeY));
    doc.appendChild(svg);

    for (int i = 0; i < gParams.countY; i++)
    {
        for (int j = 0; j < gParams.countX; j++)
        {
            int slidePos = i * gParams.countX + j;
            int xOffset = (imageWidth + gParams.space) * j;
            int yOffset = (imageHeight + gParams.space) * i;
            QDomElement cube = doc.createElement("rect");
            cube.setAttribute("x", QString::number(xOffset));
            cube.setAttribute("y", QString::number(yOffset));
            cube.setAttribute("width", QString::number(imageWidth));
            cube.setAttribute("height", QString::number(imageHeight));
            cube.setAttribute("fill", "black");
            svg.appendChild(cube);
            for (auto& starCoordinate : coordsOfStars[slidePos])
            {
                QDomElement star = doc.createElement("rect");
                star.setAttribute("fill", "white");
                star.setAttribute("x", QString::number(starCoordinate.x + xOffset));
                star.setAttribute("y", QString::number(starCoordinate.y + yOffset));
                star.setAttribute("width", QString::number(starCoordinate.sizeX));
                star.setAttribute("height", QString::number(starCoordinate.sizeY));
                svg.appendChild(star);

            }
            QDomElement textElement = doc.createElement("text");
            QDomText text = doc.createTextNode(setableText[slidePos]);
            textElement.setAttribute("x", QString::number(iParams.fontX + xOffset));
            textElement.setAttribute("y", QString::number(iParams.fontY + yOffset));
            textElement.setAttribute("style", "fill: white;");
            textElement.setAttribute("font-size", QString::number(iParams.fontSize - 30));
            textElement.appendChild(text);
            svg.appendChild(textElement);
        }
    }

    return doc;
}

QDomDocument SlideCreator::createFullSvgSymbols(QVector <QVector <StarParameters>> coordsOfStars,
                                                const int imageWidth, const int imageHeight,
                                                const GroupImgParams& gParams,
                                                const InscriptParams& iParams,
                                                QVector <QString> setableText)
{
    QDomDocument doc("pic");
    QDomElement svg = doc.createElement("svg");
    int sizeX = gParams.countX * imageWidth + gParams.countX * gParams.space;
    int sizeY = gParams.countY * imageHeight + gParams.countX * gParams.space;

    svg.setAttribute("viewBox", QString("0 ")+ "0 " + QString::number(sizeX) + " " + QString::number(sizeY));
    svg.setAttribute("xmlns", "http://www.w3.org/2000/svg");
    svg.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    svg.setAttribute("encoding", "UTF-8");
    svg.setAttribute("width", QString::number(sizeX));
    svg.setAttribute("height", QString::number(sizeY));
    doc.appendChild(svg);

    QDomElement defs = doc.createElement("defs");
    svg.appendChild(defs);

    for (int i = 0; i < gParams.countY; i++)
    {
        for (int j = 0; j < gParams.countX; j++)
        {
            int slidePos = i * gParams.countX + j;
            QDomElement symbol = doc.createElement("symbol");
            symbol.setAttribute("id", "slide" + QString::number(slidePos));
            defs.appendChild(symbol);
            QDomElement cube = doc.createElement("rect");
            cube.setAttribute("x", "0");
            cube.setAttribute("y", "0");
            cube.setAttribute("width", QString::number(imageWidth));
            cube.setAttribute("height", QString::number(imageHeight));
            cube.setAttribute("fill", "black");
            symbol.appendChild(cube);
            for (auto& starCoordinate : coordsOfStars[slidePos])
            {
                QDomElement star = doc.createElement("rect");
                star.setAttribute("fill", "white");
                star.setAttribute("x", QString::number(starCoordinate.x));
                star.setAttribute("y", QString::number(starCoordinate.y));
                star.setAttribute("width", QString::number(starCoordinate.sizeX));
                star.setAttribute("height", QString::number(starCoordinate.sizeY));
                symbol.appendChild(star);
            }
            QDomElement textElement = doc.createElement("text");
            QDomText text = doc.createTextNode(setableText[slidePos]);
            textElement.setAttribute("x", QString::number(iParams.fontX));
            textElement.setAttribute("y", QString::number(iParams.fontY));
            textElement.setAttribute("style", "fill: white;");
            textElement.setAttribute("font-size", QString::number(iParams.fontSize - 30));
            textElement.appendChild(text);
            symbol.appendChild(textElement);
        }
    }


    for (int i = 0; i < gParams.countY; i++)
    {
        for (int j = 0; j < gParams.countX; j++)
        {
            int slidePos = i * gParams.countX + j;
            QDomElement use = doc.createElement("use");
            use.setAttribute("xlink:href", "#slide" + QString::number(slidePos));
            use.setAttribute("x", QString::number(j * imageWidth + j * gParams.space));
            use.setAttribute("y", QString::number(i * imageHeight + i * gParams.space));
            svg.appendChild(use);
        }
    }

    return doc;
}



QDomDocument SlideCreator::createSvg (const StarSlideData& sData,
                                      const InscriptParams& iParams,
                                      const QString setableText,
                                      QVector <StarParameters> coordsOfStars)
{

    QDomDocument doc("pic");
    QDomElement svg = doc.createElement("svg");
    int imageWidth = sData.slideSizeX;
    int imageHeight = sData.slideSizeY;
    svg.setAttribute("viewBox", QString("0 ")+"0 "+ QString::number(imageWidth) +" "+ QString::number(imageHeight));
    svg.setAttribute("xmlns", "http://www.w3.org/2000/svg");
    svg.setAttribute("encoding", "UTF-8");
    svg.setAttribute("width", QString::number(imageWidth));
    svg.setAttribute("height", QString::number(imageHeight));

    QDomElement cube = doc.createElement("rect");
    cube.setAttribute("x", "0");
    cube.setAttribute("y", "0");
    cube.setAttribute("width", QString::number(imageWidth));
    cube.setAttribute("height", QString::number(imageHeight));
    cube.setAttribute("fill", "black");
    doc.appendChild(svg);
    svg.appendChild(cube);
    for (auto& starCoordinate : coordsOfStars)
    {
        QDomElement star = doc.createElement("rect");
        star.setAttribute("x", QString::number(starCoordinate.x));
        star.setAttribute("fill", "white");
        star.setAttribute("y", QString::number(starCoordinate.y));
        star.setAttribute("width", QString::number(starCoordinate.sizeX));
        star.setAttribute("height", QString::number(starCoordinate.sizeY));
        svg.appendChild(star);
    }
    QDomElement textElement = doc.createElement("text");
    QDomText text = doc.createTextNode(setableText);
    textElement.setAttribute("x", QString::number(iParams.fontX));
    textElement.setAttribute("y", QString::number(iParams.fontY));
    textElement.setAttribute("style", "fill: white;");
    textElement.setAttribute("font-size", QString::number(iParams.fontSize - 30));
    textElement.appendChild(text);
    svg.appendChild(textElement);

    return doc;
}

QDomDocument SlideCreator::createSvg (const GridSlideData& gData,
                                      const InscriptParams& iParams,
                                      const QString setableText,
                                      QVector <StarParameters> coordsOfStars)
{
    StarSlideData sData;
    sData.slideSizeX = gData.slideSizeX;
    sData.slideSizeY = gData.slideSizeY;
    return createSvg(sData, iParams, setableText, coordsOfStars);
}
