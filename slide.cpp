#include "slide.h"
#include <qDebug>

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
double& SlideCreator::calc_dist(double& coord_a, const double& coord_b, const QList <double>& distorsio_coef)
{
    coord_a = coord_a + distorsio_coef[0] + distorsio_coef[1]*coord_a + distorsio_coef[2] * coord_b
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
    return coord_a;
}
/*Рассчитывает переходную матрицу для пересчета угловых координат звезд на координаты слайда*/
QVector< QVector<float> > SlideCreator::calcTransitionMatrix(double pointAlpha, double pointBeta,double pointAzimut)
{

    QVector< QVector<float> > trMat(3);
    float PS,PC,QS,QC,RS,RC;
    for (int i = 0;i < trMat.size();i ++) trMat[i].resize(3);

    PS = sin(pointAzimut * trans_to_rad); PC= cos(pointAzimut * trans_to_rad);
    QS = sin(pointBeta * trans_to_rad); QC = cos(pointBeta * trans_to_rad);
    RS = sin(pointAlpha * trans_to_rad); RC = cos(pointAlpha * trans_to_rad);
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
    double l_oz = cos(slideData.pointBeta * trans_to_rad) * cos(slideData.pointAlpha * trans_to_rad);
    double m_oz = cos(slideData.pointBeta * trans_to_rad) * sin(slideData.pointAlpha * trans_to_rad);
    double n_oz = sin(slideData.pointBeta * trans_to_rad);

    // Для входа в сектор

    QVector <double> l_oz_sec;
    l_oz_sec.reserve(catalogData.alphaVecSec().size());
    QVector <double> m_oz_sec;
    m_oz_sec.reserve(catalogData.alphaVecSec().size());
    QVector <double> n_oz_sec;
    n_oz_sec.reserve(catalogData.alphaVecSec().size());

    for (int i = 0;i < catalogData.alphaVecSec().size();i ++)// направляющие косинусы центров секторов
    {
        double cos_b = cos(catalogData.betaVecSec()[i] * trans_to_rad);
        double cos_a = cos(catalogData.alphaVecSec()[i] * trans_to_rad);
        double sin_b = sin(catalogData.betaVecSec()[i] * trans_to_rad);
        double sin_a = sin(catalogData.alphaVecSec()[i] * trans_to_rad);
        l_oz_sec.append(cos_b * cos_a);
        m_oz_sec.append(cos_b * sin_a);
        n_oz_sec.append(sin_b);
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
    angleData.result_beta.reserve(countOfStars);
    angleData.result_alpha.reserve(countOfStars);

    for (int i = 0;i < countOfStars; i ++)
    {
        angleData.result_alpha.append(catalogData.alphaVec()[starsInSector[i]]);
        angleData.result_beta.append(catalogData.betaVec()[starsInSector[i]]);
    }


    QVector<double> l_st;
    l_st.reserve(countOfStars);
    QVector<double> m_st;
    m_st.reserve(countOfStars);
    QVector<double> n_st;
    n_st.reserve(countOfStars);

    for (int i = 0;i < countOfStars;i ++)//направляющие косинусы звезд в секторе
    {
        double cos_b = cos(angleData.result_beta[i] * trans_to_rad);
        double cos_a = cos(angleData.result_alpha[i] * trans_to_rad);
        double sin_b = sin(angleData.result_beta[i] * trans_to_rad);
        double sin_a = sin(angleData.result_alpha[i] * trans_to_rad);
        l_st.append(cos_b * cos_a);
        m_st.append(cos_b * sin_a);
        n_st.append(sin_b);
    }

    angleData.angle_cos.reserve(countOfStars);

    for (int i = 0;i < countOfStars;i ++)// косинусы между точкой проектирования и звездами в секторе
    {
        auto scalar_product = calcScalarProduct(l_oz, l_st[i], m_oz, m_st[i], n_oz, n_st[i]);
        angleData.angle_cos.append(scalar_product);
    }

}
void SlideCreator::calcAngularDistancesNoSectors()
{
    // вычисляем направляющие косинусы точки проектирования
    double l_oz = cos(slideData.pointBeta * trans_to_rad) * cos(slideData.pointAlpha * trans_to_rad);
    double m_oz = cos(slideData.pointBeta * trans_to_rad) * sin(slideData.pointAlpha * trans_to_rad);
    double n_oz = sin(slideData.pointBeta * trans_to_rad);

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
        double cos_b = cos(catalogData.betaVec()[i] * trans_to_rad);
        double cos_a = cos(catalogData.alphaVec()[i] * trans_to_rad);
        double sin_b = sin(catalogData.betaVec()[i] * trans_to_rad);
        double sin_a = sin(catalogData.alphaVec()[i]  * trans_to_rad);
        l_st.append(cos_b * cos_a);
        m_st.append(cos_b * sin_a);
        n_st.append(sin_b);
    }


    angleData.angle_cos.reserve(countOfStars);
    for (int i = 0;i < countOfStars;i ++)
    {
        // вычисляем косинус угла между точкой проецирования и координатами звезд
        auto scalar_product = calcScalarProduct(l_oz, l_st[i], m_oz, m_st[i], n_oz, n_st[i]);
        angleData.angle_cos.append(scalar_product);

    }

    angleData.result_alpha = catalogData.alphaVec();
    angleData.result_beta = catalogData.betaVec();
}

void SlideCreator::calcViewAngle(double& view_angle_x, double& view_angle_y, double& view_angle)
{
    // считаем угол зрения в зависимости от фокусного расстояния и размерности матрицы
    view_angle_x = atan((((slideData.slideSizeX / 2) * slideData.pix)/(slideData.focStart))) * 2 * trans_to_grad;
    view_angle_y = atan((((slideData.slideSizeY / 2) * slideData.pix)/(slideData.focStart))) * 2 * trans_to_grad;
    // угол зрения - описанная окружность
    view_angle = sqrt(pow(view_angle_x, 2) + pow(view_angle_y, 2));
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
QVector<StarParameters> SlideCreator::createGridSlide(const GridSlideData& grid_d, bool check_distorsio, const DistorsioData& distData)
{

    slideType = SLIDE_TYPE::INVALID_TYPE;
    QScopedArrayPointer <unsigned char> dataImage(new unsigned char [grid_d.slideSizeX * grid_d.slideSizeY]);
    QVector <StarParameters> coordsOfStars;
    if (check_distorsio)
    {
        for (int y = grid_d.grid_distance;y < (grid_d.slideSizeY - grid_d.grid_distance);y += (grid_d.grid_distance + grid_d.pixelPerStar))
        {
            for (int x = grid_d.grid_distance;x < (grid_d.slideSizeX - grid_d.grid_distance);x += (grid_d.grid_distance + grid_d.pixelPerStar))
            {

                double y_mm = (y - grid_d.slideSizeY / 2) * grid_d.pix;
                double x_mm = (x - grid_d.slideSizeX / 2) * grid_d.pix;
                y_mm = calc_dist(y_mm,x_mm,distData.yDistorsioVector);
                x_mm = calc_dist(x_mm,y_mm,distData.xDistorsioVector);

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
        for (int y = grid_d.grid_distance;y < (grid_d.slideSizeY-grid_d.grid_distance);y += (grid_d.grid_distance+grid_d.pixelPerStar))
        {
            for (int x = grid_d.grid_distance;x < (grid_d.slideSizeX-grid_d.grid_distance);x += (grid_d.grid_distance+grid_d.pixelPerStar))
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
SlideParameters SlideCreator::createStarSlide(float focus, bool check_sector, bool check_distorsio, const DistorsioData& distData = DistorsioData())
{
    slideType = SLIDE_TYPE::INVALID_TYPE;
    if (!starSlideDataPrepare) return SlideParameters();

    double viewAngleX, viewAngleY, view_angle;
    calcViewAngle(viewAngleX, viewAngleY, view_angle);


    QVector <double> result_alpha;
    QVector <double> result_beta;

    for (int i = 0;i < angleData.angle_cos.size();i ++)
    {
        if (check_sector)// если учитываем сектора
        {
            double fieldOfViewCos = cos(view_angle * trans_to_rad);
            if ((angleData.angle_cos[i]) >= fieldOfViewCos
                    && catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                result_alpha.append(angleData.result_alpha[i]);
                result_beta.append(angleData.result_beta[i]);

            }
        }
        else // если не учитываем
        {
            double insideDiamOfViewCos = cos((slideData.insideViewAngle / 2) * trans_to_rad);
            if (angleData.angle_cos[i] >= insideDiamOfViewCos
                    && catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                result_alpha.append(angleData.result_alpha[i]);
                result_beta.append(angleData.result_beta[i]);
            }
        }
    }
    QVector <double> filtered_l_st;
    QVector <double> filtered_m_st;
    QVector <double> filtered_n_st;

    for (int i = 0;i < result_beta.size();i ++)
    {
        double cos_b = cos(result_beta[i] * trans_to_rad);
        double cos_a = cos(result_alpha[i] * trans_to_rad);
        double sin_b = sin(result_beta[i] * trans_to_rad);
        double sin_a = sin(result_alpha[i] * trans_to_rad);

        filtered_l_st.append(cos_b * cos_a);
        filtered_m_st.append(cos_b * sin_a);
        filtered_n_st.append(sin_b);
    }



    QScopedArrayPointer <unsigned char> dataImage(new unsigned char [slideData.slideSizeX * slideData.slideSizeY]);
    QVector <StarParameters> coordsOfStars;

    double x_coord_mm, y_coord_mm;
    int x_coord, y_coord;
    float CC;
    int countOfStars = 0;//для подсчёта числа звезд, попавших на слайд
    for (int i = 0;i < filtered_l_st.size();i ++)
    {

        CC = angleData.trMat[2][0] * filtered_l_st[i]
                +
                angleData.trMat[2][1] * filtered_m_st[i]
                +
                angleData.trMat[2][2] * filtered_n_st[i];

        if (check_distorsio)
        {

            x_coord_mm = (-focus * (angleData.trMat[0][0] * filtered_l_st[i]
                    + angleData.trMat[0][1] * filtered_m_st[i]
                    + angleData.trMat[0][2] * filtered_n_st[i]) / CC);
            y_coord_mm = (-focus * (angleData.trMat[1][0] * filtered_l_st[i]
                    + angleData.trMat[1][1] * filtered_m_st[i]
                    + angleData.trMat[1][2] * filtered_n_st[i]) / CC);
            x_coord_mm = calc_dist(x_coord_mm, y_coord_mm, distData.xDistorsioVector);
            y_coord_mm = calc_dist(y_coord_mm, x_coord_mm, distData.yDistorsioVector);
            x_coord = x_coord_mm / slideData.pix + 0.5;
            y_coord = y_coord_mm / slideData.pix + 0.5;

        }

        else
        {
            x_coord = (-focus * (angleData.trMat[0][0] * filtered_l_st[i]
                    + angleData.trMat[0][1] * filtered_m_st[i]
                    + angleData.trMat[0][2]*filtered_n_st[i]) / CC) / slideData.pix + 0.5;
            y_coord = (-focus * (angleData.trMat[1][0] * filtered_l_st[i]
                    + angleData.trMat[1][1] * filtered_m_st[i]
                    + angleData.trMat[1][2] * filtered_n_st[i]) / CC) / slideData.pix + 0.5;

        }

        x_coord += slideData.slideSizeX / 2; y_coord += slideData.slideSizeY / 2;// x_coord,slideSizeX- ось X, y_coord,slideSizeY - ось Y

        if (x_coord > 0 && x_coord < slideData.slideSizeX && y_coord > 0 && y_coord < slideData.slideSizeY)
        {
            StarParameters starParameters;
            starParameters.x = getStarPos(slideData.pixelPerStar, x_coord);
            starParameters.y = getStarPos(slideData.pixelPerStar, y_coord);
            starParameters.sizeX = getStarSize(slideData.pixelPerStar, x_coord);
            starParameters.sizeY = getStarSize(slideData.pixelPerStar, y_coord);
            coordsOfStars.append(starParameters);


            int pos_pix = y_coord * slideData.slideSizeX + x_coord;
            int start_pos = pos_pix - (slideData.pixelPerStar/2) * slideData.slideSizeX - (slideData.pixelPerStar/2);

            for (int y = 0;y < slideData.pixelPerStar;y ++)
            {
                for (int x = 0;x < slideData.pixelPerStar;x ++)
                {

                    if (outOfImage(start_pos, slideData.slideSizeX, slideData.slideSizeY, x_coord, x, y, slideData.pixelPerStar))
                    {
                        continue;
                    }
                    dataImage[start_pos + x + y * slideData.slideSizeX] = 255;
                }

            }
            ++ countOfStars;// подсчёт числа звезд в кадре
        }

    }

    QImage img(dataImage.data(), slideData.slideSizeX, slideData.slideSizeY, slideData.slideSizeX, QImage::Format_Grayscale8);
    img.setColorTable(colorTable);
    optimalImage.reset(new QImage(img.convertToFormat(QImage::Format_Mono)));

    SlideParameters imageData;
    imageData.count_of_stars = countOfStars;
    imageData.view_angle_x = viewAngleX;
    imageData.view_angle_y = viewAngleY;
    imageData.coordsOfStars = coordsOfStars;

    slideType = SLIDE_TYPE::STAR_TYPE;
    return imageData;

}
/* Выдает предварительную информацию о размерах слайда, числе звезд на слайде.*/
/* Если слайдов несколько выдает так же информацию об размере общего изображения*/
TestSlideParameters SlideCreator::testStarSlide( bool check_sector, bool check_distorsio, const DistorsioData &distData)
{
    if (!starSlideDataPrepare) return TestSlideParameters();
    int count_of_stars = 0;//для подсчёта числа звезд, попавших на слайд

    // считаем угол зрения в зависимости от фокусного расстояния и размерности матрицы
    double view_angle_x = atan((((slideData.slideSizeX / 2) * slideData.pix)/(slideData.focStart))) * 2 * trans_to_grad;
    double view_angle_y = atan((((slideData.slideSizeY / 2) * slideData.pix)/(slideData.focStart))) * 2 * trans_to_grad;
    double view_angle = sqrt(pow(view_angle_x, 2) + pow(view_angle_y, 2));// угол зрения- описанная окружность

    QVector <double> result_alpha;
    QVector <double> result_beta;


    for (int i = 0;i < angleData.angle_cos.size();i ++)
    {
        if (check_sector)
        {
            double fieldOfViewCos = cos(view_angle * trans_to_rad);
            if (angleData.angle_cos[i] >= fieldOfViewCos
                    && catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                result_alpha.append(angleData.result_alpha[i]);
                result_beta.append(angleData.result_beta[i]);

            }
        }
        else
        {
            double inside_fieldOfViewCos = cos((slideData.insideViewAngle/2) * trans_to_rad);
            if (angleData.angle_cos[i] >= inside_fieldOfViewCos
                    &&  catalogData.mvVec()[i] > slideData.minMv
                    && catalogData.mvVec()[i] < slideData.maxMv)
            {
                result_alpha.append(angleData.result_alpha[i]);
                result_beta.append(angleData.result_beta[i]);
            }
        }
    }


    QVector <double> filtered_l_st;
    QVector <double> filtered_m_st;
    QVector <double> filtered_n_st;

    for (int i = 0;i < result_beta.size();i ++)
    {
        double cos_b = cos(result_beta[i] * trans_to_rad);
        double cos_a = cos(result_alpha[i] * trans_to_rad);
        double sin_b = sin(result_beta[i] * trans_to_rad);
        double sin_a = sin(result_alpha[i] * trans_to_rad);

        filtered_l_st.append(cos_b * cos_a);
        filtered_m_st.append(cos_b * sin_a);
        filtered_n_st.append(sin_b);
    }

    double x_coord_mm, y_coord_mm;
    int x_coord, y_coord;
    float CC;

    for (int i = 0;i < filtered_l_st.size();i ++)
    {
        CC = angleData.trMat[2][0] * filtered_l_st[i]
                +
                angleData.trMat[2][1] * filtered_m_st[i]
                +
                angleData.trMat[2][2] * filtered_n_st[i];

        if (check_distorsio)
        {

            x_coord_mm = (-slideData.focStart * (angleData.trMat[0][0] * filtered_l_st[i]
                    + angleData.trMat[0][1] * filtered_m_st[i]
                    + angleData.trMat[0][2] * filtered_n_st[i]) / CC);
            y_coord_mm = (-slideData.focStart * (angleData.trMat[1][0] * filtered_l_st[i]
                    + angleData.trMat[1][1] * filtered_m_st[i]
                    + angleData.trMat[1][2] * filtered_n_st[i]) / CC);
            x_coord_mm = calc_dist(x_coord_mm, y_coord_mm, distData.xDistorsioVector);
            y_coord_mm = calc_dist(y_coord_mm, x_coord_mm, distData.yDistorsioVector);
            x_coord = x_coord_mm / slideData.pix + 0.5;
            y_coord = y_coord_mm / slideData.pix + 0.5;
        }

        else
        {
            x_coord = (-slideData.focStart * (angleData.trMat[0][0] * filtered_l_st[i]
                    + angleData.trMat[0][1] * filtered_m_st[i]
                    + angleData.trMat[0][2] * filtered_n_st[i]) / CC) / slideData.pix + 0.5;
            y_coord = (-slideData.focStart * (angleData.trMat[1][0] * filtered_l_st[i]
                    + angleData.trMat[1][1] * filtered_m_st[i]
                    + angleData.trMat[1][2] * filtered_n_st[i]) / CC) / slideData.pix + 0.5;

        }

        x_coord += slideData.slideSizeX / 2; y_coord += slideData.slideSizeY / 2;// x_coord,slideSizeX- ось X, y_coord,slideSizeY - ось Y

        if ( x_coord > 0
                && x_coord < slideData.slideSizeX
                && y_coord > 0
                && y_coord < slideData.slideSizeY)
        {
            ++ count_of_stars; // подсчёт числа звезд
        }

    }

    TestSlideParameters testData;
    testData.view_angle_x = view_angle_x;
    testData.view_angle_y = view_angle_y;
    testData.count_of_stars = count_of_stars;
    return testData;

}

QSharedPointer <QImage> SlideCreator::getSlidePointer() noexcept
{
    return optimalImage;
}
