#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <QDebug>



Q_DECLARE_METATYPE(QList<double>)
using namespace std;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStyle();
    qRegisterMetaTypeStreamOperators<QList<double> >("QList<double>");
    settings = new QSettings(QDir::currentPath() + "/" + "config.ini", QSettings::IniFormat, this);
    initWidgetsOptionConnections();
    loadSettings();
}

void MainWindow::initWidgetsOptionConnections()
{
    QStringList pixperstar;
    pixperstar<<"Выберите..."<<"1x1"<<"2x2"<<"3x3"<<"4x4"<<"5x5"<<"6x6"<<"7x7"<<"8x8"<<"9x9"<<"10x10";
    ui->comboBox->insertItems(0,pixperstar);
    QRegExp rx( "^[-0-9]*[.]{1}[0-9]*$" );
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->betaLineEdit->setValidator(validator);
    ui->alphaLineEdit->setValidator(validator);
    ui->azLineEdit->setValidator(validator);
    ui->FocStartLineEdit->setValidator(validator);
    ui->FocEndLineEdit->setValidator(validator);
    ui->FocStepLineEdit->setValidator(validator);
    ui->minMvLineEdit->setValidator(validator);
    ui->maxMvLineEdit->setValidator(validator);
    ui->pixSizeLineEdit->setValidator(validator);
    ui->slideHeightLineEdit->setValidator(validator);
    ui->slideWidthLineEdit->setValidator(validator);
    ui->slidePixHeightLineEdit->setValidator(new QIntValidator(100, 32767, this));
    ui->slidePixWidthLineEdit->setValidator(new QIntValidator(100, 32767, this));
    ui->textLeftLineEdit->setValidator(new QIntValidator(100, 32767,this));
    ui->textAtopLineEdit->setValidator(new QIntValidator(100, 32767,this));
    ui->textSizeLineEdit->setValidator(new QIntValidator(0, 32767,this));
    ui->imageWeightLineEdit->setValidator(new QIntValidator(0,100,this));
    ui->imageHeightLineEdit->setValidator(new QIntValidator(0,100,this));
    ui->GridLenghtLineEdit->setValidator(new QIntValidator(0,1000,this));
    ui->imageSpaceLineEdit->setValidator(new QIntValidator(0,1000,this));
    ui->OneSlideRadioButton->setChecked(true);
    ui->filenameLineEdit->setReadOnly(true);
    ui->DistorsioPathLineEdit->setReadOnly(true);
    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setInteractive(true);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);


    if(ui->OneSlideRadioButton->isChecked())
    {
        ui->FocEndLineEdit->setDisabled(true);
        ui->FocStepLineEdit->setDisabled(true);
        ui->imageWeightLineEdit->setDisabled(true);
        ui->imageHeightLineEdit->setDisabled(true);
        ui->imageSpaceLineEdit->setDisabled(true);
    }
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);




    connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::chooseCatalog);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&MainWindow::useCurrentCatalog);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::createImage);
    connect(ui->slideHeightLineEdit,&QLineEdit::textEdited,this,&MainWindow::updateLineXPix);
    connect(ui->slideWidthLineEdit,&QLineEdit::textEdited,this,&MainWindow::updateLineYPix);
    connect(ui->pixSizeLineEdit,&QLineEdit::textEdited,this,&MainWindow::updateLineXMM);
    connect(ui->slidePixHeightLineEdit,&QLineEdit::textEdited,this,&MainWindow::updateLineXMM);
    connect(ui->pixSizeLineEdit,&QLineEdit::textEdited,this,&MainWindow::updateLineYMM);
    connect(ui->slidePixWidthLineEdit,&QLineEdit::textEdited,this,&MainWindow::updateLineYMM);
    connect(ui->GroupSlideRadioButton,&QRadioButton::toggled,this,&MainWindow::setLine);
    connect(ui->OneSlideRadioButton,&QRadioButton::toggled,this,&MainWindow::setLine);
    connect(ui->slideHeightLineEdit,&QLineEdit::textEdited,this,&MainWindow::setMatrixState);
    connect(ui->slidePixHeightLineEdit,&QLineEdit::textEdited,this,&MainWindow::setMatrixState);
    connect(ui->checkBox,&QCheckBox::stateChanged,this,&MainWindow::setMatrixState);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&MainWindow::saveImage);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&MainWindow::testForSlide);
    connect(ui->checkSector,&QCheckBox::stateChanged,this,&MainWindow::setAlgorithmState);
    connect(ui->pushButton_6,&QPushButton::clicked,this,&MainWindow::chooseDistorsioFile);
    connect(ui->DistorsioCheck,&QCheckBox::stateChanged,this,&MainWindow::setDistButtonState);
    connect(ui->GenTabWidget,&QTabWidget::currentChanged,this,&MainWindow::chooseTab);
    connect(ui->pushButton_7,&QPushButton::clicked,this,&MainWindow::createGrid);
}

void MainWindow::setStyle()
{
    QFile file(":/style.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
        file.close();
    }
}


void MainWindow::saveSettings()
{
    settings->setValue("filename",filename);
    settings->setValue("distorsioFilename",distorsioFilename);
    settings->setValue("DistRead",distorsioIsRead);
    settings->setValue("betaLineEdit",ui->betaLineEdit->text());
    settings->setValue("alphaLineEdit",ui->alphaLineEdit->text());
    settings->setValue("azLineEdit",ui->azLineEdit->text());
    settings->setValue("FocStartLineEdit",ui->FocStartLineEdit->text());
    settings->setValue("FocEndLineEdit",ui->FocEndLineEdit->text());
    settings->setValue("FocStepLineEdit",ui->FocStepLineEdit->text());
    settings->setValue("minMvLineEdit",ui->minMvLineEdit->text());
    settings->setValue("maxMvLineEdit",ui->maxMvLineEdit->text());
    settings->setValue("pixSizeLineEdit",ui->pixSizeLineEdit->text());
    settings->setValue("slideHeightLineEdit",ui->slideHeightLineEdit->text());
    settings->setValue("slidewidthLineEdit",ui->slideWidthLineEdit->text());
    settings->setValue("slidePixHeightLineEdit",ui->slidePixHeightLineEdit->text());
    settings->setValue("slidePixwidthLineEdit",ui->slidePixWidthLineEdit->text());
    settings->setValue("textLeftLineEdit",ui->textLeftLineEdit->text());
    settings->setValue("textAtopLineEdit",ui->textAtopLineEdit->text());
    settings->setValue("textSizeLineEdit",ui->textSizeLineEdit->text());
    settings->setValue("imageWeightLineEdit",ui->imageWeightLineEdit->text());
    settings->setValue("imageHeightLineEdit",ui->imageHeightLineEdit->text());
    settings->setValue("imageSpaceLineEdit",ui->imageSpaceLineEdit->text());
    settings->setValue("prefixLineEdit",ui->prefixLineEdit->text());
    settings->setValue("suffixLineEdit",ui->suffixLineEdit->text());
    settings->setValue("slidewidthLineEdit_state",ui->slideWidthLineEdit->isReadOnly());
    settings->setValue("slidePixwidthLineEdit_state",ui->slideWidthLineEdit->isReadOnly());
    settings->setValue("checkBox", ui->checkBox->isChecked());
    settings->setValue("comboBoxIndex", ui->comboBox->currentIndex());
    settings->setValue("showPreviewCheckBox", ui->showPreviewCheckBox->isChecked());
    settings->setValue("checkSector",ui->checkSector->isChecked());
    settings->setValue("PicDiamLineEdit",ui->PicDiamLineEdit->text());
    settings->setValue("PicDiamLineEdit_state",ui->checkSector->isChecked());
    settings->setValue("DistorsioCheck",ui->DistorsioCheck->isChecked());
    settings->setValue("xDistorsioVector",QVariant::fromValue(xDistorsioVector));
    settings->setValue("yDistorsioVector",QVariant::fromValue(yDistorsioVector));
    settings->setValue("GenTabCurrent",ui->GenTabWidget->currentIndex());
    settings->setValue("AddTabCurrent",ui->AddTabWidget->currentIndex());
    settings->setValue("GridLenghtLineEdit",ui->GridLenghtLineEdit->text());

    settings->sync();

}

void MainWindow::loadSettings()
{
    filename = settings->value("filename","Каталог не выбран...").toString();
    ui->filenameLineEdit->setText(filename);
    distorsioFilename = settings->value("distorsioFilename","Файл не выбран...").toString();
    ui->DistorsioPathLineEdit->setText(distorsioFilename);
    distorsioIsRead = settings->value("DistRead","false").toBool();
    ui->betaLineEdit->setText(settings->value("betaLineEdit","45").toString());
    ui->alphaLineEdit->setText(settings->value("alphaLineEdit","45").toString());
    ui->azLineEdit->setText(settings->value("azLineEdit","0").toString());
    ui->FocStartLineEdit->setText(settings->value("FocStartLineEdit","105").toString());
    ui->FocEndLineEdit->setText(settings->value("FocEndLineEdit","110").toString());
    ui->FocStepLineEdit->setText(settings->value("FocStepLineEdit","0.5").toString());
    ui->minMvLineEdit->setText(settings->value("minMvLineEdit","0").toString());
    ui->maxMvLineEdit->setText(settings->value("maxMvLineEdit","10").toString());
    ui->pixSizeLineEdit->setText(settings->value("pixSizeLineEdit","0.00635").toString());
    ui->slideHeightLineEdit->setText(settings->value("slideHeightLineEdit","45.085").toString());
    ui->slideWidthLineEdit->setText(settings->value("slidewidthLineEdit","45.085").toString());
    ui->slidePixHeightLineEdit->setText(settings->value("slidePixHeightLineEdit","7100").toString());
    ui->slidePixWidthLineEdit->setText(settings->value("slidePixwidthLineEdit","7100").toString());
    ui->textLeftLineEdit->setText(settings->value("textLeftLineEdit","150").toString());
    ui->textAtopLineEdit->setText(settings->value("textAtopLineEdit","150").toString());
    ui->textSizeLineEdit->setText(settings->value("textSizeLineEdit","200").toString());
    ui->imageWeightLineEdit->setText(settings->value("imageWeightLineEdit","4").toString());
    ui->imageHeightLineEdit->setText(settings->value("imageHeightLineEdit","4").toString());
    ui->imageSpaceLineEdit->setText(settings->value("imageSpaceLineEdit","200").toString());
    ui->prefixLineEdit->setText(settings->value("prefixLineEdit","Hi").toString());
    ui->suffixLineEdit->setText(settings->value("suffixLineEdit","Bye").toString());
    ui->PicDiamLineEdit->setText(settings->value("PicDiamLineEdit","34").toString());
    ui->checkBox->setChecked(settings->value("checkBox").toBool());
    ui->showPreviewCheckBox->setChecked(settings->value("showPreviewCheckBox").toBool());
    int index = settings->value("comboBoxIndex", 10).toInt();
    ui->comboBox->setCurrentIndex(index);
    ui->GenTabWidget->setCurrentIndex(settings->value("GenTabCurrent","0").toInt());
    ui->AddTabWidget->setCurrentIndex(settings->value("AddTabCurrent","0").toInt());
    ui->slideWidthLineEdit->setReadOnly(settings->value("slidewidthLineEdit_state","false").toBool());
    ui->slidePixWidthLineEdit->setReadOnly(settings->value("slidePixwidthLineEdit_state","false").toBool());
    ui->checkSector->setChecked(settings->value("checkSector").toBool());
    ui->PicDiamLineEdit->setDisabled(settings->value("PicDiamLineEdit_state","true").toBool());
    ui->GridLenghtLineEdit->setText(settings->value("GridLenghtLineEdit","100").toString());
    ui->DistorsioCheck->setChecked(settings->value("DistorsioCheck").toBool());
    ui->pushButton_6->setDisabled(!settings->value("DistorsioCheck").toBool());
    xDistorsioVector = settings->value("xDistorsioVector").value<QList<double> >();
    yDistorsioVector = settings->value("yDistorsioVector").value<QList<double> >();

    if(!xDistorsioVector.size())
    {
        ui->distReadInfoLabel->setText("Коэффициенты не считаны");
    }
    else
    {
        ui->distReadInfoLabel->setText("Коэффициенты считаны");
    }

}

void MainWindow::makeInscription(QSharedPointer<QImage> optimalImage,const QString& setableText,int fontX,int fontY,int fontSize)
{
    // наносим надпись на слайд
    QPainter painter_for_text;
    painter_for_text.begin(optimalImage.data());
    QPen pen_for_text;
    QFont font = painter_for_text.font();
    font.setPixelSize(fontSize);
    painter_for_text.setFont(font);
    painter_for_text.setPen(pen_for_text);
    painter_for_text.setPen(QColor(255,255,255,255));
    painter_for_text.drawText(fontX,fontY,setableText);
    painter_for_text.end();// завершаем нанесение надписи на слайд
}


InscriptParams MainWindow::readInscriptionParams()
{
    InscriptParams data;
    data.fontX = ui->textLeftLineEdit->text().toInt();
    data.fontY = ui->textAtopLineEdit->text().toInt();
    data.fontSize = ui->textSizeLineEdit->text().toInt();
    if(data.fontSize <= 0)
    {
        throw data.fontSize;
    }
    data.prefix = ui->prefixLineEdit->text();
    data.suffix = ui->suffixLineEdit->text();

    return data;

}
StarSlideData MainWindow::readInputStarSlideData()
{
    StarSlideData data;
    data.pointAlpha = ui->alphaLineEdit->text().toDouble();
    if(data.pointAlpha < 0 || data.pointAlpha > 360)
    {
        throw std::invalid_argument("Неверно задан угол альфа");
    }

    data.pointBeta = ui->betaLineEdit->text().toDouble();
    if(data.pointBeta < -90 || data.pointBeta > 90)
    {
        throw std::invalid_argument("Неверно задан угол дельта");
    }

    data.pointAzimut = ui->azLineEdit->text().toDouble();
    if(data.pointAzimut < 0 || data.pointAzimut > 360)
    {
        throw std::invalid_argument("Неверно задан угол азимут");
    }

    data.insideViewAngle = ui->PicDiamLineEdit->text().toDouble();
    if(data.insideViewAngle < 0)
    {
        throw std::invalid_argument("Неверно задано внутреннее поле зрения");
    }

    data.focStart = ui->FocStartLineEdit->text().toDouble();
    if(data.focStart < 0)
    {
        throw std::invalid_argument("Неверно задано фокусное расстояние");
    }

    data.focEnd = ui->FocEndLineEdit->text().toDouble();
    if(data.focEnd < 0 || data.focEnd <= data.focStart)
    {
        throw std::invalid_argument("Неверно задано фокусное расстояние (конец интервала)");
    }


    data.focStep = ui->FocStepLineEdit->text().toDouble();
    if(data.focStep < 0)
    {
        throw std::invalid_argument("Неверно задан шаг по фокуснуму расстоянию");
    }

    data.minMv = ui->minMvLineEdit->text().toDouble();
    data.maxMv = ui->maxMvLineEdit->text().toDouble();
    if(data.maxMv < data.minMv)
    {
        throw std::invalid_argument("Неверно задана максимальная звёздная величина");
    }
    data.pixelPerStar = ui->comboBox->currentIndex();
    data.slideSizeX = ui->slidePixWidthLineEdit->text().toInt();
    if(data.slideSizeX <= 0)
    {
        throw std::invalid_argument("Неверно задан размер слайда по Х");
    }
    data.slideSizeY = ui->slidePixHeightLineEdit->text().toInt();
    if(data.slideSizeY <= 0)
    {
        throw std::invalid_argument("Неверно задан размер слайда по У");
    }
    data.slideSizeMM_Y = ui->slideHeightLineEdit->text().toDouble();
    data.slideSizeMM_X = ui->slideWidthLineEdit->text().toDouble();

    data.pix = ui->pixSizeLineEdit->text().toDouble();
    if(data.pix < 0)
    {
        throw std::invalid_argument("Неверно задан пиксель");
    }

    return data;
}

GroupImageParameters MainWindow::readGroupImageParameters()
{
    GroupImageParameters data;
    if(ui->GroupSlideRadioButton->isChecked())
    {
        data.groupImageHeight = ui->imageHeightLineEdit->text().toDouble();
        data.groupImageWidth = ui->imageWeightLineEdit->text().toDouble();
        data.space = ui->imageSpaceLineEdit->text().toInt();
        if(data.space < 0)
        {
            throw std::invalid_argument("Неверно задано значение пропуска между слайдами");
        }
    }
    return data;
}

GridSlideData MainWindow::readInputGridSlideData()
{
    GridSlideData data;
    data.grid_distance = ui->GridLenghtLineEdit->text().toInt();
    if(data.grid_distance<0)
    {
        throw std::invalid_argument("Неверно задана дистанция сетки");
    }
    data.pixelPerStar = ui->comboBox->currentIndex();
    data.slideSizeX = ui->slidePixWidthLineEdit->text().toInt();
    if(data.slideSizeX <= 0)
    {
        throw std::invalid_argument("Неверно задан размер сетки по Х");
    }
    data.slideSizeY = ui->slidePixHeightLineEdit->text().toInt();
    if(data.slideSizeY <= 0)
    {
        throw std::invalid_argument("Неверно задан размер сетки по У");
    }

    data.pix = ui->pixSizeLineEdit->text().toDouble();
    if(data.pix < 0)
    {
        throw std::invalid_argument("Неверно задан пиксель");
    }

    if(ui->GroupSlideRadioButton->isChecked())
    {
        data.groupImageHeight=ui->imageHeightLineEdit->text().toDouble();
        data.groupImageWidth=ui->imageWeightLineEdit->text().toDouble();
        data.space=ui->imageSpaceLineEdit->text().toInt();
        if(data.space < 0)
        {
            throw std::invalid_argument("Неверно задано значение пропуска между сетками");
        }
    }

    return data;
}


void MainWindow::drawGridSlides(QScopedPointer<QImage> &im, QImage &opt_img, int sz_x, int sz_y, int space, int slideSizeX, int slideSizeY)
{
    QPainter grid_painter;
    grid_painter.begin(im.data());

    for(int curColumn = 0;curColumn < sz_y;curColumn ++)
    {
        for(int curRow = 0;curRow < sz_x;curRow ++)
        {
            grid_painter.drawImage(slideSizeX * curRow + space * curRow, slideSizeY * curColumn + space * curColumn, opt_img);
        }
    }
     grid_painter.end();
    }






    void MainWindow::drawSlide(QScopedPointer <QImage> &im, QVector <QImage> &im_v, int sz_x, int sz_y, int space, int  slideSizeX, int slideSizeY)
    {
        QPainter slide_painter;
        slide_painter.begin(im.data());

        for(int i = 0;i < sz_y;i ++)
        {
            for(int j = 0;j < sz_x;j ++)
            {
                slide_painter.drawImage(slideSizeX * j + space * j,slideSizeY * i + space * i ,im_v[j + i * sz_x]);
                qApp->processEvents();
            }

        }
        slide_painter.end();

    }

    void MainWindow::drawPreviewItem(const QByteArray svg_byteArray,int  slideSizeX,int  slideSizeY)
    {
        svgPreview.reset(new QSvgRenderer(svg_byteArray));
        svgPreviewItem.reset(new QGraphicsSvgItem());
        svgPreviewItem->setSharedRenderer(svgPreview.data());

        if(ui->showPreviewCheckBox->isChecked())
        {
            scene->setSceneRect(0,0,slideSizeX,slideSizeY);
            scene->addItem(svgPreviewItem.data());
            ui->graphicsView->setScene(scene.data());
        }
    }

    void MainWindow::drawPreviewItems(int  slideSizeX,int  slideSizeY,int  groupImageWidth,int  groupImageHeight,int  space)
    {
        scene->setSceneRect(0,0,slideSizeX * groupImageWidth+(space*2) * groupImageWidth,slideSizeY * groupImageHeight+(space*2) * groupImageHeight);
        for(int i = 0;i < groupImageHeight;i ++)
        {
            for(int j = 0;j < groupImageWidth;j ++)
            {
                scene->addItem(vectorOfSvgItems[j+i*groupImageWidth].data());
                vectorOfSvgItems[j + i * groupImageWidth]->setPos(slideSizeX * j + space * j,slideSizeY * i + space * i);
                qApp->processEvents();
            }

        }
    }



    void MainWindow::setUIstate(bool state)
    {
        ui->OneSlideRadioButton->setDisabled(state);
        ui->GroupSlideRadioButton->setDisabled(state);
        ui->pushButton_2->setDisabled(state);
        ui->pushButton_4->setDisabled(state);
        ui->DistorsioCheck->setDisabled(state);
    }

    void MainWindow::updateLineXPix()
    {
        QString updateString;
        int updateBuffer;
        float pix = ui->pixSizeLineEdit->text().toDouble();
        double slideSizeMM_X = ui->slideHeightLineEdit->text().toDouble();
        if(slideSizeMM_X != 0 && pix != 0)
        {
            updateBuffer = slideSizeMM_X / pix;
            updateString = QString::number(updateBuffer);
            ui->slidePixHeightLineEdit->setText(updateString);
        }

    }

    void MainWindow::updateLineYPix()
    {
        QString updateString;
        int updateBuffer;
        float pix = ui->pixSizeLineEdit->text().toDouble();
        double slideSizeMM_Y=ui->slideWidthLineEdit->text().toDouble();
        if(slideSizeMM_Y != 0 && pix != 0)
        {
            updateBuffer = slideSizeMM_Y/pix;
            updateString = QString::number((updateBuffer));
            ui->slidePixWidthLineEdit->setText(updateString);
        }

    }



    void MainWindow::updateLineXMM()

    {
        QString updateString;
        float pix = ui->pixSizeLineEdit->text().toDouble();
        int slideSizeX = ui->slidePixHeightLineEdit->text().toInt();
        if(pix != 0 && slideSizeX != 0)
        {
            updateString = QString::number(slideSizeX * pix);
            ui->slideHeightLineEdit->setText(updateString);
        }
    }





    void MainWindow::updateLineYMM()

    {
        QString updateString;
        float pix = ui->pixSizeLineEdit->text().toDouble();
        int slideSizeY = ui->slidePixWidthLineEdit->text().toInt();
        if(pix != 0 && slideSizeY != 0)
        {
            updateString = QString::number(slideSizeY * pix);
            ui->slideWidthLineEdit->setText(updateString);
        }
    }


    void MainWindow::setLine()
    {
        if(ui->GroupSlideRadioButton->isChecked())
        {
            ui->FocEndLineEdit->setDisabled(false);
            ui->FocStepLineEdit->setDisabled(false);
            ui->imageWeightLineEdit->setDisabled(false);
            ui->imageHeightLineEdit->setDisabled(false);
            ui->imageSpaceLineEdit->setDisabled(false);

        }
        else if(ui->OneSlideRadioButton->isChecked())
        {
            ui->FocEndLineEdit->setDisabled(true);
            ui->FocStepLineEdit->setDisabled(true);
            ui->imageWeightLineEdit->setDisabled(true);
            ui->imageHeightLineEdit->setDisabled(true);
            ui->imageSpaceLineEdit->setDisabled(true);

        }
    }

    void MainWindow::setMatrixState()
    {
        if(ui->checkBox->isChecked())
        {
            ui->slideWidthLineEdit->setReadOnly(true);
            ui->slidePixWidthLineEdit->setReadOnly(true);
            ui->slideWidthLineEdit->setText(ui->slideHeightLineEdit->text());
            ui->slidePixWidthLineEdit->setText(ui->slidePixHeightLineEdit->text());
        }
        else if(!ui->checkBox->isChecked())
        {
            ui->slideWidthLineEdit->setReadOnly(false);
            ui->slidePixWidthLineEdit->setReadOnly(false);
        }

    }





    void MainWindow::setAlgorithmState()
    {
        if(ui->checkSector->isChecked())
        {
            ui->PicDiamLineEdit->setDisabled(true);
        }
        else
        {
            ui->PicDiamLineEdit->setDisabled(false);
        }


    }





    void MainWindow::setDistButtonState()
    {
        if(!ui->DistorsioCheck->isChecked())
        {
            ui->pushButton_6->setDisabled(true);
        }
        else if(ui->DistorsioCheck->isChecked())
        {
            ui->pushButton_6->setDisabled(false);
        }

    }



    void MainWindow::chooseTab()
    {
        if(!ui->GenTabWidget->currentIndex())
        {
            ui->FocEndLineEdit->setDisabled(false);
            ui->FocStartLineEdit->setDisabled(false);
            ui->FocStepLineEdit->setDisabled(false);
            ui->minMvLineEdit->setDisabled(false);
            ui->maxMvLineEdit->setDisabled(false);
        }
        else if(ui->GenTabWidget->currentIndex() == 1)
        {
            ui->FocEndLineEdit->setDisabled(true);
            ui->FocStartLineEdit->setDisabled(true);
            ui->FocStepLineEdit->setDisabled(true);
            ui->minMvLineEdit->setDisabled(true);
            ui->maxMvLineEdit->setDisabled(true);
        }
    }


    void MainWindow::clearSceneAndImages()
    {
        if(ui->showPreviewCheckBox->isChecked())// если отображаем на превью, то очищаем его каждый раз
        {
            if(!scene)
            {
                scene.reset(new QGraphicsScene);
            }
            else
            {
                svgPreview.reset();
                svgPreviewItem.reset();
                vectorOfSvgPreviews.clear();
                vectorOfSvgItems.clear();
                scene->update(scene->sceneRect());
            }

            ui->graphicsView->setScene(scene.data());
        }
        leftTopImage.reset();
        leftDownImage.reset();
        rightDownImage.reset();
        rightTopImage.reset();
        optimalImage.reset();
    }

    void MainWindow::setImagesSizes(int slideSizeX,int slideSizeY,int groupImageHeight ,int groupImageWidth, int space)
    {
        const int imageSizeLimit = 32767;
        const int limitX = imageSizeLimit / slideSizeX;// вычисляем максимально воintсло слайдов в ширинуintизображении
        const int limitY = imageSizeLimit / slideSizeY;// вычисляем максимально возможное число слайдов в высоту на одном изображении
        int imageSizeX = 0;
        int imageSizeY = 0;
        int imageSizeXLeftDown = 0;
        int imageSizeYLeftDown = 0;
        int imageSizeXRightTop = 0;
        int imageSizeYRightTop = 0;


        //размер изображения в зависимости от лимита и размера одного слайда
        if(groupImageWidth <= limitX && groupImageHeight <= limitY) //если желаемая размерность не превышает лимит
        {

            imageSizeX = (slideSizeX * groupImageWidth)+(space*2) * groupImageWidth;
            imageSizeY = (slideSizeY * groupImageHeight)+(space*2) * groupImageHeight;
            leftTopImage.reset(new QImage(imageSizeX, imageSizeY , QImage::Format_Mono));
            leftTopImage->fill(Qt::color1);

        }

        else if(groupImageWidth > limitX && groupImageHeight > limitY) //если желаемая размерность превышает лимит по высоте и ширине
        {
            int imageSizeXRightDown = 0;
            int imageSizeYRightDown = 0;
            imageSizeX = (slideSizeX * limitX) + (space * 2) * limitX;
            imageSizeY = (slideSizeY * limitY) + (space * 2) * limitY;
            imageSizeXLeftDown = imageSizeX;
            imageSizeYLeftDown = imageSizeY;
            imageSizeXRightTop = imageSizeX;
            imageSizeYRightTop = imageSizeY;

            /* Если ширина и высота больше, чем могут вместить 4 изображение, то последнему так же присваеваем максимально
         * возможный размер, игнорируя превышение */
            if((groupImageWidth-limitX) < limitX && (groupImageHeight-limitY) < limitY)
            {
                imageSizeXRightDown = slideSizeX * (groupImageWidth-limitX) + (space*2) * (groupImageWidth - limitX);
                imageSizeYRightDown = slideSizeY * (groupImageHeight-limitY) + (space*2) * (groupImageHeight - limitY);
            }
            else
            {
                imageSizeXRightDown=imageSizeX;
                imageSizeYRightDown=imageSizeY;
            }


            leftTopImage.reset(new QImage(imageSizeX,imageSizeY, QImage::Format_Mono));
            leftTopImage->fill(Qt::color1);
            leftDownImage.reset(new QImage(imageSizeXLeftDown,imageSizeYLeftDown, QImage::Format_Mono));
            leftDownImage->fill(Qt::color1);
            rightTopImage.reset(new QImage(imageSizeXRightTop,imageSizeYRightTop, QImage::Format_Mono));
            rightTopImage->fill(Qt::color1);
            rightDownImage.reset(new QImage(imageSizeXRightDown,imageSizeYRightDown, QImage::Format_Mono));
            rightDownImage->fill(Qt::color1);


        }
        else if(groupImageWidth>limitX) //если желаемая размерность превышает лимит по ширине
        {
            imageSizeX = (slideSizeX * limitX) + (space * 2) * limitX;
            imageSizeY = (slideSizeY * groupImageHeight) + (space * 2) * groupImageHeight;

            /* Если ширина больше, чем может вместить правое изображение, то ему так же присваеваем максимально
         * возможный размер, игнорируя превышение */
            if(groupImageWidth - limitX < limitX)
            {
                imageSizeXRightTop = slideSizeX * (groupImageWidth - limitX)+(space*2)*(groupImageWidth - limitX);
                imageSizeYRightTop = slideSizeY * groupImageHeight+(space*2)*groupImageHeight;
            }
            else
            {
                imageSizeXRightTop = imageSizeX;
                imageSizeYRightTop = imageSizeY;
            }


            leftTopImage.reset(new QImage(imageSizeX,imageSizeY, QImage::Format_Mono));
            leftTopImage->fill(Qt::color1);
            rightTopImage.reset(new QImage(imageSizeXRightTop,imageSizeYRightTop, QImage::Format_Mono));
            rightTopImage->fill(Qt::color1);


        }
        else if(groupImageHeight > limitY)  //если желаемая размерность превышает лимит по высоте
        {
            imageSizeX = (slideSizeX * groupImageWidth) + (space * 2) * groupImageWidth;
            imageSizeY = (slideSizeY * limitY) + (space*2) * limitY;

            /* Если высота больше, чем может вместить нижнее изображение, то ему так же присваеваем максимально
         * возможный размер, игнорируя превышение */
            if(groupImageHeight - limitY < limitY)
            {
                imageSizeXLeftDown = slideSizeX * groupImageWidth + (space*2) * groupImageWidth;
                imageSizeYLeftDown = slideSizeY * (groupImageHeight - limitY) + (space*2)*(groupImageHeight - limitY);
            }
            else
            {
                imageSizeXLeftDown = imageSizeX;
                imageSizeYLeftDown = imageSizeY;
            }

            leftTopImage.reset(new QImage(imageSizeX,imageSizeY, QImage::Format_Mono));
            leftTopImage->fill(Qt::color1);
            leftDownImage.reset(new QImage(imageSizeXLeftDown, imageSizeYLeftDown, QImage::Format_Mono));
            leftDownImage->fill(Qt::color1);

        }
    }

    void MainWindow::openCatalog()
    {

        QVector <double> alphaVec;
        QVector <double> betaVec;
        QVector <float> mvVec;
        QVector <double> alphaVecSec;
        QVector <double> betaVecSec;
        QVector <long> countSecVec;
        QVector <long> shiftVec;
        QVector<short> newNumn;


        data_star generalCat;
        sector secCat;
        QVector <sector> secVec;
        QVector <data_star> dataStarVec;
        QString bufFilename;
        QString filenameAdd;
        numbers number;

        constexpr  static double transToGrad = 57.29577957855229;
        constexpr  static double div = 0.00000001;
        constexpr  static int structSize = 18;
        catalogIsRead = false;

        if(sizeof(generalCat) != structSize)
        {
            QMessageBox::information(NULL,"Ошибка","Размер структуры не соответствует заданному. Обратитесь к разработчику");
            exit(1);
        }
        ifstream in(filename.toLocal8Bit().constData(),ios::binary);
        if(in.is_open())
        {
            while(in.read((char*)&generalCat,sizeof(generalCat)))// считываем каталог в вектор структур
            {
                dataStarVec.append(generalCat);
            }
            in.close();
        }
        else
        {
            QMessageBox::critical(NULL,"Ошибка","Ошибка открытия файла");
            ui->catalogStatusLabel->setText("Ошибка");
            return;
        }

        for (QVector<data_star>::iterator it = dataStarVec.begin();it != dataStarVec.end();it ++)// расшифровываем считанный каталог
        {
            alphaVec.append((it->alpha) * div * (transToGrad));
            betaVec.append((it->beta) * div * (transToGrad));
            mvVec.append(((it->mv) - 20));
        }
        dataStarVec.clear();

        for(int i = 0;i < mvVec.size();i ++)
        {
            mvVec[i] = mvVec[i] / 10;
        }


        bufFilename = filename;
        filenameAdd = bufFilename.remove(filename.lastIndexOf("."),filename.end() - filename.begin());
        filenameAdd.append("_SEC.CAT");
        in.open(filenameAdd.toLocal8Bit().constData(),ios::binary);
        if(in.is_open())
        {
            while(in.read((char*)&secCat,sizeof(sector)))// считываем каталог в вектор структур
            {
                secVec.append(secCat);
            }
            in.close();
        }
        else
        {
            QMessageBox::critical(NULL,"Ошибка","Ошибка открытия файла");
            ui->catalogStatusLabel->setText("Ошибка");
            return;
        }

        for (QVector<sector>::iterator it = secVec.begin();it != secVec.end();it ++)
        {
            alphaVecSec.append((it->alpha_c)*(transToGrad));
            betaVecSec.append((it->beta_c)*(transToGrad));
            countSecVec.append(it->count_in_sector);
            shiftVec.append(it->shift);
        }
        secVec.clear();


        filenameAdd = bufFilename.remove(filename.lastIndexOf("."),filename.end()-filename.begin());
        filenameAdd.append("_NUM.CAT");
        in.open(filenameAdd.toLocal8Bit().constData(),ios::binary);
        if(in.is_open())
        {
            while(in.read((char*)&number.num,sizeof(number.num)))// считываем каталог в вектор
            {
                newNumn.append(number.num);
            }
            in.close();
        }
        else
        {
            QMessageBox::critical(NULL,"Ошибка","Ошибка открытия файла");
            ui->catalogStatusLabel->setText("Ошибка");
            return;
        }

        for(QVector<short>::iterator it = newNumn.begin();it != newNumn.end();it++)
        {
            if(*it < 0)
            {
                *it = *it*(-1);// избавляемся от отрицательных значений
            }
        }
        for(int i = 0;i <= newNumn.size() - 1;i ++)
        {
            newNumn[i] = newNumn[i] - 1;
        }

        QString countOfStar(QString::number(alphaVec.size()));
        ui->catalogStatusLabel->setText(countOfStar);

        catalogData.alpha_vec = alphaVec;
        catalogData.beta_vec = betaVec;
        catalogData.alpha_vec_sec = alphaVecSec;
        catalogData.beta_vec_sec = betaVecSec;
        catalogData.mv_vec = mvVec;
        catalogData.new_numn = newNumn;
        catalogData.shift_vec = shiftVec;
        catalogData.count_sec_vec = countSecVec;

        catalogIsRead = true;
    }





    void MainWindow::chooseCatalog()
    {
        if(lastCatalogDirectory.isNull())
        {
            filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open .CAT"), ".",
                                                    tr(".CAT files (*.CAT)"));
            ui->filenameLineEdit->setText(filename);
            int pos = filename.lastIndexOf("/");
            lastCatalogDirectory = filename.left(pos);
        }
        else
        {
            filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open .CAT"), lastCatalogDirectory,
                                                    tr(".CAT files (*.CAT)"));
            ui->filenameLineEdit->setText(filename);

        }
        openCatalog();

    }



    void MainWindow::useCurrentCatalog()
    {
        openCatalog();
    }



    void MainWindow::chooseDistorsioFile()
    {
        xDistorsioVector.clear();
        yDistorsioVector.clear();

        if(lastDistorsioDirectory.isNull())
        {
            distorsioFilename = QFileDialog::getOpenFileName(this,
                                                             tr("Open .txt"), ".",
                                                             tr(".txt files (*.txt)"));

            int pos = distorsioFilename.lastIndexOf("/");
            lastDistorsioDirectory = distorsioFilename.left(pos);

        }
        else
        {
            distorsioFilename = QFileDialog::getOpenFileName(this,
                                                             tr("Open .txt"), lastDistorsioDirectory,
                                                             tr(".txt files (*.txt)"));
        }
        QFile file(distorsioFilename);
        QTextStream in(&file);
        in.setRealNumberNotation(QTextStream::ScientificNotation);
        if(file.open(QIODevice::ReadOnly |QIODevice::Text))
        {
            double x, y;
            while(!(in >> x >> y).atEnd())
            {
                xDistorsioVector.append(x);
                yDistorsioVector.append(y);
            }
        }
        else
        {
            QMessageBox::critical(NULL,"Ошибка","Ошибка открытия файла");
            ui->distReadInfoLabel->setText("Коэффициенты не считаны");
            ui->DistorsioPathLineEdit->setText(" ");
            file.close();
            distorsioIsRead = false;
            return;
        }
        file.close();
        constexpr int maxCoefsCount = 21;
        while(xDistorsioVector.size() != maxCoefsCount)
        {
            xDistorsioVector.append(0);
            yDistorsioVector.append(0);
        }

        ui->DistorsioPathLineEdit->setText(distorsioFilename);
        ui->distReadInfoLabel->setText("Коэффициенты считаны");
        distorsioIsRead = true;

    }



    QByteArray MainWindow::createPreviewImage(const int imageWidth, const int iamgeHeight, const int fontSize, const int fontX, const int fontY, const QString setableText, QVector<StarParameters> coordinatesOfStars)
    {

        QDomDocument doc("pic");
        QDomElement svg = doc.createElement("svg");
        QDomElement cube = doc.createElement("rect");
        svg.setAttribute("viewBox",QString("0 ")+"0 "+QString::number(imageWidth)+" "+QString::number(iamgeHeight));
        svg.setAttribute("xmlns","http://www.w3.org/2000/svg");
        svg.setAttribute("encoding","UTF-8");
        svg.setAttribute("width",QString::number(imageWidth));
        svg.setAttribute("height",QString::number(iamgeHeight));
        cube.setAttribute("x","0");
        cube.setAttribute("y","0");
        cube.setAttribute("width",QString::number(imageWidth));
        cube.setAttribute("height",QString::number(iamgeHeight));
        cube.setAttribute("fill","black");
        doc.appendChild(svg);
        svg.appendChild(cube);
        for(auto& starCoordinate : coordinatesOfStars)
        {
            QDomElement star = doc.createElement("rect");
            star.setAttribute("x",QString::number(starCoordinate.x));
            star.setAttribute("fill","white");
            star.setAttribute("y",QString::number(starCoordinate.y));
            star.setAttribute("width",QString::number(starCoordinate.sizeX));
            star.setAttribute("height",QString::number(starCoordinate.sizeY));
            svg.appendChild(star);
        }
        QDomElement textElement = doc.createElement("text");
        QDomText text = doc.createTextNode(setableText);
        textElement.setAttribute("x",QString::number(fontX));
        textElement.setAttribute("y",QString::number(fontY));
        textElement.setAttribute("style","fill: white;");
        textElement.setAttribute("font-size",QString::number(fontSize));
        textElement.appendChild(text);
        svg.appendChild(textElement);
        QByteArray svgImage = doc.toByteArray();
        return svgImage;
    }



    void MainWindow::createImage()
    {

        if(!catalogIsRead)// если каталог не считан
        {
            QMessageBox::warning(NULL,"Ошибка","Выберите каталог или подтвердите использование текущего");
            return;
        }

        if(ui->DistorsioCheck->isChecked() && !distorsioIsRead)
        {
            QMessageBox::warning(NULL,"Ошибка","Выберите файл с коэффициентами дисторсии");
            return;
        }

        ui->progressBar->setValue(0);

        try
        {
            setUIstate(true);
            clearSceneAndImages();
            StarSlideData slideData(readInputStarSlideData());
            GroupImageParameters groupImgData (readGroupImageParameters());
            InscriptParams inscriptData(readInscriptionParams());
            QScopedPointer<SlideCreator> slideCreator(new SlideCreator());
            slideCreator->calculateAngularDistOptions(slideData,catalogData,ui->checkSector->isChecked());


            if(ui->OneSlideRadioButton->isChecked())
            {
                ui->progressBar->setRange(0, 100);
                DistorsioData distData;
                if(ui->DistorsioCheck->isChecked())
                {
                    distData.xDistorsioVector = xDistorsioVector;
                    distData.yDistorsioVector = yDistorsioVector;
                }

                SlideParameters imgData = slideCreator->createStarSlide(slideData.focStart,ui->checkSector->isChecked(),ui->DistorsioCheck->isChecked(),distData);
                optimalImage = slideCreator->getSlidePointer();
                ui->progressBar->setValue(50);

                QString setableText = inscriptData.prefix + " "+ "f=" + QString::number(slideData.focStart)+"mm"+","+" "
                        +QString::number(imgData.count_of_stars)+"zv." + "," + " " + " cat."+" "
                        +QString::number(catalogData.alpha_vec.size())+","+" " + ui->comboBox->currentText()+","
                        +" "+QChar(0x03B1) + "=" + QString::number(slideData.pointAlpha) + QChar(0x00B0) + ","
                        +" "+QChar(0x03B4) + "=" + QString::number(slideData.pointBeta) + QChar(0x00B0) + ","+" "
                        +"2w= " + QString::number(imgData.view_angle_x)+"x"
                        +QString::number(imgData.view_angle_y)+" "+inscriptData.suffix;


                makeInscription(optimalImage,setableText,inscriptData.fontX,inscriptData.fontY,inscriptData.fontSize);

                if(ui->showPreviewCheckBox->isChecked())
                {
                    QByteArray previewImageData =
                            createPreviewImage(slideData.slideSizeX,slideData.slideSizeY,inscriptData.fontSize,inscriptData.fontX,inscriptData.fontY,setableText,imgData.coordinatesOfStars);
                    drawPreviewItem(previewImageData,slideData.slideSizeX,slideData.slideSizeY);
                }
                ui->progressBar->setValue(100);
            }


            else if(ui->GroupSlideRadioButton->isChecked())
            {
                // проверка соответствия длины массива фокусных расстояний размерности матрицы
                if ((groupImgData.groupImageWidth * groupImgData.groupImageHeight) < ((slideData.focEnd - slideData.focStart) / slideData.focStep) + 1)
                {

                    QMessageBox msgBox;
                    msgBox.setText("Внимание!");
                    msgBox.setInformativeText("Заданное число изображений меньше интервала по фокусному расстоянию");
                    msgBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Retry);
                    int ret = msgBox.exec();
                    switch (ret) {
                    case QMessageBox::Ignore:
                        break;
                    case QMessageBox::Retry:
                        setUIstate(false);
                        return;
                    default:

                        break;
                    }
                }

                if ((groupImgData.groupImageWidth * groupImgData.groupImageHeight) > ((slideData.focEnd - slideData.focStart) / slideData.focStep) + 1)
                {
                    QMessageBox msgBox;
                    msgBox.setText("Внимание!");
                    msgBox.setInformativeText("Заданное число изображений больше интервала по фокусному расстоянию");
                    msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Ignore );
                    int ret = msgBox.exec();
                    switch (ret) {
                    case QMessageBox::Ignore:
                        break;
                    case QMessageBox::Retry:
                        setUIstate(false);
                        return;
                    default:

                        break;
                    }

                }

                QVector <float> focusVector;
                QVector <QImage> imageVector;
                QVector <QImage> leftDownVector;
                QVector <QImage> rightTopVector;
                QVector <QImage> rightDownVector;
                QSharedPointer <QImage> bufImage;


                const int imageSizeLimit = 32767;
                int limitX = imageSizeLimit / slideData.slideSizeX;// вычисляем максимально возможное число слайдов в ширину на одном изображении
                int limitY = imageSizeLimit / slideData.slideSizeY;// вычисляем максимально возможное число слайдов в высоту на одном изображении
                setImagesSizes(slideData.slideSizeX, slideData.slideSizeY, groupImgData.groupImageHeight, groupImgData.groupImageWidth, groupImgData.space);



                for(float i = slideData.focStart;i <= slideData.focEnd;i += slideData.focStep) // формируем вектор фокусных расстояний
                {
                    focusVector.append(i);
                }
                // если размерность матрицы больше длины массива фок.расстояний, дозабиваем массив значениями из его
                // начала

                int difference = (groupImgData.groupImageWidth * groupImgData.groupImageHeight)-(((slideData.focEnd - slideData.focStart) / slideData.focStep) + 1);
                if(difference >= 0)
                {
                    for(int i = 0; i <= difference;i ++)
                    {
                        focusVector.append(focusVector[i]);
                    }

                }

                ui->progressBar->setRange(0, groupImgData.groupImageHeight * groupImgData.groupImageWidth);

                DistorsioData distData;
                if(ui->DistorsioCheck->isChecked())
                {
                    distData.xDistorsioVector = xDistorsioVector;
                    distData.yDistorsioVector = yDistorsioVector;
                }

                QVector<float>::const_iterator focusValue = focusVector.begin();
                for(int imgHeight = 0; imgHeight < groupImgData.groupImageHeight;imgHeight ++)
                {

                    for(int imWidth = 0; imWidth < groupImgData.groupImageWidth;imWidth ++)
                    {

                        ui->progressBar->setValue(imWidth + imgHeight * groupImgData.groupImageWidth + 1);
                        qDebug() << *focusValue;
                        SlideParameters imgData =
                                slideCreator->createStarSlide(*focusValue, ui->checkSector->isChecked(), ui->DistorsioCheck->isChecked(), distData);

                        bufImage = slideCreator->getSlidePointer();
                        QString setableText = inscriptData.prefix+" "+"f="+QString::number(*focusValue)+ "mm" + "," + " "
                                +QString::number(imgData.count_of_stars)+"zv."+","+" "+" cat."+" "
                                +QString::number(catalogData.alpha_vec.size())+","+" "+ui->comboBox->currentText()+","
                                +" "+QChar(0x03B1)+"="+QString::number(slideData.pointAlpha)+QChar(0x00B0)+","
                                +" "+QChar(0x03B4)+"="+QString::number(slideData.pointBeta)+QChar(0x00B0)+","+" "
                                +"2w= " +QString::number(imgData.view_angle_x)+"x"
                                +QString::number(imgData.view_angle_y)+" "+inscriptData.suffix; // ПРОВЕРИТЬ
                        makeInscription(bufImage,setableText,inscriptData.fontX,inscriptData.fontY,inscriptData.fontSize);


                        if(ui->showPreviewCheckBox->isChecked()) //если используем превью
                        {
                            QByteArray previewImageData =
                                    createPreviewImage(slideData.slideSizeX,slideData.slideSizeY,inscriptData.fontSize,inscriptData.fontX,inscriptData.fontY,setableText,imgData.coordinatesOfStars);
                            QSharedPointer<QSvgRenderer> temporarySvg(new QSvgRenderer(previewImageData));
                            vectorOfSvgPreviews.append(temporarySvg);
                            QSharedPointer <QGraphicsSvgItem> temporarySvgItem(new QGraphicsSvgItem);
                            temporarySvgItem->setSharedRenderer(vectorOfSvgPreviews.last().data());
                            vectorOfSvgItems.append(temporarySvgItem);
                        }

                        // расфасовываем слайды по изображениям
                        if(imgHeight <= limitY - 1 && imWidth <= limitX - 1) // если текущая ширина и высота не превышают лимит
                        {
                            imageVector.append(*bufImage);
                        }

                        else if(imgHeight > limitY - 1 && imWidth > limitX - 1) // если текущая ширина и высота превышают лимит
                        {
                            rightDownVector.append(*bufImage);
                        }
                        else if(imWidth > limitX - 1) // если текущая ширина превышает лимит
                        {
                            rightTopVector.append(*bufImage);
                        }

                        else if(imgHeight > limitY - 1) // если текущая высота превышает лимит
                        {
                            leftDownVector.append(*bufImage);
                        }
                        bufImage.reset();
                        ++focusValue;

                    }
                }

                if(leftTopImage)
                {

                    int sizeY = leftTopImage->height()/(slideData.slideSizeY + groupImgData.space * 2);
                    int sizeX = leftTopImage->width()/(slideData.slideSizeX + groupImgData.space * 2);
                    drawSlide(leftTopImage,imageVector,sizeX,sizeY,groupImgData.space, slideData.slideSizeX,slideData.slideSizeY);

                }

                if(leftDownImage)
                {
                    int sizeY = leftDownImage->height()/(slideData.slideSizeY + groupImgData.space * 2);
                    int sizeX = leftDownImage->width()/(slideData.slideSizeX + groupImgData.space * 2);
                    drawSlide(leftDownImage,leftDownVector,sizeX,sizeY,groupImgData.space,slideData.slideSizeX,slideData.slideSizeY);

                }

                if(rightTopImage)
                {
                    int sizeY = rightTopImage->height()/(slideData.slideSizeY+groupImgData.space * 2);
                    int sizeX = rightTopImage->width()/(slideData.slideSizeX+groupImgData.space * 2);
                    drawSlide(rightTopImage,rightTopVector,sizeX,sizeY,groupImgData.space, slideData.slideSizeX,slideData.slideSizeY);

                }

                if(rightDownImage)
                {

                    int sizeY = rightDownImage->height()/(slideData.slideSizeY + groupImgData.space * 2);
                    int sizeX = rightDownImage->width()/(slideData.slideSizeX + groupImgData.space * 2);
                    drawSlide(rightDownImage,rightDownVector,sizeX,sizeY,groupImgData.space, slideData.slideSizeX,slideData.slideSizeY);
                }

                if(ui->showPreviewCheckBox->isChecked())
                {
                    drawPreviewItems(slideData.slideSizeX, slideData.slideSizeY, groupImgData.groupImageWidth, groupImgData.groupImageHeight, groupImgData.space);
                }

            }
            setUIstate(false);
        }

        catch(exception &e)
        {
            setUIstate(false);
            QMessageBox::information(NULL,"Ошибка",e.what());
            return;
        }


    }




    void MainWindow::createGrid()
    {

        if(ui->DistorsioCheck->isChecked() && !distorsioIsRead)
        {
            QMessageBox::warning(NULL,"Ошибка","Выберите файл с коэффициентами дисторсии");
            return;
        }
        try
        {

            ui->progressBar->setValue(0);
            setUIstate(true);
            clearSceneAndImages();

            QScopedPointer <SlideCreator> slideCreator(new SlideCreator());
            GridSlideData gridData(readInputGridSlideData());
            InscriptParams inscriptData(readInscriptionParams());

            DistorsioData distData;
            distData.xDistorsioVector=xDistorsioVector;
            distData.yDistorsioVector=yDistorsioVector;
            if(ui->OneSlideRadioButton->isChecked())
            {

                QVector<StarParameters> coordinatesOfStars = slideCreator->createGridSlide(gridData,ui->DistorsioCheck->isChecked(),distData);
                optimalImage = slideCreator->getSlidePointer();
                QString textForGrid=inscriptData.prefix+" "+"Разм.пикселя:"+" "+ui->comboBox->currentText()+","+" "+
                        "Расстояние,пикс.:"+" "+QString::number(gridData.grid_distance)+" "+inscriptData.suffix;
                makeInscription(optimalImage,textForGrid,inscriptData.fontX,inscriptData.fontY,inscriptData.fontSize);
                QByteArray previewImageData=createPreviewImage(gridData.slideSizeX,gridData.slideSizeY,inscriptData.fontSize,inscriptData.fontX,inscriptData.fontY,textForGrid,coordinatesOfStars);

                if(ui->showPreviewCheckBox->isChecked())
                {
                    drawPreviewItem(previewImageData,gridData.slideSizeX,gridData.slideSizeY);
                }
                ui->progressBar->setValue(100);
            }

            else if(ui->GroupSlideRadioButton->isChecked())
            {
                setImagesSizes(gridData.slideSizeX,gridData.slideSizeY,gridData.groupImageHeight,gridData.groupImageWidth,gridData.space);
                ui->progressBar->setRange(0,100);
                QSharedPointer<QImage>bufImage;

                QVector<StarParameters> coordinatesOfStars = slideCreator->createGridSlide(gridData,ui->DistorsioCheck->isChecked(),distData);
                bufImage = slideCreator->getSlidePointer();
                QString textForGrid = inscriptData.prefix+" "+"Size,pix:"+" "+ui->comboBox->currentText()+","+" "+
                        "Distance,pix.:"+" "+QString::number(gridData.grid_distance)+" "+inscriptData.suffix;
                makeInscription(bufImage,textForGrid,inscriptData.fontX,inscriptData.fontY,inscriptData.fontSize);

                if(ui->showPreviewCheckBox->isChecked()) //если используем превью
                {
                    QByteArray previewImageData =
                            createPreviewImage(gridData.slideSizeX,gridData.slideSizeY,inscriptData.fontSize,inscriptData.fontX,inscriptData.fontY,textForGrid,coordinatesOfStars);
                    QSharedPointer<QSvgRenderer> temporarySvg(new QSvgRenderer(previewImageData));


                    for(int imgHeight = 0;imgHeight < gridData.groupImageHeight;imgHeight ++)
                    {
                        for( int imWidth = 0;imWidth < gridData.groupImageWidth;imWidth ++)
                        {
                            vectorOfSvgPreviews.append(temporarySvg);
                            QSharedPointer <QGraphicsSvgItem> temporarySvgItem(new QGraphicsSvgItem);
                            temporarySvgItem->setSharedRenderer(vectorOfSvgPreviews.last().data());
                            vectorOfSvgItems.append(temporarySvgItem);
                            qApp->processEvents();
                        }
                    }
                }
                ui->progressBar->setValue(50);
                if (leftTopImage)
                {
                    int sizeY = leftTopImage->height()/(gridData.slideSizeY+gridData.space*2);
                    int sizeX = leftTopImage->width()/(gridData.slideSizeX+gridData.space*2);
                    drawGridSlides(leftTopImage,*bufImage,sizeX,sizeY,gridData.space, gridData.slideSizeX,gridData.slideSizeY);

                }
                if(leftDownImage)// ЗАПОЛНЯЕМ ДОПОЛНИТЕЛЬНЫЕ ИЗОБРАЖЕНИЯ
                {

                    int sizeY=leftDownImage->height()/(gridData.slideSizeY+gridData.space*2);
                    int sizeX=leftDownImage->width()/(gridData.slideSizeX+gridData.space*2);
                    drawGridSlides(leftDownImage,*bufImage,sizeX,sizeY,gridData.space, gridData.slideSizeX,gridData.slideSizeY);

                }

                if(rightTopImage) // ЗАПОЛНЯЕМ ДОПОЛНИТЕЛЬНЫЕ ИЗОБРАЖЕНИЯ

                {
                    int sizeY=rightTopImage->height()/(gridData.slideSizeY+gridData.space*2);
                    int sizeX=rightTopImage->width()/(gridData.slideSizeX+gridData.space*2);
                    drawGridSlides(rightTopImage,*bufImage,sizeX,sizeY,gridData.space, gridData.slideSizeX,gridData.slideSizeY);

                }

                if(rightDownImage) // ЗАПОЛНЯЕМ ДОПОЛНИТЕЛЬНЫЕ ИЗОБРАЖЕНИЯ
                {
                    int sizeY=rightDownImage->height()/(gridData.slideSizeY+gridData.space*2);
                    int sizeX=rightDownImage->width()/(gridData.slideSizeX+gridData.space*2);
                    drawGridSlides(rightDownImage,*bufImage,sizeX,sizeY,gridData.space, gridData.slideSizeX,gridData.slideSizeY);
                }

                ui->progressBar->setValue(100);
                if(ui->showPreviewCheckBox->isChecked())
                {
                    drawPreviewItems(gridData.slideSizeX,gridData.slideSizeY,gridData.groupImageWidth,gridData.groupImageHeight,gridData.space);
                }

            }
            setUIstate(false);
        }

        catch(exception &e)
        {
            setUIstate(false);
            QMessageBox::information(NULL,"Ошибка",e.what());
            return;
        }

    }






    void MainWindow::testForSlide()
    {
        //QMessageBox::information(NULL,"Сохранение",QString::number(ui->frame->width()));
        if(!catalogIsRead)// если каталог не считан
        {
            QMessageBox::warning(NULL,"Ошибка","Выберите каталог или подтвердите использование текущего");
            return;
        }
        else
        {
            try
            {
                QScopedPointer<SlideCreator> slideCreator(new SlideCreator());
                StarSlideData slideData = readInputStarSlideData();
                slideCreator->calculateAngularDistOptions(slideData,catalogData,ui->checkSector->isChecked());

                DistorsioData distData;
                distData.xDistorsioVector = xDistorsioVector;
                distData.yDistorsioVector = yDistorsioVector;


                TestSlideParameters testData=slideCreator->testStarSlide(ui->checkSector->isChecked(),ui->DistorsioCheck->isChecked(),distData);
                QString message;

                if(ui->OneSlideRadioButton->isChecked())
                {
                    message = QString("Число звёзд на слайде :")+" "+QString::number(testData.count_of_stars)
                            +"\n"+ "Фокусное расстояние :"+" "+QString::number(slideData.focStart)
                            +"\n"+ "Угол зрения слайда ШхВ, град :"+" "+QString::number(testData.view_angle_x)+"x"+QString::number(testData.view_angle_y)
                            +"\n"+ "Размер слайда пикс. :"+" "+QString::number(slideData.slideSizeX)+"x"+QString::number(slideData.slideSizeY)
                            +"\n"+ "Размер слайда мм. :"+" "+QString::number(slideData.slideSizeMM_X)+"x"+QString::number(slideData.slideSizeMM_Y);

                }

                else if(ui->GroupSlideRadioButton->isChecked())
                {
                    GroupImageParameters groupImageParams(readGroupImageParameters());
                    message=QString("ШхВ :")+" "+QString::number(groupImageParams.groupImageWidth) + "x" + QString::number(groupImageParams.groupImageHeight)+
                            "\n"+"Число cлайдов :"+" "+QString::number(groupImageParams.groupImageWidth * groupImageParams.groupImageHeight)+
                            "\n"+"Число фокусных расстояний :"+" "+QString::number(((slideData.focEnd - slideData.focStart)/slideData.focStep) + 1)+
                            "\n"+"Размер мм. :"+" "+QString::number(slideData.slideSizeMM_X * groupImageParams.groupImageWidth)+
                            "x"+QString::number(slideData.slideSizeMM_Y * groupImageParams.groupImageHeight)+
                            "\n"+"Размер пикс. :"+" "+QString::number(slideData.slideSizeX * groupImageParams.groupImageWidth)+
                            "x"+QString::number(slideData.slideSizeY*groupImageParams.groupImageHeight)
                            +"\n"+ "Фокусное расстояние первого слайда :"+" "+QString::number(slideData.focStart)
                            +"\n"+ "Размер первого слайда мм. :"+" "+QString::number(slideData.slideSizeMM_X) + "x" + QString::number(slideData.slideSizeMM_Y)
                            +"\n"+ "Размер первого слайда пикс. :"+" "+QString::number(slideData.slideSizeX) + "x" + QString::number(slideData.slideSizeY)
                            +"\n"+ "Угол зрения первого слайда ШхВ, град :"+" "+QString::number(testData.view_angle_x)+"x" + QString::number(testData.view_angle_y)
                            +"\n"+ "Число звезд на первом слайде :"+" "+QString::number(testData.count_of_stars);


                }
                QMessageBox::information(NULL,"Тест",message);


            }

            catch(exception &e)
            {
                QMessageBox::information(NULL,"Ошибка",e.what());
                return;
            }

        }
    }


    void MainWindow::saveImage()
    {
        QTime currentTime;


        QString filenameSave = QFileDialog::getSaveFileName(this,
                                                            tr("Save tiff"), ".",
                                                            tr("tiff files (*.tiff)"));
        QImageWriter writer;
        writer.setFormat("tiff");
        writer.setFileName(filenameSave);
        if(ui->OneSlideRadioButton->isChecked())
        {
            if(!optimalImage.isNull())
            {
                if(!writer.write(*optimalImage))
                {
                    QMessageBox::information(NULL,"Ошибка",writer.errorString());
                    return;
                }
                optimalImage.reset();
            }
            else
            {
                QMessageBox::information(NULL,"Ошибка","Нечего сохранять");
                return;
            }
        }
        else if(ui->GroupSlideRadioButton->isChecked())
        {
            if(!leftTopImage.isNull())
            {

                if(!writer.write(*leftTopImage))
                {

                    QMessageBox::information(NULL,"Ошибка",writer.errorString());
                    return;
                }

                leftTopImage.reset();
            }
            else
            {
                QMessageBox::information(NULL,"Ошибка","Нечего сохранять");
                return;
            }
            if(!leftDownImage.isNull())
            {

                QString filenameSaveAdd = filenameSave.remove(filenameSave.lastIndexOf("/") + 1,filenameSave.end() - filenameSave.begin());
                filenameSaveAdd.append(currentTime.currentTime().toString("hh_mm_ss")+"left_down.tiff");
                writer.setFileName(filenameSaveAdd);
                if(!writer.write(*leftDownImage))
                {
                    QMessageBox::information(NULL,"Ошибка",writer.errorString());
                    return;
                }
                leftDownImage.reset();


            }

            if(!rightTopImage.isNull())
            {

                QString filenameSaveAdd = filenameSave.remove(filenameSave.lastIndexOf("/") + 1,filenameSave.end() - filenameSave.begin());
                filenameSaveAdd.append(currentTime.currentTime().toString("hh_mm_ss") + "right_top.tiff");
                writer.setFileName(filenameSaveAdd);
                if(!writer.write(*rightTopImage))
                {
                    QMessageBox::information(NULL,"Ошибка",writer.errorString());
                    return;
                }
                rightTopImage.reset();
            }

            if(!rightDownImage.isNull())
            {

                QString filenameSaveAdd = filenameSave.remove(filenameSave.lastIndexOf("/")+1,filenameSave.end()-filenameSave.begin());
                filenameSaveAdd.append(currentTime.currentTime().toString("hh_mm_ss") + "right_down.tiff");
                writer.setFileName(filenameSaveAdd);
                if(!writer.write(*rightDownImage))
                {
                    QMessageBox::information(NULL,"Ошибка",writer.errorString());
                    return;
                }
                rightDownImage.reset();
            }
        }
        QMessageBox::information(NULL,"Сохранение","Успешно сохранено");
    }




    bool MainWindow::eventFilter(QObject *object, QEvent *event)
    {

        if (object == ui->graphicsView->viewport() && event->type() == QEvent::Wheel) {
            return true;
        }
        return false;
    }

    void MainWindow::wheelEvent(QWheelEvent *event){

        QPoint cPos = ui->graphicsView->pos();
        if(QCursor::pos().x() > cPos.x())
        {
            ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            // шкала
            double scaleFactor = 1.15;
            if(event->delta() > 0)
            {
                // Приближение
                ui->graphicsView-> scale(scaleFactor, scaleFactor);

            } else
            {
                //Отдаление
                ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
            }
        }
    }


    MainWindow::~MainWindow()
    {
        saveSettings();
        delete ui;
    }

