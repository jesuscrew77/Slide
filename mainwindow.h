#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define QT_SHAREDPOINTER_TRACK_POINTERS
#include <QMainWindow>
#include <slide.h>
#include <QVector>
#include <QPen>
#include <QPainter>
#include <QApplication>
#include <QMessageBox>
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
#include <QImageIOPlugin>
#include <QImageWriter>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QChar>
#include <QSettings>
#include <QTextCodec>
#include <exception>
#include <QTextStream>
#include <QTime>
#include <QDomDocument>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QGraphicsSvgItem>
#include <QSharedPointer>


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
    void initWidgetsOptionConnections();
    QByteArray createPreviewImage(const int imageWidth,const int iamgeHeight,const int fontSize,const int fontX,const int fontY,const QString setableText,QVector<StarParameters> coordinatesOfStars);
    GroupImageParameters readGroupImageParameters();
    StarSlideData readInputStarSlideData();
    GridSlideData readInputGridSlideData();
    InscriptParams readInscriptionParams();
    void drawSlide(QSharedPointer <QImage> im, QVector<QImage>& im_v, int  sz_x, int  sz_y, int  space, int  slideSizeX, int  slideSizeY);
    void drawGridSlides(QSharedPointer <QImage> im, QImage &opt_img, int  sz_x, int  sz_y, int  space, int  slideSizeX, int  slideSizeY);
    void drawPreviewItems(int  slideSizeX,int  slideSizeY,int  groupImageWidth,int  groupImageHeight,int  space);
    void chooseTab();
    void drawPreviewItem(const QByteArray svg_img,int  slideSizeX,int  slideSizeY);
    void clearSceneAndImages();
    void setImagesSizes(int slideSizeX,int slideSizeY,int groupImageHeight ,int groupImageWidth, int space);
    void makeInscription(QSharedPointer<QImage> optimalImage,const QString& setableText, int fontX,int fontY,int fontSize);
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


    CatalogData catalogData;
//    QScopedPointer<QImage> leftTopImage;
//    QScopedPointer<QImage> leftDownImage;
//    QScopedPointer<QImage> rightTopImage;
//    QScopedPointer<QImage> rightDownImage;
    QVector <QSharedPointer <QImage>> images;
    QSharedPointer<QImage> optimalImage;


    QScopedPointer<QGraphicsScene> scene;
    QSharedPointer<QGraphicsSvgItem> svgPreviewItem;
    QSharedPointer<QSvgRenderer> svgPreview;
    QVector<QSharedPointer<QSvgRenderer>> vectorOfSvgPreviews;
    QVector<QSharedPointer<QGraphicsSvgItem>> vectorOfSvgItems;

    Ui::MainWindow* ui;
    QSettings* settings = nullptr;
    QList<double> xDistorsioVector;
    QList<double> yDistorsioVector;
    QString filename;
    QString lastCatalogDirectory;
    QString distorsioFilename;
    QString lastDistorsioDirectory;
    bool catalogIsRead = false;
    bool distorsioIsRead = false;

};






#endif // MAINWINDOW_H
