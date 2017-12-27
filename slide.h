#ifndef SLIDE_H
#define SLIDE_H
#include <catalog.h>
#include <QSharedPointer>
#include <QImage>
#include <QRgb>
#include <QScopedArrayPointer>

struct StarSlideData /*информация для создания слайда*/
{
    double pointAlpha;
    double pointBeta;
    double pointAzimut;
    double insideViewAngle;
    float focStart;
    float focEnd;
    float focStep;
    double minMv;
    double maxMv;
    float pix;
    int pixelPerStar;
    int slideSizeX;
    int slideSizeY;
    double slideSizeMM_Y;
    double slideSizeMM_X;
};
struct StarParameters /*информация о координатах и размерах звезд*/
{
    int x;
    int y;
    int sizeX;
    int sizeY;
};


struct GridSlideData /* данные для создания слайда-сетки*/
{
    int grid_distance;
    int pixelPerStar;
    int slideSizeX;
    int slideSizeY;
    double pix;
};


struct DistorsioData
{
    QList <double> xDistorsioVector;
    QList <double> yDistorsioVector;
};

struct AngularDistanceOptions
{
    QVector <double> angle_cos;
    QVector <double> result_alpha;
    QVector <double> result_delta;
    QVector <QVector<float>> trMat;
};
using ADO = AngularDistanceOptions ;

struct SlideParameters /*инфомация о звездном слайде*/
{
    double view_angle_x = 0;
    double view_angle_y = 0;
    int count_of_stars = 0;
    QVector <StarParameters> coordsOfStars;
};

struct TestSlideParameters
{
    double view_angle_x = 0;
    double view_angle_y = 0;
    int count_of_stars = 0;
    QVector <StarParameters> coordsOfStars;
};


class SlideCreator
{

public:

     enum class SLIDE_TYPE
    {
        INVALID_TYPE,
        STAR_TYPE,
        GRID_TYPE

    };

    explicit SlideCreator();
    SlideCreator(const SlideCreator&)                   =delete;
    SlideCreator(SlideCreator&& )                       =delete;
    SlideCreator& operator = (const SlideCreator&)      =delete;
    SlideCreator& operator = (SlideCreator&&)           =delete;
    ~SlideCreator()                                     =default;


    void calculateAngularDistOptions (const StarSlideData& _slide_data,const Catalog& _cat_data,bool check_sector);

    SlideParameters  createStarSlide(float focus, bool check_sector, bool dist_check, const DistorsioData& distData);

    QVector <StarParameters>  createGridSlide(const GridSlideData& slideData, bool check_distorsio, const DistorsioData& distData);

    TestSlideParameters testStarSlide(bool check_sector, bool check_distorsio, const DistorsioData& distData);

    QSharedPointer <QImage> getSlidePointer() noexcept;

    SLIDE_TYPE SlideType() const noexcept { return slideType; }


private:
    double calcScalarProduct(double l_oz, double l_st, double m_oz, double m_st, double n_oz, double n_st);

    void calcAngularDistancesWithSectors();

    void calcAngularDistancesNoSectors();

    void calcViewAngle(double& view_angle_x, double& view_angle_y, double& view_angle);

    bool outOfImage(int start_pos,int x_size, int y_size,int central_x_c, int x, int y, int pixelPerStar);

    int getStarSize(int default_size, int star_pos);

    int getStarPos(int default_size, int star_pos);

    double calculateDistorsio(double point_c, double coord_a, double coord_b, const QList <double>& distorsio_coef);

    QVector< QVector<float> > calcTransitionMatrix(double pointAlpha, double pointBeta, double pointAzimut);


    QSharedPointer <QImage> optimalImage;
    AngularDistanceOptions angleData;
    StarSlideData slideData;
    Catalog catalogData;
    SLIDE_TYPE slideType;
    bool starSlideDataPrepare = false;
    static  QVector <QRgb> colorTable;
    constexpr static double trans_to_rad = 0.0174532925;
    constexpr static double trans_to_grad = 57.29577957855229;



};

#endif // SLIDE_H
