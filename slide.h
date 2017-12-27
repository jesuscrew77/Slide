#ifndef SLIDE_H
#define SLIDE_H
#include <catalog.h>
#include <QSharedPointer>
#include <QImage>
#include <QRgb>
#include <QScopedArrayPointer>
#include <QDomDocument>

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
    int gridDistance;
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
    QVector <double> angleCos;
    QVector <double> resultAlpha;
    QVector <double> resultBeta;
    QVector <QVector<float>> trMat;
};
using ADO = AngularDistanceOptions ;

struct SlideParameters /*инфомация о звездном слайде*/
{
    double viewAngleX;
    double viewAngleY;
    int countOfStars;
    QVector <StarParameters> coordsOfStars;
};

struct TestSlideParameters
{
    double viewAngleX;
    double viewAngleY;
    int countOfStars;
    QVector <StarParameters> coordsOfStars;
};


struct InscriptParams
{
    int fontSize = 0;
    int fontX = 0;
    int fontY = 0;
    QString prefix;
    QString suffix;
};

struct GroupImgParams
{
    int countY;/*количество слайдов в высоту*/
    int countX;/*количество слайдов в ширину*/
    int space;/*расстояние между слайдами*/
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

    SlideParameters createStarSlide(float focus, bool check_sector, bool dist_check, const DistorsioData& distData);

    QVector <StarParameters> createGridSlide(const GridSlideData& slideData, bool check_distorsio, const DistorsioData& distData);

    TestSlideParameters testStarSlide(bool check_sector, bool check_distorsio, const DistorsioData& distData);

    QSharedPointer <QImage> getSlidePointer() noexcept;

    SLIDE_TYPE SlideType() const noexcept { return slideType; }

    void setOnlyParameters(bool flag) noexcept
    {
        onlyParameters = flag;
    }

    static QDomDocument createFullSvgSymbols(QVector <QVector <StarParameters>> coordsOfStars,
                                        const int imageWidth, const int imageHeight,
                                        const GroupImgParams& gParams,
                                        const InscriptParams& iParams,
                                        QVector <QString> setableText);

    static QDomDocument createFullSvg(QVector <QVector <StarParameters>> coordsOfStars,
                                          const int imageWidth, const int imageHeight,
                                          const GroupImgParams& gParams,
                                          const InscriptParams& iParams,
                                          QVector <QString> setableText);


    static QDomDocument createSvg(const StarSlideData& sData,
                                 const InscriptParams& iParams,
                                 const QString setableText,
                                 QVector <StarParameters> coordsOfStars);

    static QDomDocument createSvg(const GridSlideData& gData,
                                 const InscriptParams& iParams,
                                 const QString setableText,
                                 QVector <StarParameters> coordsOfStars);

private:
    double calcScalarProduct(double l_oz, double l_st, double m_oz, double m_st, double n_oz, double n_st);

    void calcAngularDistancesWithSectors();

    void calcAngularDistancesNoSectors();

    void calcViewAngle(double& viewAngleX, double& viewAngleY, double& view_angle);

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
    bool onlyParameters = false;
    static  QVector <QRgb> colorTable;
    constexpr static double transToRad = 0.0174532925;
    constexpr static double transToGrad = 57.29577957855229;



};

#endif // SLIDE_H
