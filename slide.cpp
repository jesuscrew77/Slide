#include "slide.h"
#include <qDebug>
#include <QFile>
#include <QTextStream>

QVector <QRgb> createColorTable ()
{
    QVector <QRgb> vector;
    for (int i = 0; i < 256; ++i)
    {
        vector.append(qRgb(i,i,i));
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
double SlideCreator::calculateDistorsio(double point_c, double coord_a, double coord_b, const QList <double>& distorsio_coef)
{
    point_c = point_c + distorsio_coef[0] + distorsio_coef[1]*coord_a + distorsio_coef[2] * coord_b
            +distorsio_coef[3] * pow( coord_a,2) + distorsio_coef[4] * coord_a * coord_b
            +distorsio_coef[5] * pow(coord_b,2)+ distorsio_coef[6] * pow( coord_a,3)
            +distorsio_coef[7] * pow( coord_a,2) * coord_b + distorsio_coef[8]* coord_a * pow(coord_b,2)
            +distorsio_coef[9] * pow(coord_b,3) + distorsio_coef[10] * pow(coord_a,4)
            +distorsio_coef[11] * pow(coord_a,3) * coord_b + distorsio_coef[12] * pow(coord_b,2) * pow(coord_a,2)
            +distorsio_coef[13] * coord_a * pow(coord_b,3) + distorsio_coef[14] * pow(coord_b,4)
            +distorsio_coef[15] * pow(coord_a,5) + distorsio_coef[16] * pow(coord_a,4) * coord_b
            +distorsio_coef[17] * pow(coord_a,3) * pow(coord_b,2)
            +distorsio_coef[18] * pow(coord_a,2) * pow(coord_b,3)
            +distorsio_coef[19] * coord_a * pow(coord_b,4)
            +distorsio_coef[20] * pow(coord_b,5);
    return point_c;
}
/*Рассчитывает переходную матрицу для пересчета угловых координат звезд на координаты слайда*/
QVector< QVector<float> > SlideCreator::calcTransitionMatrix(double pointAlpha, double pointBeta,double pointAzimut)
{

    QVector< QVector<float> > trMat(3);
    float PS,PC,QS,QC,RS,RC;
    for (int i = 0;i < trMat.size();i ++) trMat[i].resize(3);

    PS = sin(pointAzimut * transToRad); PC= cos(pointAzimut * transToRad);
    QS = sin(pointBeta * transToRad); QC = cos(pointBeta * transToRad);
    RS = sin(pointAlpha * transToRad); RC = cos(pointAlpha * transToRad);
    trMat[0][0] = -PC*RS-PS*RC*QS;
    trMat[0][1] = PC*RC-PS*RS*QS;
    trMat[0][2] = PS*QC;
    trMat[1][0] = PS*RS-PC*RC*QS;
    trMat[1][1] = -PS*RC-PC*RS*QS;
    trMat[1][2] = PC*QC;
    trMat[2][0] = QC*RC;
    trMat[2][1] = QC*RS;
    trMat[2][2] = QS;

    return trMat;

}

int SlideCreator::getStarSize(int default_size, int star_pos)
{
    return star_pos - (default_size / 2) < 0 ? default_size - abs(star_pos - (default_size / 2)) : default_size;
}

int SlideCreator::getStarPos(int default_size, int star_pos)
{
    return star_pos - (default_size / 2) < 0 ? 0: star_pos - (default_size / 2);
}

/*Проверяет, выходит ли текущий пиксель, составляющий звезду за границу изображение*/
/*start_pos-одномерная координата левого верхнего пикселя звезды*/
/*x и y - координаты текущего пикселя*/
/*x_size и y_size - размер изображения*/
/*central_x_c - x-координата центра звезды*/

bool SlideCreator::outOfImage(int start_pos, int x_size, int y_size, int central_x_c, int x, int y, int pixelPerStar)
{
    const int one_dimensional_pix_pos = start_pos + x + y * x_size;
    const int max_one_dimensional_pix_pos = x_size * y_size;
    const int x_coord_pix_position = central_x_c - (pixelPerStar / 2) + x;

    return (one_dimensional_pix_pos <= 0
            || one_dimensional_pix_pos >= max_one_dimensional_pix_pos
            || x_coord_pix_position < 0
            || x_coord_pix_position >= x_size);
}


double SlideCreator::calcScalarProduct(double l_oz, double l_st, double m_oz, double m_st, double n_oz, double n_st)
{
    double scalar_product = l_oz * l_st + m_oz * m_st + n_oz * n_st;

    if (scalar_product > 1.0)// проверка на выход значения косинуса за диапазон [-1,1]
    {
        scalar_product = 1;
    }
    else if (scalar_product < -1.0)
    {
        scalar_product = -1;
    }
    return scalar_product;
}

void SlideCreator::calcAngularDistancesWithSectors()
{
    // вычисляем направляющие косинусы точки проектирования
    double l_oz = cos(slideData.pointBeta * transToRad) * cos(slideData.pointAlpha * transToRad);
    double m_oz = cos(slideData.pointBeta * transToRad) * sin(slideData.pointAlpha * transToRad);
    double n_oz = sin(slideData.pointBeta * transToRad);

    // Для входа в сектор

    QVector <double> l_oz_sec;
    l_oz_sec.reserve(catalogData.alphaVecSec().size());
    QVector <double> m_oz_sec;
    m_oz_sec.reserve(catalogData.alphaVecSec().size());
    QVector <double> n_oz_sec;
    n_oz_sec.reserve(catalogData.alphaVecSec().size());

    for (int i = 0;i < catalogData.alphaVecSec().size();i ++)// направляющие косинусы центров секторов
    {
        double cos_d = cos(catalogData.betaVecSec()[i] * transToRad);
        double cos_a = cos(catalogData.alphaVecSec()[i] * transToRad);
        double sin_d = sin(catalogData.betaVecSec()[i] * transToRad);
        double sin_a = sin(catalogData.alphaVecSec()[i] * transToRad);
        l_oz_sec.append(cos_d * cos_a);
        m_oz_sec.append(cos_d * sin_a);
        n_oz_sec.append(sin_d);
    }

    // считаем косинусы между центром проектирования и центрами секторов, находим ближайший
    QVector <double> angleCosSec;
    angleCosSec.reserve(l_oz_sec.size());

    for (int i = 0;i < l_oz_sec.size();i ++)
    {
        auto scalar_product = calcScalarProduct(l_oz, l_oz_sec[i], m_oz , m_oz_sec[i] , n_oz, n_oz_sec[i]);
        angleCosSec.append(scalar_product);
    }

    QVector<double>::iterator it_sec;
    it_sec = std::max_element(angleCosSec.begin(), angleCosSec.end());// вычисляем максимальной близкий к точке проецирования сектор
    int pos = it_sec-angleCosSec.begin();

    QVector <int> starsInSector;
    for (int i = catalogData.shiftVec()[pos];i < (catalogData.shiftVec()[pos] + catalogData.countVecSec()[pos]); i++)
    {
        starsInSector.append(catalogData.newNumn()[i]);// формируем вектор, содержащий номера звезд в секторе
    }

    const auto countOfStars = starsInSector.size();
    angleData.resultBeta.reserve(countOfStars);
    angleData.resultAlpha.reserve(countOfStars);

    for (int i = 0;i < countOfStars; i ++)
    {
        angleData.resultAlpha.append(catalogData.alphaVec()[starsInSector[i]]);
        angleData.resultBeta.append(catalogData.betaVec()[starsInSector[i]]);
    }


    QVector<double> l_st;
    l_st.reserve(countOfStars);
    QVector<double> m_st;
    m_st.reserve(countOfStars);
    QVector<double> n_st;
    n_st.reserve(countOfStars);

    for (int i = 0; i < countOfStars; i++)//направляющие косинусы звезд в секторе
    {
        double cos_d = cos(angleData.resultBeta[i] * transToRad);
        double cos_a = cos(angleData.resultAlpha[i] * transToRad);
        double sin_d = sin(angleData.resultBeta[i] * transToRad);
        double sin_a = sin(angleData.resultAlpha[i] * transToRad);
        l_st.append(cos_d * cos_a);
        m_st.append(cos_d * sin_a);
        n_st.append(sin_d);
    }

    angleData.angleCos.reserve(countOfStars);

    for (int i = 0;i < countOfStars;i ++)// косинусы между точкой проектирования и звездами в секторе
    {
        auto scalar_product = calcScalarProduct(l_oz, l_st[i], m_oz, m_st[i], n_oz, n_st[i]);
        angleData.angleCos.append(scalar_product);
    }

}
void SlideCreator::calcAngularDistancesNoSectors()
{
    // вычисляем направляющие косинусы точки проектирования
    double l_oz = cos(slideData.pointBeta * transToRad) * cos(slideData.pointAlpha * transToRad);
    double m_oz = cos(slideData.pointBeta * transToRad) * sin(slideData.pointAlpha * transToRad);
    double n_oz = sin(slideData.pointBeta * transToRad);

    const auto countOfStars = catalogData.alphaVec().size();
    QVector <double> l_st;
    l_st.reserve(countOfStars);
    QVector <double> m_st;
    m_st.reserve(countOfStars);
    QVector <double> n_st;
    n_st.reserve(countOfStars);

    for (int i = 0;i < countOfStars;i ++)
    {
        // вычисляем направляющие косинусы всех звезд
        double cosD = cos(catalogData.betaVec()[i] * transToRad);
        double cos_a = cos(catalogData.alphaVec()[i] * transToRad);
        double sinD = sin(catalogData.betaVec()[i] * transToRad);
        double sin_a = sin(catalogData.alphaVec()[i]  * transToRad);
        l_st.append(cosD * cos_a);
        m_st.append(cosD * sin_a);
        n_st.append(sinD);
    }


    angleData.angleCos.reserve(countOfStars);
    for (int i = 0;i < countOfStars;i ++)
    {
        // вычисляем косинус угла между точкой проецирования и координатами звезд
        auto scalar_product = calcScalarProduct(l_oz, l_st[i], m_oz, m_st[i], n_oz, n_st[i]);
        angleData.angleCos.append(scalar_product);

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
void SlideCreator::calculateAngularDistOptions(const StarSlideData& _slide_data,const Catalog& _cat_data, bool check_sector)
{

    starSlideDataPrepare = false;
    slideData = _slide_data;
    catalogData = _cat_data;
    if (check_sector) calcAngularDistancesWithSectors();
    else calcAngularDistancesNoSectors();
    auto trMat = calcTransitionMatrix(slideData.pointAlpha,slideData.pointBeta,slideData.pointAzimut);
    angleData.trMat = trMat;
    starSlideDataPrepare = true;

}


/*возвращает вспомогательные слайды в виде сетки*/
QVector <StarParameters> SlideCreator::createGridSlide(const GridSlideData& grid_d, bool check_distorsio, const DistorsioData& distData)
{

    slideType = SLIDE_TYPE::INVALID_TYPE;
    QScopedArrayPointer <unsigned char> dataImage(new unsigned char [grid_d.slideSizeX * grid_d.slideSizeY]);
    QVector <StarParameters> coordsOfStars;
    if (check_distorsio)
    {
        for (int y = grid_d.gridDistance;y < (grid_d.slideSizeY - grid_d.gridDistance);y += (grid_d.gridDistance + grid_d.pixelPerStar))
        {
            for (int x = grid_d.gridDistance;x < (grid_d.slideSizeX - grid_d.gridDistance);x += (grid_d.gridDistance + grid_d.pixelPerStar))
            {

                double y_mm = (y - grid_d.slideSizeY / 2) * grid_d.pix;
                double x_mm = (x - grid_d.slideSizeX / 2) * grid_d.pix;
                y_mm = calculateDistorsio(y_mm, x_mm, y_mm,distData.yDistorsioVector);
                x_mm = calculateDistorsio(x_mm, x_mm, y_mm,distData.xDistorsioVector);

                int y_pix = ((y_mm/grid_d.pix) + 0.5) + grid_d.slideSizeY / 2;
                int x_pix = ((x_mm/grid_d.pix) + 0.5) + grid_d.slideSizeX / 2;

                StarParameters starParameters;
                starParameters.x = getStarPos(grid_d.pixelPerStar,x_pix);
                starParameters.y = getStarPos(grid_d.pixelPerStar,y_pix);
                starParameters.sizeX = getStarSize(grid_d.pixelPerStar,x_pix);
                starParameters.sizeY = getStarSize(grid_d.pixelPerStar,y_pix);
                coordsOfStars.append(starParameters);

                int start_pos = x_pix + y_pix * grid_d.slideSizeX;

                for (int i = 0;i < grid_d.pixelPerStar;i ++)/*i-Y, j-X*/
                {
                    for (int j = 0;j < grid_d.pixelPerStar;j ++)
                    {
                        if (outOfImage(start_pos,grid_d.slideSizeX, grid_d.slideSizeY, x_pix, j , i, grid_d.pixelPerStar))
                        {
                            continue;
                        }
                        dataImage[start_pos + j + i * grid_d.slideSizeX] = 255;
                    }

                }
            }
        }

    }

    else
    {
        for (int y = grid_d.gridDistance;y < (grid_d.slideSizeY-grid_d.gridDistance);y += (grid_d.gridDistance+grid_d.pixelPerStar))
        {
            for (int x = grid_d.gridDistance;x < (grid_d.slideSizeX-grid_d.gridDistance);x += (grid_d.gridDistance+grid_d.pixelPerStar))
            {
                StarParameters starParameters;
                starParameters.x = getStarPos(grid_d.pixelPerStar,x);
                starParameters.y = getStarPos(grid_d.pixelPerStar,y);
                starParameters.sizeX = getStarSize(grid_d.pixelPerStar,x);
                starParameters.sizeY = getStarSize(grid_d.pixelPerStar,y);
                coordsOfStars.append(starParameters);
                int start_pos = x + y * grid_d.slideSizeX;
                for (int i = 0;i< grid_d.pixelPerStar;i ++)
                {
                    for (int j = 0;j < grid_d.pixelPerStar;j ++)
                    {
                        if (outOfImage(start_pos,grid_d.slideSizeX,grid_d.slideSizeY,x,i,j,grid_d.pixelPerStar))
                        {
                            continue;
                        }
                        dataImage[start_pos+j+i*grid_d.slideSizeX] = 255;
                    }

                }
            }
        }
    }

    QImage img(dataImage.data(), grid_d.slideSizeX, grid_d.slideSizeY, grid_d.slideSizeX, QImage::Format_Grayscale8);
    img.setColorTable(colorTable);
    optimalImage.reset(new QImage(img.convertToFormat(QImage::Format_RGB32)));
    slideType = SLIDE_TYPE::GRID_TYPE;
    return coordsOfStars;
}


/*Перед выполнением этой функции должны быть рассчитаны угловые расстояния между точкой проецирования и звездами, т.е выполнена функция calculateAngularDistOptions  */
SlideParameters SlideCreator::createStarSlide(float focus, bool check_sector, bool check_distorsio, const DistorsioData& distData = DistorsioData {})
{
    slideType = SLIDE_TYPE::INVALID_TYPE;
    if (!starSlideDataPrepare) return SlideParameters {};

    double viewAngleX, viewAngleY, viewAngle;
    calcViewAngle(viewAngleX, viewAngleY, viewAngle);


    QVector <double> resultAlpha;
    QVector <double> resultBeta;

    for (int i = 0;i < angleData.angleCos.size(); i++)
    {
        if (check_sector)// если учитываем сектора
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

    for (int i = 0;i < resultBeta.size();i ++)
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

    double x_coord_mm, y_coord_mm;
    int xCoord, yCoord;
    float CC;
    int countOfStars = 0;//для подсчёта числа звезд, попавших на слайд
    for (int i = 0;i < filteredLSt.size();i ++)
    {

        CC = angleData.trMat[2][0] * filteredLSt[i]
                +
                angleData.trMat[2][1] * filteredMSt[i]
                +
                angleData.trMat[2][2] * filteredNSt[i];

        if (check_distorsio)
        {

            x_coord_mm = (-focus * (angleData.trMat[0][0] * filteredLSt[i]
                    + angleData.trMat[0][1] * filteredMSt[i]
                    + angleData.trMat[0][2] * filteredNSt[i]) / CC);
            y_coord_mm = (-focus * (angleData.trMat[1][0] * filteredLSt[i]
                    + angleData.trMat[1][1] * filteredMSt[i]
                    + angleData.trMat[1][2] * filteredNSt[i]) / CC);
            x_coord_mm = calculateDistorsio (x_coord_mm, x_coord_mm, y_coord_mm, distData.xDistorsioVector);
            y_coord_mm = calculateDistorsio (y_coord_mm, x_coord_mm, y_coord_mm, distData.yDistorsioVector);

            xCoord = x_coord_mm / slideData.pix + 0.5;
            yCoord = y_coord_mm / slideData.pix + 0.5;

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

        xCoord += slideData.slideSizeX / 2; yCoord += slideData.slideSizeY / 2;// x_coord,slideSizeX- ось X, y_coord,slideSizeY - ось Y

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
    if (!starSlideDataPrepare) return TestSlideParameters {};
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
            double inside_fieldOfViewCos = cos(slideData.insideViewAngle/2 * transToRad);
            if (angleData.angleCos[i] >= inside_fieldOfViewCos
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

    for (int i = 0; i < resultBeta.size(); i ++)
    {
        double cos_d = cos(resultBeta[i] * transToRad);
        double cos_a = cos(resultAlpha[i] * transToRad);
        double sin_d = sin(resultBeta[i] * transToRad);
        double sin_a = sin(resultAlpha[i] * transToRad);

        filteredLSt.append(cos_d * cos_a);
        filteredMSt.append(cos_d * sin_a);
        filteredNSt.append(sin_d);
    }

    double x_coord_mm, y_coord_mm;
    int xCoord, yCoord;
    float CC;

    for (int i = 0;i < filteredLSt.size();i ++)
    {
        CC = angleData.trMat[2][0] * filteredLSt[i]
                +
                angleData.trMat[2][1] * filteredMSt[i]
                +
                angleData.trMat[2][2] * filteredNSt[i];

        if (checkDistorsio)
        {

            x_coord_mm = (-slideData.focStart * (angleData.trMat[0][0] * filteredLSt[i]
                    + angleData.trMat[0][1] * filteredMSt[i]
                    + angleData.trMat[0][2] * filteredNSt[i]) / CC);
            y_coord_mm = (-slideData.focStart * (angleData.trMat[1][0] * filteredLSt[i]
                    + angleData.trMat[1][1] * filteredMSt[i]
                    + angleData.trMat[1][2] * filteredNSt[i]) / CC);
            x_coord_mm = calculateDistorsio (x_coord_mm, x_coord_mm, y_coord_mm, distData.xDistorsioVector);
            y_coord_mm = calculateDistorsio (y_coord_mm, x_coord_mm, y_coord_mm, distData.yDistorsioVector);
            xCoord = x_coord_mm / slideData.pix + 0.5;
            yCoord = y_coord_mm / slideData.pix + 0.5;
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

        xCoord += slideData.slideSizeX / 2; yCoord += slideData.slideSizeY / 2;// x_coord,slideSizeX- ось X, y_coord,slideSizeY - ось Y

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
    svg.setAttribute("xmlns","http://www.w3.org/2000/svg");
    svg.setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink");
    svg.setAttribute("encoding","UTF-8");
    svg.setAttribute("width",QString::number(sizeX));
    svg.setAttribute("height",QString::number(sizeY));
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
            cube.setAttribute("fill","black");
            svg.appendChild(cube);
            for (auto& starCoordinate : coordsOfStars[slidePos])
            {
                QDomElement star = doc.createElement("rect");
                star.setAttribute("fill","white");
                star.setAttribute("x",QString::number(starCoordinate.x + xOffset));
                star.setAttribute("y",QString::number(starCoordinate.y + yOffset));
                star.setAttribute("width",QString::number(starCoordinate.sizeX));
                star.setAttribute("height",QString::number(starCoordinate.sizeY));
                svg.appendChild(star);

            }
            QDomElement textElement = doc.createElement("text");
            QDomText text = doc.createTextNode(setableText[slidePos]);
            textElement.setAttribute("x",QString::number(iParams.fontX + xOffset));
            textElement.setAttribute("y",QString::number(iParams.fontY + yOffset));
            textElement.setAttribute("style","fill: white;");
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

    svg.setAttribute("viewBox",QString("0 ")+"0 "+QString::number(sizeX) + " " + QString::number(sizeY));
    svg.setAttribute("xmlns","http://www.w3.org/2000/svg");
    svg.setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink");
    svg.setAttribute("encoding","UTF-8");
    svg.setAttribute("width",QString::number(sizeX));
    svg.setAttribute("height",QString::number(sizeY));
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
            cube.setAttribute("y","0");
            cube.setAttribute("width", QString::number(imageWidth));
            cube.setAttribute("height", QString::number(imageHeight));
            cube.setAttribute("fill","black");
            symbol.appendChild(cube);
            for (auto& starCoordinate : coordsOfStars[slidePos])
            {
                QDomElement star = doc.createElement("rect");
                star.setAttribute("fill","white");
                star.setAttribute("x",QString::number(starCoordinate.x));
                star.setAttribute("y",QString::number(starCoordinate.y));
                star.setAttribute("width",QString::number(starCoordinate.sizeX));
                star.setAttribute("height",QString::number(starCoordinate.sizeY));
                symbol.appendChild(star);
            }
            QDomElement textElement = doc.createElement("text");
            QDomText text = doc.createTextNode(setableText[slidePos]);
            textElement.setAttribute("x",QString::number(iParams.fontX));
            textElement.setAttribute("y",QString::number(iParams.fontY));
            textElement.setAttribute("style","fill: white;");
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
    svg.setAttribute("xmlns","http://www.w3.org/2000/svg");
    svg.setAttribute("encoding","UTF-8");
    svg.setAttribute("width",QString::number(imageWidth));
    svg.setAttribute("height",QString::number(imageHeight));

    QDomElement cube = doc.createElement("rect");
    cube.setAttribute("x","0");
    cube.setAttribute("y","0");
    cube.setAttribute("width",QString::number(imageWidth));
    cube.setAttribute("height",QString::number(imageHeight));
    cube.setAttribute("fill","black");
    doc.appendChild(svg);
    svg.appendChild(cube);
    for (auto& starCoordinate : coordsOfStars)
    {
        QDomElement star = doc.createElement("rect");
        star.setAttribute("x",QString::number(starCoordinate.x));
        star.setAttribute("fill","white");
        star.setAttribute("y",QString::number(starCoordinate.y));
        star.setAttribute("width",QString::number(starCoordinate.sizeX));
        star.setAttribute("height",QString::number(starCoordinate.sizeY));
        svg.appendChild(star);
    }
    QDomElement textElement = doc.createElement("text");
    QDomText text = doc.createTextNode(setableText);
    textElement.setAttribute("x",QString::number(iParams.fontX));
    textElement.setAttribute("y",QString::number(iParams.fontY));
    textElement.setAttribute("style","fill: white;");
    textElement.setAttribute("font-size",QString::number(iParams.fontSize - 30));
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
