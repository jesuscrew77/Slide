#ifndef SLIDE_H
#define SLIDE_H
#include<QVector>
#include <QImage>
#include <QPen>
#include <QPainter>
#include <QApplication>
#include <QMessageBox>
#include <fstream>
#include <cmath>
#include <QVector>
#include <QString>
#include <algorithm>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QDataStream>
#include <QStringList>
#include <QImage>
#include<QImageIOPlugin>
#include<QImageWriter>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include<QWheelEvent>
#include <QScrollBar>
#include<QChar>
#include<QSettings>
#include<QTextCodec>
#include <exception>
#include <QTextStream>
#include <QTime>
#include <QDomDocument>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QGraphicsSvgItem>
#define QT_SHAREDPOINTER_TRACK_POINTERS
#include <QSharedPointer>

struct StarSlideData /*структура, содержащая информацию для создания слайда*/
{
    double pointAlpha = 0;
    double pointBeta = 0;
    double pointAzimut = 0;
    double insideViewAngle = 0;
    float focStart = 0;
    float focEnd = 0;
    float focStep = 0;
    double minMv = 0;
    double maxMv = 0;
    float pix = 0;
    int pixelPerStar = 0;
    int slideSizeX = 0;
    int slideSizeY = 0;
    double slideSizeMM_Y = 0;
    double slideSizeMM_X = 0;
};
struct StarParameters /*структура, содержащая информацию о координатах и размерах звезд*/
{
    int x = 0;
    int y = 0;
    int sizeX = 0;
    int sizeY = 0;
};


struct GridSlideData /*структура содержащая данные для создания слайда-сетки*/
{
    int grid_distance = 0;
    int pixelPerStar = 0;
    int slideSizeX = 0;
    int slideSizeY = 0;
    double pix = 0;
    int groupImageHeight = 0;/*количество слайдов в высоту*/
    int groupImageWidth = 0;/*количество слайдов в ширину*/
    int space = 0;/*расстояние между слайдами*/
};

struct CatalogData /*структура, содержащая звездный каталог*/
{
    QVector <double> alpha_vec;
    QVector <double> beta_vec;
    QVector <float> mv_vec;
    QVector <double> alpha_vec_sec;
    QVector <double> beta_vec_sec;
    QVector <long> count_sec_vec;
    QVector <long> shift_vec;
    QVector<short> new_numn;
};
struct DistorsioData
{
    QList<double> xDistorsioVector;
    QList<double> yDistorsioVector;
};

struct AngularDistanceOptions
{
    QVector<double> angle_cos;
    QVector<double> result_sec_alpha;
    QVector<double> result_sec_beta;
    QVector< QVector<float>> trMat;
};
typedef AngularDistanceOptions ADO;

struct SlideParameters /*структура, содержания инфомацию о звездном слайде*/
{
    double view_angle_x = 0;
    double view_angle_y = 0;
    int count_of_stars = 0;
    QVector<StarParameters> coordinatesOfStars;
};

struct TestSlideParameters
{
    double view_angle_x = 0;
    double view_angle_y = 0;
    int count_of_stars = 0;
};


class SlideCreator
{

public:

    enum SLIDE_TYPE
    {
        INVALID_TYPE,
        STAR_TYPE,
        GRID_TYPE

    };

    explicit SlideCreator();
    SlideCreator(const SlideCreator&)                   =delete;
    SlideCreator(SlideCreator&& )                       =delete;
    SlideCreator& operator=(const SlideCreator&)        =delete;
    SlideCreator& operator=(SlideCreator&&)             =delete;
    ~SlideCreator()                                     =default;


    void calculateAngularDistOptions (const StarSlideData &_slide_data,const CatalogData& _cat_data,bool check_sector);
    SlideParameters  createStarSlide(const float& focus, bool check_sector, bool dist_check, const DistorsioData& distData);
    QVector<StarParameters>  createGridSlide(const GridSlideData& slideData, bool check_distorsio, const DistorsioData& distData);
    TestSlideParameters testStarSlide(bool check_sector, bool check_distorsio, const DistorsioData &distData);
    QSharedPointer<QImage> getSlidePointer();
    SLIDE_TYPE SlideType() const
    {
        return slide_type;
    }

private:
    void calcAngularDistancesWithSectors();
    void calcAngularDistancesNoSectors();
    bool outOfImage(const int&start_pos,const int& x_size,const int& y_size,const int&central_x_c,const int&x, const int&y, const int& pixelPerStar);
    int getStarSize(const int& default_size, const int& star_pos);
    int getStarPos(const int& default_size, const int& star_pos);
    double& calc_dist(double &coord_a, const double &coord_b,const QList<double>distorsio_coef);
    QVector< QVector<float> > calc_transition_matrix(const double &pointAlpha,const double &pointBeta,const double &pointAzimut);

    QSharedPointer<QImage> optimalImage;
    AngularDistanceOptions angle_data;
    StarSlideData slideData;
    CatalogData catalogData;
    SLIDE_TYPE slide_type;
    bool star_slide_data_prepared=false;

    constexpr static double trans_to_rad=0.0174532925;
    constexpr static double trans_to_grad=57.29577957855229;



};

#endif // SLIDE_H
