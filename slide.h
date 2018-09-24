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
    double focStart;
    double focEnd;
    double focStep;
    double minMv;
    double maxMv;
    double pix;
    int pixelPerStar;
    int slideSizeX;
    int slideSizeY;
    double slideSizeMMy;
    double slideSizeMMx;
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
    QVector <QVector<double>> trMat;
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


    void calculateAngularDistOptions (const StarSlideData& _slideData, const Catalog& _catData, bool checkSector);

    SlideParameters createStarSlide(double focus, bool checkSector, bool dist_check, const DistorsioData& distData);

    QVector <StarParameters> createGridSlide(const GridSlideData& slideData, bool checkDistorsio, const DistorsioData& distData);

    TestSlideParameters testStarSlide(bool checkSector, bool checkDistorsio, const DistorsioData& distData);

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
    double calcScalarProduct(double lOz, double lSt, double mOz, double mSt, double nOz, double nSt);

    void calcAngularDistancesWithSectors();

    void calcAngularDistancesNoSectors();

    void calcViewAngle(double& viewAngleX, double& viewAngleY, double& view_angle);

    bool outOfImage(int startPos, int xSize, int ySize, int centralXc, int x, int y, int pixelPerStar);

    int getStarSize(int defaultSize, int starPos);

    int getStarPos(int defaultSize, int starPos);

    double calculateDistorsio(double coordF, double coordS, const QList <double>& distCoef);

    QVector< QVector<double> > calcTransitionMatrix(double pointAlpha, double pointBeta, double pointAzimut);



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
