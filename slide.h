#ifndef SLIDE_H
#define SLIDE_H
#include <catalog.h>
#include <QSharedPointer>
#include <QImage>
#include <QRgb>

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
    QVector<StarParameters> coordsOfStars;
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
    SlideParameters  createStarSlide(float focus, bool check_sector, bool dist_check, const DistorsioData& distData);
    QVector <StarParameters>  createGridSlide(const GridSlideData& slideData, bool check_distorsio, const DistorsioData& distData);
    TestSlideParameters testStarSlide(bool check_sector, bool check_distorsio, const DistorsioData &distData);
    QSharedPointer <QImage> getSlidePointer();
    SLIDE_TYPE SlideType() const noexcept { return slide_type; }

private:
    double calcScalarProduct(double l_oz, double l_st, double m_oz, double m_st, double n_oz, double n_st);
    void calcAngularDistancesWithSectors();
    void calcAngularDistancesNoSectors();
    bool outOfImage(int start_pos,int x_size, int y_size,int central_x_c, int x, int y, int pixelPerStar);
    int getStarSize(int default_size, int star_pos);
    int getStarPos(int default_size, int star_pos);
    double& calc_dist(double &coord_a, const double &coord_b,const QList<double>distorsio_coef);
    QVector< QVector<float> > calc_transition_matrix(const double &pointAlpha,const double &pointBeta,const double &pointAzimut);

    QSharedPointer<QImage> optimalImage;
    AngularDistanceOptions angle_data;
    StarSlideData slideData;
    CatalogData catalogData;
    SLIDE_TYPE slide_type;
    bool star_slide_data_prepared = false;

    constexpr static double trans_to_rad = 0.0174532925;
    constexpr static double trans_to_grad = 57.29577957855229;



};

#endif // SLIDE_H
