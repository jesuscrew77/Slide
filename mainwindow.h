#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include<QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include<QSettings>
#include <slide.h>


struct InscriptParams
{
    int fontSize = 0;
    int fontX = 0;
    int fontY = 0;
    QString prefix;
    QString suffix;
};

struct GroupImageParameters
{
    int groupImageHeight = 0;/*количество слайдов в высоту*/
    int groupImageWidth = 0;/*количество слайдов в ширину*/
    int space = 0;/*расстояние между слайдами*/
};

class SlideUIException : public std::exception
{
public:
    SlideUIException() noexcept: whatStr("Неизвестная ошибка") {}
    SlideUIException(const std::string &&whatStr) noexcept : whatStr(std::move(whatStr)) { }
    SlideUIException(const std::string &whatStr) noexcept : whatStr(whatStr) { }
    ~DbException() noexcept = default;

    const char* what() const noexcept override
    {
        return whatStr.c_str();
    }

private:
    std::string whatStr;
};

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void chooseCatalog();
    void useCurrentCatalog();
    void createImage();
    void createGrid();
    void testForSlide();
    void saveImage();
    void chooseDistorsioFile();




private:
    void setStyle();
    QByteArray createPreviewImage(const int imageWidth,const int iamgeHeight,const int fontSize,const int fontX,const int fontY,const QString setableText,QVector<StarParameters> coordinatesOfStars);
    GroupImageParameters readGroupImageParameters();
    StarSlideData readInputStarSlideData();
    GridSlideData readInputGridSlideData();
    InscriptParams readInscriptionParams();
    void drawSlide(QScopedPointer<QImage> &im, QVector<QImage> &im_v, const int &sz_x, const int &sz_y, const int &sp, const int &slideSizeX, const int &slideSizeY);
    void drawGridSlides(QScopedPointer<QImage> &im, QImage &opt_img, const int &sz_x, const int &sz_y, const int &sp, const int &slideSizeX, const int &slideSizeY);
    void drawPreviewItems(const int &slideSizeX,const int &slideSizeY,const int &groupImageWidth,const int &groupImageHeight,const int &space);
    void chooseTab();
    void drawPreviewItem(const QByteArray svg_img,const int &slideSizeX,const int &slideSizeY);
    void clearSceneAndImages();
    void setImagesSizes(const int& slideSizeX,const int &slideSizeY,const int &groupImageHeight ,const int &groupImageWidth, const int &space);
    void makeInscription(QSharedPointer<QImage> optimalImage,const QString& setableText,const int& fontX,const int& fontY,const int& fontSize);
    void setUIstate(bool state);
    void openCatalog();
    void updateLineXPix();
    void updateLineYPix();
    void updateLineXMM();
    void updateLineYMM();
    void setLine();
    void setDistButtonState();
    void saveSettings();
    void loadSettings();
    void setMatrixState();
    void setAlgorithmState();
    bool eventFilter(QObject *object, QEvent *event);
    void wheelEvent(QWheelEvent* event);

    Ui::MainWindow *ui;

    CatalogData catalogData;
    QScopedPointer<QImage> leftTopImage;
    QScopedPointer<QImage> leftDownImage;
    QScopedPointer<QImage> rightTopImage;
    QScopedPointer<QImage> rightDownImage;
    QSharedPointer<QImage> optimalImage;


    QScopedPointer<QGraphicsScene> scene;
    QSharedPointer<QGraphicsSvgItem> svgPreviewItem;
    QSharedPointer<QSvgRenderer> svgPreview;
    QVector<QSharedPointer<QSvgRenderer>> vectorOfSvgPreviews;
    QVector<QSharedPointer<QGraphicsSvgItem>> vectorOfSvgItems;


    QSettings* settings = nullptr;
    QList<double> xDistorsioVector;
    QList<double> yDistorsioVector;
    QString filename;
    QString lastCatalogDirectory;
    QString distorsioFilename;
    QString lastDistorsioDirectory;
    bool catalogIsRead = false;
    bool distorsioIsRead = false;
    void initWidgetsOptionConnections();
};



#pragma pack(push,1)
struct sector // каталог секторов
{
    float alpha_c;
    float beta_c;
    qint16 count_in_sector;
    int shift;
};
#pragma pack(pop)



#pragma pack(push,1)
struct data_star // основной каталог/бортовой каталог
{
    qint32  NSAO;
    qint32 alpha;
    qint32 beta;
    qint16 ualpha;
    qint16 ubeta;
    unsigned char mv;
    char sp;
};
#pragma pack(pop)



#pragma pack(push,1)
struct numbers // основной каталог/бортовой каталог
{
    qint16 num;
};
#pragma pack(pop)




#endif // MAINWINDOW_H
