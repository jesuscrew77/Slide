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
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QSharedPointer>



namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



private:

    void chooseCatalog();
    void useCurrentCatalog();
    void createImage();
    void createGrid();
    void testForSlide();
    void saveImage();
    void chooseDistorsioFile();
    void setStyle();
    void initWidgetsOptionConnections();
    GroupImgParams readGroupImgParams();
    StarSlideData readInputStarSlideData();
    GridSlideData readInputGridSlideData();
    InscriptParams readInscriptionParams();
    void drawSlide(QSharedPointer <QImage> im, QVector<QImage>& im_v, int  sz_x, int  sz_y, int space, int slideSizeX, int slideSizeY);
    void drawGridSlides(QSharedPointer <QImage> im, QImage &opt_img, int  sz_x, int  sz_y, int space, int slideSizeX, int slideSizeY);
    void chooseTab();
    void prepareTextToSvg(QString& text);
    void drawPreviewItem(const QByteArray& svgByteArray, int slideSizeX, int slideSizeY);
    void clearSceneAndImages();
    void setImagesSizes(int slideSizeX, int slideSizeY, const GroupImgParams& p);
    void makeInscription(QSharedPointer <QImage> optimalImage,const QString& setableText, int fontX, int fontY, int fontSize);
    void compareImageFocSize(const int countOfSlides,const int countOfFoc);
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


    Ui::MainWindow* ui;

    Catalog catalogData;
    QVector <QSharedPointer <QImage>> images;
    QSharedPointer<QImage> optimalImage;
    QScopedPointer<QGraphicsScene> scene;
    QSharedPointer<QGraphicsSvgItem> svgPreviewItem;
    QSharedPointer<QSvgRenderer> svgPreview;
    QDomDocument svgSlide;

    QSettings* settings = nullptr;
    QList <double> xDistorsioVector;
    QList <double> yDistorsioVector;
    QString filename;
    QString lastCatalogDirectory;
    QString distorsioFilename;
    QString lastDistorsioDirectory;
    bool catalogIsRead = false;
    bool distorsioIsRead = false;
    constexpr static const int imageSizeLimit = 32767;
    constexpr static const int countOfImages = 4;

};






#endif // MAINWINDOW_H
