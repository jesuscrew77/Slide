#include "slide.h"
#include <qDebug>


SlideCreator::SlideCreator()
    :angle_data(AngularDistanceOptions()),
      slideData(StarSlideData()),
      catalogData(CatalogData()),
      slide_type(INVALID_TYPE)
{

}

/*Расчет скорректированной по полиному дисторсии координаты пикселя*/
double& SlideCreator::calc_dist(double &coord_a, const double &coord_b, const QList<double> distorsio_coef)
{
    coord_a= coord_a+distorsio_coef[0]+distorsio_coef[1]*coord_a+distorsio_coef[2]*coord_b
            +distorsio_coef[3]*pow( coord_a,2)+distorsio_coef[4]* coord_a*coord_b
            +distorsio_coef[5]*pow(coord_b,2)+ distorsio_coef[6]*pow( coord_a,3)
            +distorsio_coef[7]*pow( coord_a,2)*coord_b+distorsio_coef[8]* coord_a*pow(coord_b,2)
            +distorsio_coef[9]*pow(coord_b,3)+distorsio_coef[10]*pow(coord_a,4)
            +distorsio_coef[11]*pow(coord_a,3)*coord_b+distorsio_coef[12]*pow(coord_b,2)*pow(coord_a,2)
            +distorsio_coef[13]*coord_a*pow(coord_b,3)+distorsio_coef[14]*pow(coord_b,4)
            +distorsio_coef[15]*pow(coord_a,5)+distorsio_coef[16]*pow(coord_a,4)*coord_b
            +distorsio_coef[17]*pow(coord_a,3)*pow(coord_b,2)
            +distorsio_coef[18]*pow(coord_a,2)*pow(coord_b,3)
            +distorsio_coef[19]*coord_a*pow(coord_b,4)
            +distorsio_coef[20]*pow(coord_b,5);
    return coord_a;
}
/*Рассчитывает переходную матрицу для пересчета угловых координат звезд на координаты слайда*/
QVector< QVector<float> > SlideCreator::calc_transition_matrix(const double &pointAlpha,const double &pointBeta,const double &pointAzimut)
{
    constexpr  static double trans_to_rad=0.0174532925;
    QVector< QVector<float> > trMat(3);
    float PS,PC,QS,QC,RS,RC;
    for(int i=0;i<trMat.size();i++) trMat[i].resize(3);

    PS=sin(pointAzimut*(trans_to_rad)); PC=cos(pointAzimut*(trans_to_rad));
    QS=sin(pointBeta*(trans_to_rad)); QC=cos(pointBeta*(trans_to_rad));
    RS=sin(pointAlpha*(trans_to_rad)); RC=cos(pointAlpha*(trans_to_rad));
    trMat[0][0]=-PC*RS-PS*RC*QS;
    trMat[0][1]=PC*RC-PS*RS*QS;
    trMat[0][2]=PS*QC;
    trMat[1][0]=PS*RS-PC*RC*QS;
    trMat[1][1]=-PS*RC-PC*RS*QS;
    trMat[1][2]=PC*QC;
    trMat[2][0]=QC*RC;
    trMat[2][1]=QC*RS;
    trMat[2][2]=QS;
    return trMat;

}

int SlideCreator::getStarSize(const int& default_size, const int& star_pos)
{
    return star_pos-(default_size/2)<0?default_size-abs(star_pos-(default_size/2)):default_size;
}

int SlideCreator::getStarPos(const int& default_size, const int& star_pos)
{
    return star_pos-(default_size/2)<0?0:star_pos-(default_size/2);
}

/*Проверяет, выходит ли текущий пиксель, составляющий звезду за границу изображение*/
/*start_pos-одномерная координата левого верхнего пикселя звезды*/
/*x и y - координаты текущего пикселя*/
/*x_size и y_size - размер изображения*/
/*central_x_c - x-координата центра звезды*/
bool SlideCreator::outOfImage(const int&start_pos,const int& x_size,const int& y_size,const int&central_x_c,const int&x, const int&y, const int& pixelPerStar)
{
    const int one_dimensional_pix_pos=start_pos+x+y*x_size;
    const int max_one_dimensional_pix_pos=x_size*y_size;
    const int x_coord_pix_position=central_x_c-(pixelPerStar/2)+x;

    return (one_dimensional_pix_pos<=0 ||
            one_dimensional_pix_pos>=max_one_dimensional_pix_pos ||
            x_coord_pix_position<0||
            x_coord_pix_position>=x_size);
}


void SlideCreator::calcAngularDistancesWithSectors()
{
    // вычисляем направляющие косинусы точки проектирования
    double l_oz=cos(slideData.pointBeta*(trans_to_rad))*cos(slideData.pointAlpha*(trans_to_rad));
    double m_oz=cos(slideData.pointBeta*(trans_to_rad))*sin(slideData.pointAlpha*(trans_to_rad));
    double n_oz=sin(slideData.pointBeta*(trans_to_rad));

    // Для входа в сектор

    QVector <double> l_oz_sec;
    l_oz_sec.reserve(catalogData.alpha_vec_sec.size());
    QVector <double> m_oz_sec;
    m_oz_sec.reserve(catalogData.alpha_vec_sec.size());
    QVector <double> n_oz_sec;
    n_oz_sec.reserve(catalogData.alpha_vec_sec.size());

    for(int i=0;i<catalogData.alpha_vec_sec.size();i++)// направляющие косинусы центров секторов
    {
        double cos_b=cos(catalogData.beta_vec_sec[i]*(trans_to_rad));
        double cos_a=cos(catalogData.alpha_vec_sec[i]*(trans_to_rad));
        double sin_b=sin(catalogData.beta_vec_sec[i]*(trans_to_rad));
        double sin_a=sin(catalogData.alpha_vec_sec[i]*(trans_to_rad));
        l_oz_sec.append(cos_b*cos_a);
        m_oz_sec.append(cos_b*sin_a);
        n_oz_sec.append(sin_b);
    }

    // считаем косинусы между центром проектирования и центрами секторов, находим ближайший
    QVector<double> angle_cos_sec;
    angle_cos_sec.reserve(l_oz_sec.size());

    for(int i=0;i<l_oz_sec.size();i++)
    {
        double scalar_product=l_oz*l_oz_sec[i]+m_oz*m_oz_sec[i]+n_oz*n_oz_sec[i];

        if(scalar_product>1.0)// проверка на выход значения косинуса за диапазон [-1,1]
        {
            scalar_product=1;
        }
        else if(scalar_product<-1.0)
        {
            scalar_product=-1;
        }
        angle_cos_sec.append(scalar_product);
    }

    QVector<double>::iterator it_sec;
    it_sec=std::max_element(angle_cos_sec.begin(),angle_cos_sec.end());// вычисляем максимальной близкий к точке проецирования сектор
    int pos=it_sec-angle_cos_sec.begin();

    QVector<int> stars_in_sector;
    for(int i=catalogData.shift_vec[pos];i<(catalogData.shift_vec[pos]+catalogData.count_sec_vec[pos]);i++)
    {
        stars_in_sector.append(catalogData.new_numn[i]);// формируем вектор, содержащий номера звезд в секторе
    }

    QVector<double> result_sec_beta;
    result_sec_beta.reserve(stars_in_sector.size());
    QVector<double> result_sec_alpha;
    result_sec_alpha.reserve(stars_in_sector.size());

    for(int i=0;i<stars_in_sector.size();i++)
    {
        result_sec_alpha.append(catalogData.alpha_vec[stars_in_sector[i]]);
        result_sec_beta.append(catalogData.beta_vec[stars_in_sector[i]]);
    }


    QVector<double> l_st;
    l_st.reserve(result_sec_alpha.size());
    QVector<double> m_st;
    m_st.reserve(result_sec_alpha.size());
    QVector<double> n_st;
    n_st.reserve(result_sec_alpha.size());

    for(int i=0;i<result_sec_alpha.size();i++)//направляющие косинусы звезд в секторе
    {
        double cos_b=cos(result_sec_beta[i]*(trans_to_rad));
        double cos_a=cos(result_sec_alpha[i]*(trans_to_rad));
        double sin_b=sin(result_sec_beta[i]*(trans_to_rad));
        double sin_a=sin(result_sec_alpha[i]*(trans_to_rad));
        l_st.append(cos_b*cos_a);
        m_st.append(cos_b*sin_a);
        n_st.append(sin_b);
    }

    QVector<double> angle_cos;
    angle_cos.reserve(result_sec_alpha.size());

    for(int i=0;i<result_sec_alpha.size();i++)// косинусы между точкой проектирования и звездами в секторе
    {
        double scalar_product=l_oz*l_st[i]+m_oz*m_st[i]+n_oz*n_st[i];
        if(scalar_product>1.0)
        {
            scalar_product=1;
        }
        else if(scalar_product<-1.0)
        {
            scalar_product=-1;
        }
        angle_cos.append(scalar_product);
    }

    angle_data.angle_cos=angle_cos;
    angle_data.result_sec_alpha=result_sec_alpha;
    angle_data.result_sec_beta=result_sec_beta;

}
void SlideCreator::calcAngularDistancesNoSectors()
{

    // вычисляем направляющие косинусы точки проектирования
    double l_oz=cos(slideData.pointBeta*(trans_to_rad))*cos(slideData.pointAlpha*(trans_to_rad));
    double m_oz=cos(slideData.pointBeta*(trans_to_rad))*sin(slideData.pointAlpha*(trans_to_rad));
    double n_oz=sin(slideData.pointBeta*(trans_to_rad));



    QVector<double> l_st;
    l_st.reserve(catalogData.alpha_vec.size());
    QVector<double> m_st;
    m_st.reserve(catalogData.alpha_vec.size());
    QVector<double> n_st;
    n_st.reserve(catalogData.alpha_vec.size());

    for(int i=0;i<catalogData.alpha_vec.size();i++)
    {
        // вычисляем направляющие косинусы всех звезд
        double cos_b=cos(catalogData.beta_vec[i]*(trans_to_rad));
        double cos_a=cos(catalogData.alpha_vec[i]*(trans_to_rad));
        double sin_b=sin(catalogData.beta_vec[i]*(trans_to_rad));
        double sin_a=sin(catalogData.alpha_vec[i]*(trans_to_rad));
        l_st.append(cos_b*cos_a);
        m_st.append(cos_b*sin_a);
        n_st.append(sin_b);
    }

    QVector<double> angle_cos;
    angle_cos.reserve(catalogData.alpha_vec.size());
    for(int i=0;i<catalogData.alpha_vec.size();i++)
    {
        // вычисляем косинус угла между точкой проецирования и координатами звезд
        double scalar_product=l_oz*l_st[i]+m_oz*m_st[i]+n_oz*n_st[i];
        if(scalar_product>1.0)
        {
            scalar_product=1;
        }
        else if(scalar_product<-1.0)
        {
            scalar_product=-1;
        }
        angle_cos.append(scalar_product);

    }

    angle_data.angle_cos=angle_cos;
    angle_data.result_sec_alpha=catalogData.alpha_vec;
    angle_data.result_sec_beta=catalogData.beta_vec;
}


/*Рассчитывает угловые расстояние между точкой проецирования и звездами для звездного слайда, а так же переходную матрицу на координаты слайда*/
void SlideCreator::calculateAngularDistOptions(const StarSlideData& _slide_data,const CatalogData& _cat_data, bool check_sector)
{

    star_slide_data_prepared=false;
    slideData=_slide_data;
    catalogData=_cat_data;


    if(check_sector) // если учитываем сектора
    {
        calcAngularDistancesWithSectors();

    }

    else // если проходим весь каталог и не учитываем сектора
    {
        calcAngularDistancesNoSectors();
    }
    QVector< QVector<float> > trMat(calc_transition_matrix(slideData.pointAlpha,slideData.pointBeta,slideData.pointAzimut));
    angle_data.trMat=trMat;
    star_slide_data_prepared=true;

}


/*возвращает вспомогательные слайды в виде сетки*/
QVector<StarParameters> SlideCreator::createGridSlide(const GridSlideData &grid_d, bool check_distorsio, const DistorsioData& distData)
{

    slide_type=INVALID_TYPE;
    unsigned char* data_image = new unsigned char[grid_d.slideSizeX*grid_d.slideSizeY];
    QVector<StarParameters> coordinatesOfStars;
    if(check_distorsio)
    {
        for(int y=grid_d.grid_distance;y<(grid_d.slideSizeY-grid_d.grid_distance);y+=(grid_d.grid_distance+grid_d.pixelPerStar))
        {
            for (int x=grid_d.grid_distance;x<(grid_d.slideSizeX-grid_d.grid_distance);x+=(grid_d.grid_distance+grid_d.pixelPerStar))
            {

                double y_mm=(y-grid_d.slideSizeY/2)*grid_d.pix;
                double x_mm=(x-grid_d.slideSizeX/2)*grid_d.pix;
                y_mm=calc_dist(y_mm,x_mm,distData.yDistorsioVector);
                x_mm=calc_dist(x_mm,y_mm,distData.xDistorsioVector);

                int y_pix=((y_mm/grid_d.pix)+0.5)+grid_d.slideSizeY/2;
                int x_pix=((x_mm/grid_d.pix)+0.5)+grid_d.slideSizeX/2;

                StarParameters star_parameters;
                star_parameters.x=getStarPos(grid_d.pixelPerStar,x_pix);
                star_parameters.y=getStarPos(grid_d.pixelPerStar,y_pix);
                star_parameters.sizeX=getStarSize(grid_d.pixelPerStar,x_pix);
                star_parameters.sizeY=getStarSize(grid_d.pixelPerStar,y_pix);
                coordinatesOfStars.append(star_parameters);

                int start_pos=x_pix+y_pix*grid_d.slideSizeX;

                for(int i=0;i<grid_d.pixelPerStar;i++)/*i-Y, j-X*/
                {
                    for(int j=0;j<grid_d.pixelPerStar;j++)
                    {
                        if(outOfImage(start_pos,grid_d.slideSizeX,grid_d.slideSizeY,x_pix,j,i,grid_d.pixelPerStar))
                        {
                            continue;
                        }
                        data_image[start_pos+j+i*grid_d.slideSizeX]=255;
                    }

                }
            }
        }

    }

    else
    {
        for(int y=grid_d.grid_distance;y<(grid_d.slideSizeY-grid_d.grid_distance);y+=(grid_d.grid_distance+grid_d.pixelPerStar))
        {
            for (int x=grid_d.grid_distance;x<(grid_d.slideSizeX-grid_d.grid_distance);x+=(grid_d.grid_distance+grid_d.pixelPerStar))
            {
                StarParameters star_parameters;
                star_parameters.x=getStarPos(grid_d.pixelPerStar,x);
                star_parameters.y=getStarPos(grid_d.pixelPerStar,y);
                star_parameters.sizeX=getStarSize(grid_d.pixelPerStar,x);
                star_parameters.sizeY=getStarSize(grid_d.pixelPerStar,y);
                coordinatesOfStars.append(star_parameters);
                int start_pos=x+y*grid_d.slideSizeX;
                for(int i=0;i<grid_d.pixelPerStar;i++)
                {
                    for(int j=0;j<grid_d.pixelPerStar;j++)
                    {
                        if(outOfImage(start_pos,grid_d.slideSizeX,grid_d.slideSizeY,x,i,j,grid_d.pixelPerStar))
                        {
                            continue;
                        }
                        data_image[start_pos+j+i*grid_d.slideSizeX]=255;
                    }

                }
            }
        }
    }

    QImage img(data_image, grid_d.slideSizeX, grid_d.slideSizeY, grid_d.slideSizeX, QImage::Format_Grayscale8);

    // Создание таблицы цветов.
    QVector<QRgb> color_table;
    for(int i = 0; i < 256; ++i)
    {
        color_table.append(qRgb(i,i,i));
    }
    img.setColorTable(color_table);
    optimalImage.reset(new QImage(img.convertToFormat(QImage::Format_RGB32)));
    delete [] data_image;


    slide_type=GRID_TYPE;
    return coordinatesOfStars;
}


/*Перед выполнением этой функции должны быть рассчитаны угловые расстояния между точкой проецирования и звездами, т.е выполнена функция calculateAngularDistOptions  */
SlideParameters SlideCreator::createStarSlide(const float &focus, bool check_sector, bool check_distorsio, const DistorsioData &distData=DistorsioData())
{
    slide_type=INVALID_TYPE;
    if(!star_slide_data_prepared)
    {
        return SlideParameters();
    }

    int count_of_stars=0;//для подсчёта числа звезд, попавших на слайд
    QVector <double>result_alpha;
    QVector <double>result_beta;


    // считаем угол зрения в зависимости от фокусного расстояния и размерности матрицы
    double view_angle_x=atan((((slideData.slideSizeX/2)*slideData.pix)/(slideData.focStart)))*2*trans_to_grad;
    double view_angle_y=atan((((slideData.slideSizeY/2)*slideData.pix)/(slideData.focStart)))*2*trans_to_grad;
    double view_angle=sqrt(pow(view_angle_x,2)+pow(view_angle_y,2));

    // угол зрения- описанная окружность

    for(int i=0;i<angle_data.angle_cos.size();i++)
    {
        if(check_sector)// если учитываем сектора
        {
            double field_of_view_cos=cos(view_angle*trans_to_rad);
            if((angle_data.angle_cos[i])>=field_of_view_cos && catalogData.mv_vec[i]>slideData.minMv && catalogData.mv_vec[i]<slideData.maxMv)
            {
                result_alpha.append(angle_data.result_sec_alpha[i]);
                result_beta.append(angle_data.result_sec_beta[i]);

            }
        }
        else // если не учитываем
        {
            double inside_diam_of_view_cos=cos((slideData.insideViewAngle/2)*trans_to_rad);
            if((angle_data.angle_cos[i])>=(inside_diam_of_view_cos) && catalogData.mv_vec[i]>slideData.minMv && catalogData.mv_vec[i]<slideData.maxMv)
            {
                result_alpha.append(angle_data.result_sec_alpha[i]);
                result_beta.append(angle_data.result_sec_beta[i]);
            }
        }
    }
    QVector<double>filtered_l_st;
    QVector<double>filtered_m_st;
    QVector<double>filtered_n_st;

    for(int i=0;i<result_beta.size();i++)
    {
        double cos_b=cos(result_beta[i]*(trans_to_rad));
        double cos_a=cos(result_alpha[i]*(trans_to_rad));
        double sin_b=sin(result_beta[i]*(trans_to_rad));
        double sin_a=sin(result_alpha[i]*(trans_to_rad));

        filtered_l_st.append(cos_b*cos_a);
        filtered_m_st.append(cos_b*sin_a);
        filtered_n_st.append(sin_b);
    }


    double x_coord_mm,y_coord_mm;
    int x_coord,y_coord;
    float CC;
    unsigned char* data_image = new unsigned char[slideData.slideSizeX*slideData.slideSizeY];
    QVector<StarParameters> coordinatesOfStars;
    for(int i=0;i<filtered_l_st.size();i++)
    {

        CC=angle_data.trMat[2][0]*filtered_l_st[i]+angle_data.trMat[2][1]*filtered_m_st[i]+angle_data.trMat[2][2]*filtered_n_st[i];
        if(check_distorsio)
        {

            x_coord_mm=(-focus*(angle_data.trMat[0][0]*filtered_l_st[i]+angle_data.trMat[0][1]*filtered_m_st[i]
                    +angle_data.trMat[0][2]*filtered_n_st[i])/CC);
            y_coord_mm=(-focus*(angle_data.trMat[1][0]*filtered_l_st[i]+angle_data.trMat[1][1]*filtered_m_st[i]
                    +angle_data.trMat[1][2]*filtered_n_st[i])/CC);
            x_coord_mm= calc_dist(x_coord_mm,y_coord_mm,distData.xDistorsioVector);
            y_coord_mm= calc_dist(y_coord_mm,x_coord_mm,distData.yDistorsioVector);
            x_coord=x_coord_mm/slideData.pix+0.5;
            y_coord=y_coord_mm/slideData.pix+0.5;


        }

        else
        {
            x_coord=(-focus*(angle_data.trMat[0][0]*filtered_l_st[i]+angle_data.trMat[0][1]*filtered_m_st[i]
                    +angle_data.trMat[0][2]*filtered_n_st[i])/CC)/slideData.pix+0.5;
            y_coord=(-focus*(angle_data.trMat[1][0]*filtered_l_st[i]+angle_data.trMat[1][1]*filtered_m_st[i]
                    +angle_data.trMat[1][2]*filtered_n_st[i])/CC)/slideData.pix+0.5;

        }

        x_coord+=slideData.slideSizeX/2; y_coord+=slideData.slideSizeY/2;// x_coord,slideSizeX- ось X, y_coord,slideSizeY - ось Y

        if((x_coord>0) && (x_coord<slideData.slideSizeX) &&(y_coord>0) &&(y_coord<slideData.slideSizeY))
        {


            StarParameters star_parameters;
            star_parameters.x=getStarPos(slideData.pixelPerStar,x_coord);
            star_parameters.y=getStarPos(slideData.pixelPerStar,y_coord);
            star_parameters.sizeX=getStarSize(slideData.pixelPerStar,x_coord);
            star_parameters.sizeY=getStarSize(slideData.pixelPerStar,y_coord);
            coordinatesOfStars.append(star_parameters);


            int pos_pix=y_coord*slideData.slideSizeX+x_coord;
            int start_pos=pos_pix-(slideData.pixelPerStar/2)*slideData.slideSizeX-(slideData.pixelPerStar/2);

            for(int y=0;y<slideData.pixelPerStar;y++)
            {
                for(int x=0;x<slideData.pixelPerStar;x++)
                {

                    if(outOfImage(start_pos,slideData.slideSizeX,slideData.slideSizeY,x_coord,x,y,slideData.pixelPerStar))
                    {
                        continue;
                    }
                    data_image[start_pos+x+y*slideData.slideSizeX]=255;
                }

            }
            count_of_stars++;// подсчёт числа звезд в кадре
        }

    }

    QImage img(data_image, slideData.slideSizeX, slideData.slideSizeY, slideData.slideSizeX, QImage::Format_Grayscale8);
    // Создание таблицы цветов.
    QVector<QRgb> color_table;
    for(int i = 0; i < 256; ++i)
    {
        color_table.append(qRgb(i,i,i));
    }
    img.setColorTable(color_table);
    optimalImage.reset(new QImage(img.convertToFormat(QImage::Format_Mono)));
    delete [] data_image;

    SlideParameters image_data;
    image_data.count_of_stars=count_of_stars;
    image_data.view_angle_x=view_angle_x;
    image_data.view_angle_y=view_angle_y;
    image_data.coordinatesOfStars=coordinatesOfStars;

    slide_type=STAR_TYPE;
    return image_data;

}
/* Выдает предварительную информацию о размерах слайда, числе звезд на слайде.*/
/* Если слайдов несколько выдает так же информацию об размере общего изображения*/
TestSlideParameters SlideCreator::testStarSlide( bool check_sector, bool check_distorsio, const DistorsioData &distData)
{
    if(!star_slide_data_prepared)
    {
        return TestSlideParameters();
    }
    constexpr static double trans_to_rad=0.0174532925;
    constexpr static double trans_to_grad=57.29577957855229;
    int count_of_stars=0;//для подсчёта числа звезд, попавших на слайд


    // считаем угол зрения в зависимости от фокусного расстояния и размерности матрицы
    double view_angle_x=atan((((slideData.slideSizeX/2)*slideData.pix)/(slideData.focStart)))*2*trans_to_grad;
    double view_angle_y=atan((((slideData.slideSizeY/2)*slideData.pix)/(slideData.focStart)))*2*trans_to_grad;
    double view_angle=sqrt(pow(view_angle_x,2)+pow(view_angle_y,2));// угол зрения- описанная окружность

    QVector <double>result_alpha;
    QVector <double>result_beta;


    for(int i=0;i<angle_data.angle_cos.size();i++)
    {
        if(check_sector)
        {
            double field_of_view_cos=cos(view_angle*trans_to_rad);
            if((angle_data.angle_cos[i])>=field_of_view_cos && catalogData.mv_vec[i]>slideData.minMv && catalogData.mv_vec[i]<slideData.maxMv)
            {
                result_alpha.append(angle_data.result_sec_alpha[i]);
                result_beta.append(angle_data.result_sec_beta[i]);

            }
        }
        else
        {
            double inside_field_of_view_cos=cos((slideData.insideViewAngle/2)*trans_to_rad);
            if((angle_data.angle_cos[i])>=inside_field_of_view_cos && catalogData.mv_vec[i]>slideData.minMv && catalogData.mv_vec[i]<slideData.maxMv)
            {
                result_alpha.append(angle_data.result_sec_alpha[i]);
                result_beta.append(angle_data.result_sec_beta[i]);
            }
        }
    }


    QVector<double>filtered_l_st;
    QVector<double>filtered_m_st;
    QVector<double>filtered_n_st;
    for(int i=0;i<result_beta.size();i++)
    {
        double cos_b=cos(result_beta[i]*(trans_to_rad));
        double cos_a=cos(result_alpha[i]*(trans_to_rad));
        double sin_b=sin(result_beta[i]*(trans_to_rad));
        double sin_a=sin(result_alpha[i]*(trans_to_rad));

        filtered_l_st.append(cos_b*cos_a);
        filtered_m_st.append(cos_b*sin_a);
        filtered_n_st.append(sin_b);
    }

    double x_coord_mm,y_coord_mm;
    int x_coord,y_coord;
    float CC;
    for(int i=0;i<filtered_l_st.size();i++)
    {
        CC=angle_data.trMat[2][0]*filtered_l_st[i]+angle_data.trMat[2][1]*filtered_m_st[i]+angle_data.trMat[2][2]*filtered_n_st[i];
        if(check_distorsio)
        {

            x_coord_mm=(-slideData.focStart*(angle_data.trMat[0][0]*filtered_l_st[i]+angle_data.trMat[0][1]*filtered_m_st[i]
                    +angle_data.trMat[0][2]*filtered_n_st[i])/CC);
            y_coord_mm=(-slideData.focStart*(angle_data.trMat[1][0]*filtered_l_st[i]+angle_data.trMat[1][1]*filtered_m_st[i]
                    +angle_data.trMat[1][2]*filtered_n_st[i])/CC);
            x_coord_mm= calc_dist(x_coord_mm,y_coord_mm,distData.xDistorsioVector);
            y_coord_mm= calc_dist(y_coord_mm,x_coord_mm,distData.yDistorsioVector);
            x_coord=x_coord_mm/slideData.pix+0.5;
            y_coord=y_coord_mm/slideData.pix+0.5;
        }

        else
        {
            x_coord=(-slideData.focStart*(angle_data.trMat[0][0]*filtered_l_st[i]+angle_data.trMat[0][1]*filtered_m_st[i]
                    +angle_data.trMat[0][2]*filtered_n_st[i])/CC)/slideData.pix+0.5;
            y_coord=(-slideData.focStart*(angle_data.trMat[1][0]*filtered_l_st[i]+angle_data.trMat[1][1]*filtered_m_st[i]
                    +angle_data.trMat[1][2]*filtered_n_st[i])/CC)/slideData.pix+0.5;

        }

        x_coord+=slideData.slideSizeX/2; y_coord+=slideData.slideSizeY/2;// x_coord,slideSizeX- ось X, y_coord,slideSizeY - ось Y

        if((x_coord>0) && (x_coord<slideData.slideSizeX) &&(y_coord>0) &&(y_coord<slideData.slideSizeY))
        {
            count_of_stars++; // подсчёт числа звезд
        }

    }

    TestSlideParameters testData;
    testData.view_angle_x=view_angle_x;
    testData.view_angle_y=view_angle_y;
    testData.count_of_stars=count_of_stars;
    return testData;

}

QSharedPointer<QImage> SlideCreator::getSlidePointer()
{
    return optimalImage;
}
