#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <QDebug>
#include <QtGlobal>



Q_DECLARE_METATYPE(QList<double>)
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStyle();
    qRegisterMetaTypeStreamOperators <QList<double> >("QList<double>");
    settings = new QSettings(QDir::currentPath() + "/" + "config.ini", QSettings::IniFormat, this);
    initWidgetsOptionConnections();
    loadSettings();
}

void MainWindow::initWidgetsOptionConnections()
{
    QStringList pixperstar;
    pixperstar<<"Выберите..."<<"1x1"<<"2x2"<<"3x3"<<"4x4"<<"5x5"<<"6x6"<<"7x7"<<"8x8"<<"9x9"<<"10x10";
    ui->comboBox->insertItems(0, pixperstar);
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
    ui->textLeftLineEdit->setValidator(new QIntValidator(100, 32767, this));
    ui->textAtopLineEdit->setValidator(new QIntValidator(100, 32767, this));
    ui->textSizeLineEdit->setValidator(new QIntValidator(0, 32767, this));
    ui->imageWeightLineEdit->setValidator(new QIntValidator(0, 100, this));
    ui->imageHeightLineEdit->setValidator(new QIntValidator(0, 100, this));
    ui->GridLenghtLineEdit->setValidator(new QIntValidator(0, 1000, this));
    ui->imageSpaceLineEdit->setValidator(new QIntValidator(0, 1000, this));
    ui->OneSlideRadioButton->setChecked(true);
    ui->filenameLineEdit->setReadOnly(true);
    ui->DistorsioPathLineEdit->setReadOnly(true);
    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setInteractive(true);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);


    if (ui->OneSlideRadioButton->isChecked())
    {
        ui->FocEndLineEdit->setDisabled(true);
        ui->FocStepLineEdit->setDisabled(true);
        ui->imageWeightLineEdit->setDisabled(true);
        ui->imageHeightLineEdit->setDisabled(true);
        ui->imageSpaceLineEdit->setDisabled(true);
    }
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);



    connect(ui->chooseNewCatPushButton, &QPushButton::clicked, this, &MainWindow::chooseCatalog);
    connect(ui->chooseCurrentCatPushButton, &QPushButton::clicked, this, &MainWindow::useCurrentCatalog);
    connect(ui->chooseDistFilePushButton, &QPushButton::clicked, this, &MainWindow::chooseDistorsioFile);
    connect(ui->createGridPushButton, &QPushButton::clicked, this, &MainWindow::createGrid);
    connect(ui->createSlidePushButton, &QPushButton::clicked, this, &MainWindow::createImage);
    connect(ui->savePushButton, &QPushButton::clicked, this, &MainWindow::saveImage);
    connect(ui->makeTestPushButton, &QPushButton::clicked, this, &MainWindow::testForSlide);
    connect(ui->slideHeightLineEdit, &QLineEdit::textEdited, this, &MainWindow::updateLineXPix);
    connect(ui->slideWidthLineEdit, &QLineEdit::textEdited, this, &MainWindow::updateLineYPix);
    connect(ui->pixSizeLineEdit, &QLineEdit::textEdited, this, &MainWindow::updateLineXMM);
    connect(ui->slidePixHeightLineEdit, &QLineEdit::textEdited, this, &MainWindow::updateLineXMM);
    connect(ui->pixSizeLineEdit, &QLineEdit::textEdited, this, &MainWindow::updateLineYMM);
    connect(ui->slidePixWidthLineEdit, &QLineEdit::textEdited, this, &MainWindow::updateLineYMM);
    connect(ui->GroupSlideRadioButton, &QRadioButton::toggled, this, &MainWindow::setLine);
    connect(ui->OneSlideRadioButton, &QRadioButton::toggled, this, &MainWindow::setLine);
    connect(ui->slideHeightLineEdit, &QLineEdit::textEdited, this, &MainWindow::setMatrixState);
    connect(ui->slidePixHeightLineEdit, &QLineEdit::textEdited, this, &MainWindow::setMatrixState);
    connect(ui->checkBox, &QCheckBox::stateChanged, this, &MainWindow::setMatrixState);
    connect(ui->checkSector, &QCheckBox::stateChanged, this, &MainWindow::setAlgorithmState);
    connect(ui->DistorsioCheck, &QCheckBox::stateChanged, this, &MainWindow::setDistButtonState);
    connect(ui->GenTabWidget, &QTabWidget::currentChanged, this, &MainWindow::chooseTab);

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
    settings->setValue("filename", filename);
    settings->setValue("distorsioFilename", distorsioFilename);
    settings->setValue("DistRead", distorsioIsRead);
    settings->setValue("betaLineEdit", ui->betaLineEdit->text());
    settings->setValue("alphaLineEdit", ui->alphaLineEdit->text());
    settings->setValue("azLineEdit", ui->azLineEdit->text());
    settings->setValue("FocStartLineEdit", ui->FocStartLineEdit->text());
    settings->setValue("FocEndLineEdit", ui->FocEndLineEdit->text());
    settings->setValue("FocStepLineEdit", ui->FocStepLineEdit->text());
    settings->setValue("minMvLineEdit", ui->minMvLineEdit->text());
    settings->setValue("maxMvLineEdit", ui->maxMvLineEdit->text());
    settings->setValue("pixSizeLineEdit", ui->pixSizeLineEdit->text());
    settings->setValue("slideHeightLineEdit", ui->slideHeightLineEdit->text());
    settings->setValue("slidewidthLineEdit", ui->slideWidthLineEdit->text());
    settings->setValue("slidePixHeightLineEdit", ui->slidePixHeightLineEdit->text());
    settings->setValue("slidePixwidthLineEdit", ui->slidePixWidthLineEdit->text());
    settings->setValue("textLeftLineEdit", ui->textLeftLineEdit->text());
    settings->setValue("textAtopLineEdit", ui->textAtopLineEdit->text());
    settings->setValue("textSizeLineEdit", ui->textSizeLineEdit->text());
    settings->setValue("imageWeightLineEdit", ui->imageWeightLineEdit->text());
    settings->setValue("imageHeightLineEdit", ui->imageHeightLineEdit->text());
    settings->setValue("imageSpaceLineEdit", ui->imageSpaceLineEdit->text());
    settings->setValue("prefixLineEdit", ui->prefixLineEdit->text());
    settings->setValue("suffixLineEdit", ui->suffixLineEdit->text());
    settings->setValue("slidewidthLineEdit_state", ui->slideWidthLineEdit->isReadOnly());
    settings->setValue("slidePixwidthLineEdit_state", ui->slideWidthLineEdit->isReadOnly());
    settings->setValue("checkBox", ui->checkBox->isChecked());
    settings->setValue("comboBoxIndex", ui->comboBox->currentIndex());
    settings->setValue("showPreviewCheckBox", ui->showPreviewCheckBox->isChecked());
    settings->setValue("checkSector", ui->checkSector->isChecked());
    settings->setValue("PicDiamLineEdit", ui->PicDiamLineEdit->text());
    settings->setValue("PicDiamLineEdit_state", ui->checkSector->isChecked());
    settings->setValue("DistorsioCheck", ui->DistorsioCheck->isChecked());
    settings->setValue("xDistorsioVector", QVariant::fromValue(xDistorsioVector));
    settings->setValue("yDistorsioVector", QVariant::fromValue(yDistorsioVector));
    settings->setValue("GenTabCurrent", ui->GenTabWidget->currentIndex());
    settings->setValue("AddTabCurrent", ui->AddTabWidget->currentIndex());
    settings->setValue("GridLenghtLineEdit", ui->GridLenghtLineEdit->text());
    settings->setValue("miniCat", ui->littleCatCheckBox->isChecked());
    settings->sync();

}

void MainWindow::loadSettings()
{
    filename = settings->value("filename", "Каталог не выбран...").toString();
    ui->filenameLineEdit->setText(filename);
    distorsioFilename = settings->value("distorsioFilename", "Файл не выбран...").toString();
    ui->DistorsioPathLineEdit->setText(distorsioFilename);
    distorsioIsRead = settings->value("DistRead", "false").toBool();
    ui->betaLineEdit->setText(settings->value("betaLineEdit", "45").toString());
    ui->alphaLineEdit->setText(settings->value("alphaLineEdit", "45").toString());
    ui->azLineEdit->setText(settings->value("azLineEdit", "0").toString());
    ui->FocStartLineEdit->setText(settings->value("FocStartLineEdit", "105").toString());
    ui->FocEndLineEdit->setText(settings->value("FocEndLineEdit", "110").toString());
    ui->FocStepLineEdit->setText(settings->value("FocStepLineEdit", "0.5").toString());
    ui->minMvLineEdit->setText(settings->value("minMvLineEdit", "0").toString());
    ui->maxMvLineEdit->setText(settings->value("maxMvLineEdit", "10").toString());
    ui->pixSizeLineEdit->setText(settings->value("pixSizeLineEdit", "0.00635").toString());
    ui->slideHeightLineEdit->setText(settings->value("slideHeightLineEdit", "45.085").toString());
    ui->slideWidthLineEdit->setText(settings->value("slidewidthLineEdit", "45.085").toString());
    ui->slidePixHeightLineEdit->setText(settings->value("slidePixHeightLineEdit", "7100").toString());
    ui->slidePixWidthLineEdit->setText(settings->value("slidePixwidthLineEdit", "7100").toString());
    ui->textLeftLineEdit->setText(settings->value("textLeftLineEdit", "150").toString());
    ui->textAtopLineEdit->setText(settings->value("textAtopLineEdit", "150").toString());
    ui->textSizeLineEdit->setText(settings->value("textSizeLineEdit", "200").toString());
    ui->imageWeightLineEdit->setText(settings->value("imageWeightLineEdit", "4").toString());
    ui->imageHeightLineEdit->setText(settings->value("imageHeightLineEdit", "4").toString());
    ui->imageSpaceLineEdit->setText(settings->value("imageSpaceLineEdit", "200").toString());
    ui->prefixLineEdit->setText(settings->value("prefixLineEdit", "Hi").toString());
    ui->suffixLineEdit->setText(settings->value("suffixLineEdit", "Bye").toString());
    ui->PicDiamLineEdit->setText(settings->value("PicDiamLineEdit", "34").toString());
    ui->checkBox->setChecked(settings->value("checkBox").toBool());
    ui->showPreviewCheckBox->setChecked(settings->value("showPreviewCheckBox").toBool());
    int index = settings->value("comboBoxIndex", 10).toInt();
    ui->comboBox->setCurrentIndex(index);
    ui->GenTabWidget->setCurrentIndex(settings->value("GenTabCurrent", "0").toInt());
    ui->AddTabWidget->setCurrentIndex(settings->value("AddTabCurrent", "0").toInt());
    ui->slideWidthLineEdit->setReadOnly(settings->value("slidewidthLineEdit_state", "false").toBool());
    ui->slidePixWidthLineEdit->setReadOnly(settings->value("slidePixwidthLineEdit_state", "false").toBool());
    ui->checkSector->setChecked(settings->value("checkSector").toBool());
    ui->PicDiamLineEdit->setDisabled(settings->value("PicDiamLineEdit_state", "true").toBool());
    ui->GridLenghtLineEdit->setText(settings->value("GridLenghtLineEdit", "100").toString());
    ui->DistorsioCheck->setChecked(settings->value("DistorsioCheck").toBool());
    ui->chooseDistFilePushButton->setDisabled(!settings->value("DistorsioCheck").toBool());
    ui->littleCatCheckBox->setChecked(settings->value("miniCat").toBool());
    xDistorsioVector = settings->value("xDistorsioVector").value<QList<double> >();
    yDistorsioVector = settings->value("yDistorsioVector").value<QList<double> >();

    if (!xDistorsioVector.size())
    {
        ui->distReadInfoLabel->setText("Коэффициенты не считаны");
    }
    else
    {
        ui->distReadInfoLabel->setText("Коэффициенты считаны");
    }

}

void MainWindow::makeInscription(QSharedPointer<QImage> optimalImage, const QString& setableText, int fontX, int fontY, int fontSize)
{
    // наносим надпись на слайд
    QPainter painterForText;
    painterForText.begin(optimalImage.data());
    QPen penForText;
    QFont font = painterForText.font();
    font.setPixelSize(fontSize);
    painterForText.setFont(font);
    painterForText.setPen(penForText);
    painterForText.setPen(QColor(255, 255, 255, 255));
    painterForText.drawText(fontX, fontY, setableText);
    painterForText.end();// завершаем нанесение надписи на слайд
}


InscriptParams MainWindow::readInscriptionParams()
{
    InscriptParams data;
    data.fontX = ui->textLeftLineEdit->text().toInt();
    data.fontY = ui->textAtopLineEdit->text().toInt();
    data.fontSize = ui->textSizeLineEdit->text().toInt();
    if (data.fontSize <= 0)
    {
        throw data.fontSize;
    }
    data.prefix = ui->prefixLineEdit->text();
    data.suffix = ui->suffixLineEdit->text();

    return data;

}
StarSlideData MainWindow::readInputStarSlideData()
{
    StarSlideData data {};
    data.pointAlpha = ui->alphaLineEdit->text().toDouble();
    if (data.pointAlpha < 0 || data.pointAlpha > 360)
    {
        throw std::invalid_argument("Неверно задан угол альфа");
    }

    data.pointBeta = ui->betaLineEdit->text().toDouble();
    if (data.pointBeta < -90 || data.pointBeta > 90)
    {
        throw std::invalid_argument("Неверно задан угол дельта");
    }

    data.pointAzimut = ui->azLineEdit->text().toDouble();
    if (data.pointAzimut < 0 || data.pointAzimut > 360)
    {
        throw std::invalid_argument("Неверно задан угол азимут");
    }

    data.insideViewAngle = ui->PicDiamLineEdit->text().toDouble();
    if (data.insideViewAngle < 0)
    {
        throw std::invalid_argument("Неверно задано внутреннее поле зрения");
    }

    data.focStart = ui->FocStartLineEdit->text().toDouble();
    if (data.focStart < 0)
    {
        throw std::invalid_argument("Неверно задано фокусное расстояние");
    }

    data.focEnd = ui->FocEndLineEdit->text().toDouble();
    if (data.focEnd < 0 || data.focEnd <= data.focStart)
    {
        throw std::invalid_argument("Неверно задано фокусное расстояние (конец интервала)");
    }


    data.focStep = ui->FocStepLineEdit->text().toDouble();
    if (data.focStep < 0)
    {
        throw std::invalid_argument("Неверно задан шаг по фокуснуму расстоянию");
    }

    data.minMv = ui->minMvLineEdit->text().toDouble();
    data.maxMv = ui->maxMvLineEdit->text().toDouble();
    if (data.maxMv < data.minMv)
    {
        throw std::invalid_argument("Неверно задана максимальная звёздная величина");
    }
    data.pixelPerStar = ui->comboBox->currentIndex();
    data.slideSizeX = ui->slidePixWidthLineEdit->text().toInt();
    if (data.slideSizeX <= 0)
    {
        throw std::invalid_argument("Неверно задан размер слайда по Х");
    }
    data.slideSizeY = ui->slidePixHeightLineEdit->text().toInt();
    if (data.slideSizeY <= 0)
    {
        throw std::invalid_argument("Неверно задан размер слайда по У");
    }
    data.slideSizeMMy = ui->slideHeightLineEdit->text().toDouble();
    data.slideSizeMMx = ui->slideWidthLineEdit->text().toDouble();

    data.pix = ui->pixSizeLineEdit->text().toDouble();
    if (data.pix < 0)
    {
        throw std::invalid_argument("Неверно задан пиксель");
    }

    return data;
}

GroupImgParams MainWindow::readGroupImgParams()
{
    GroupImgParams data {};
    if (ui->GroupSlideRadioButton->isChecked())
    {
        data.countY = ui->imageHeightLineEdit->text().toDouble();
        data.countX = ui->imageWeightLineEdit->text().toDouble();
        data.space = ui->imageSpaceLineEdit->text().toInt();
        if (data.space < 0)
        {
            throw std::invalid_argument("Неверно задано значение пропуска между слайдами");
        }
    }
    return data;
}

GridSlideData MainWindow::readInputGridSlideData()
{
    GridSlideData data {};
    data.gridDistance = ui->GridLenghtLineEdit->text().toInt();
    if (data.gridDistance<0)
    {
        throw std::invalid_argument("Неверно задана дистанция сетки");
    }
    data.pixelPerStar = ui->comboBox->currentIndex();
    data.slideSizeX = ui->slidePixWidthLineEdit->text().toInt();
    if (data.slideSizeX <= 0)
    {
        throw std::invalid_argument("Неверно задан размер сетки по Х");
    }
    data.slideSizeY = ui->slidePixHeightLineEdit->text().toInt();
    if (data.slideSizeY <= 0)
    {
        throw std::invalid_argument("Неверно задан размер сетки по У");
    }

    data.pix = ui->pixSizeLineEdit->text().toDouble();
    if (data.pix < 0)
    {
        throw std::invalid_argument("Неверно задан пиксель");
    }

    return data;
}


void MainWindow::drawGridSlides(QSharedPointer<QImage> im, QImage& optImg, int szX, int szY, int space, int slideSizeX, int slideSizeY)
{
    QPainter gridPainter;
    gridPainter.begin(im.data());

    for (int curColumn = 0; curColumn < szY; curColumn++)
    {
        for (int curRow = 0; curRow < szX; curRow++)
        {
            gridPainter.drawImage(slideSizeX * curRow + space * curRow, slideSizeY * curColumn + space * curColumn, optImg);
        }
    }
    gridPainter.end();
}






void MainWindow::drawSlide(QSharedPointer<QImage> im, QVector <QImage>& imageVector, int szX, int szY, int space, int  slideSizeX, int slideSizeY)
{
    QPainter slidePainter;
    slidePainter.begin(im.data());

    for (int i = 0;i < szY; i++)
    {
        for (int j = 0;j < szX; j++)
        {
            slidePainter.drawImage(slideSizeX * j + space * j, slideSizeY * i + space * i , imageVector[j + i * szX]);
            qApp->processEvents();
        }

    }
    slidePainter.end();

}

void MainWindow::drawPreviewItem(const QByteArray& svgByteArray, int slideSizeX, int slideSizeY)
{
    svgPreview.reset(new QSvgRenderer(svgByteArray));
    svgPreviewItem.reset(new QGraphicsSvgItem());
    svgPreviewItem->setSharedRenderer(svgPreview.data());

    if (ui->showPreviewCheckBox->isChecked())
    {
        scene->setSceneRect(0, 0, slideSizeX, slideSizeY);
        scene->addItem(svgPreviewItem.data());
        ui->graphicsView->setScene(scene.data());
    }
}



void MainWindow::setUIstate(bool state)
{
    ui->OneSlideRadioButton->setDisabled(state);
    ui->GroupSlideRadioButton->setDisabled(state);
    ui->chooseNewCatPushButton->setDisabled(state);
    ui->savePushButton->setDisabled(state);
    ui->DistorsioCheck->setDisabled(state);
}

void MainWindow::updateLineXPix()
{
    QString updateString;
    int updateBuffer;
    double pix = ui->pixSizeLineEdit->text().toDouble();
    double slideSizeMMx = ui->slideHeightLineEdit->text().toDouble();
    if (!qFuzzyCompare(slideSizeMMx, 0.0)
            && !qFuzzyCompare(pix, 0.0))
    {
        updateBuffer = slideSizeMMx / pix;
        updateString = QString::number(updateBuffer);
        ui->slidePixHeightLineEdit->setText(updateString);
    }

}

void MainWindow::updateLineYPix()
{
    QString updateString;
    int updateBuffer;
    double pix = ui->pixSizeLineEdit->text().toDouble();
    double slideSizeMMy = ui->slideWidthLineEdit->text().toDouble();
    if (!qFuzzyCompare(slideSizeMMy, 0.0)
            && !qFuzzyCompare(pix, 0.0))
    {
        updateBuffer = slideSizeMMy / pix;
        updateString = QString::number((updateBuffer));
        ui->slidePixWidthLineEdit->setText(updateString);
    }

}



void MainWindow::updateLineXMM()

{
    QString updateString;
    double pix = ui->pixSizeLineEdit->text().toDouble();
    int slideSizeX = ui->slidePixHeightLineEdit->text().toInt();
    if (!qFuzzyCompare(pix, 0.0) &&
            !qFuzzyCompare(slideSizeX, 0.0))
    {
        updateString = QString::number(slideSizeX * pix);
        ui->slideHeightLineEdit->setText(updateString);
    }
}





void MainWindow::updateLineYMM()

{
    QString updateString;
    double pix = ui->pixSizeLineEdit->text().toDouble();
    int slideSizeY = ui->slidePixWidthLineEdit->text().toInt();
    if (!qFuzzyCompare(pix, 0.0)
            && !qFuzzyCompare(slideSizeY, 0.0))
    {
        updateString = QString::number(slideSizeY * pix);
        ui->slideWidthLineEdit->setText(updateString);
    }
}


void MainWindow::setLine()
{
    if (ui->GroupSlideRadioButton->isChecked())
    {
        ui->FocEndLineEdit->setDisabled(false);
        ui->FocStepLineEdit->setDisabled(false);
        ui->imageWeightLineEdit->setDisabled(false);
        ui->imageHeightLineEdit->setDisabled(false);
        ui->imageSpaceLineEdit->setDisabled(false);

    }
    else if (ui->OneSlideRadioButton->isChecked())
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
    if (ui->checkBox->isChecked())
    {
        ui->slideWidthLineEdit->setReadOnly(true);
        ui->slidePixWidthLineEdit->setReadOnly(true);
        ui->slideWidthLineEdit->setText(ui->slideHeightLineEdit->text());
        ui->slidePixWidthLineEdit->setText(ui->slidePixHeightLineEdit->text());
    }
    else if (!ui->checkBox->isChecked())
    {
        ui->slideWidthLineEdit->setReadOnly(false);
        ui->slidePixWidthLineEdit->setReadOnly(false);
    }

}





void MainWindow::setAlgorithmState()
{
    if (ui->checkSector->isChecked())
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
    if (ui->DistorsioCheck->isChecked())
    {
        ui->chooseDistFilePushButton->setDisabled(false);
    }
    else
    {
        ui->chooseDistFilePushButton->setDisabled(true);
    }

}



void MainWindow::chooseTab()
{
    if (!ui->GenTabWidget->currentIndex())
    {
        ui->FocEndLineEdit->setDisabled(false);
        ui->FocStartLineEdit->setDisabled(false);
        ui->FocStepLineEdit->setDisabled(false);
        ui->minMvLineEdit->setDisabled(false);
        ui->maxMvLineEdit->setDisabled(false);
    }
    else if (ui->GenTabWidget->currentIndex() == 1)
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
    if (ui->showPreviewCheckBox->isChecked())// если отображаем на превью, то очищаем его каждый раз
    {
        if (!scene)
        {
            scene.reset(new QGraphicsScene);
        }
        else
        {
            svgPreview.reset();
            svgPreviewItem.reset();
            scene->update(scene->sceneRect());
        }

        ui->graphicsView->setScene(scene.data());
    }

    images.clear();
    optimalImage.reset();
}

void MainWindow::setImagesSizes(int slideSizeX, int slideSizeY, const GroupImgParams& p)
{
    images.resize(countOfImages);
    const int limitX = imageSizeLimit / slideSizeX;// вычисляем максимально возможное чесло слайдов в ширинуизображении
    const int limitY = imageSizeLimit / slideSizeY;// вычисляем максимально возможное число слайдов в высоту на одном изображении
    int imageSizeX = 0;
    int imageSizeY = 0;

    //размер изображения в зависимости от лимита и размера одного слайда

    const int firstImageSlideCountX = p.countX > limitX ? limitX : p.countX;
    const int firstImageSlideCountY = p.countY > limitY ? limitY : p.countY;

    imageSizeX = (slideSizeX * firstImageSlideCountX)+(p.space*2) * firstImageSlideCountX;
    imageSizeY = (slideSizeY * firstImageSlideCountY)+(p.space*2) * firstImageSlideCountY;
    images[0].reset(new QImage(imageSizeX, imageSizeY, QImage::Format_Mono));
    images[0]->fill(Qt::color1);

    if (p.countX > limitX)
    {
        const int secondImageSlideCountX = p.countX - limitX > limitX ? limitX : p.countX - limitX;
        const int secondImageSlideCountY = p.countY > limitY ? limitY : p.countY;;

        imageSizeX = (slideSizeX * secondImageSlideCountX)+(p.space*2) * secondImageSlideCountX;
        imageSizeY = (slideSizeY * secondImageSlideCountY)+(p.space*2) * secondImageSlideCountY;
        images[1].reset(new QImage(imageSizeX, imageSizeY, QImage::Format_Mono));
        images[1]->fill(Qt::color1);
    }

    if (p.countY > limitY)
    {
        const int thirdImageSlideCountX = p.countX > limitX ? limitX : p.countX;
        const int thirdImageSlideCountY = p.countY - limitY > limitY ? limitY : p.countY - limitY;

        imageSizeX = (slideSizeX * thirdImageSlideCountX) + (p.space*2) * thirdImageSlideCountX;
        imageSizeY = (slideSizeY * thirdImageSlideCountY) + (p.space*2) * thirdImageSlideCountY;
        images[2].reset(new QImage(imageSizeX, imageSizeY, QImage::Format_Mono));
        images[2]->fill(Qt::color1);
    }

    if (p.countX > limitX && p.countY > limitY)
    {
        const int fourthImageSlideCountX = p.countX - limitX > limitX ? limitX : p.countX - limitX;
        const int fourthImageSlideCountY = p.countY - limitY > limitY ? limitY : p.countY - limitY;

        imageSizeX = (slideSizeX * fourthImageSlideCountX)+(p.space*2) * fourthImageSlideCountX;
        imageSizeY = (slideSizeY * fourthImageSlideCountY)+(p.space*2) * fourthImageSlideCountY;
        images[3].reset(new QImage(imageSizeX, imageSizeY, QImage::Format_Mono));
        images[3]->fill(Qt::color1);
    }

}

void MainWindow::openCatalog()
{
    bool status = false;
    QString error;
    bool readAdd = !ui->littleCatCheckBox->isChecked();
    if (!readAdd)
    {
        ui->checkSector->setChecked(false);
        ui->checkSector->setDisabled(true);
    }
    else
    {
        ui->checkSector->setDisabled(false);
    }
    catalogData.openCatalog (filename, status, error, readAdd);
    if (!status)
    {
        ui->catalogStatusLabel->setText("Ошибка");
        catalogIsRead = status;
        QMessageBox::critical (nullptr, "Ошибка", error);
    }
    else
    {
        catalogIsRead = status;
        ui->catalogStatusLabel->setText(QString::number(catalogData.alphaVec().size()));
    }
}





void MainWindow::chooseCatalog()
{
    if (lastCatalogDirectory.isNull())
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

    if (lastDistorsioDirectory.isNull())
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
    if (file.open(QIODevice::ReadOnly |QIODevice::Text))
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
        QMessageBox::critical(nullptr, "Ошибка", "Ошибка открытия файла");
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



// проверка соответствия длины массива фокусных расстояний размерности матрицы
void MainWindow::compareImageFocSize(const int countOfSlides, const int countOfFoc)
{
    // проверка соответствия длины массива фокусных расстояний размерности матрицы
    if (countOfSlides < countOfFoc)
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

    else if (countOfSlides > countOfFoc)
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
}


void MainWindow::prepareTextToSvg(QString& text)
{
    text.replace(text.indexOf("α"), 1, "a").replace(text.indexOf("δ"), 1, "d");
    text.replace(text.indexOf("°"), 1, "deg.");
    text.replace(text.indexOf("°"), 1, "deg.");
}

void MainWindow::createImage()
{

    if (!catalogIsRead)// если каталог не считан
    {
        QMessageBox::warning(nullptr, "Ошибка", "Выберите каталог или подтвердите использование текущего");
        return;
    }

    if (ui->DistorsioCheck->isChecked() && !distorsioIsRead)
    {
        QMessageBox::warning(nullptr, "Ошибка", "Выберите файл с коэффициентами дисторсии");
        return;
    }

    ui->progressBar->setValue(0);

    try
    {

        setUIstate(true);
        clearSceneAndImages();
        StarSlideData slideData(readInputStarSlideData());
        GroupImgParams groupImgData (readGroupImgParams());
        InscriptParams inscriptData(readInscriptionParams());
        QScopedPointer <SlideCreator> slideCreator(new SlideCreator());

        if  (ui->vectorRadioButton->isChecked())
             slideCreator->setOnlyParameters(true);

        slideCreator->calculateAngularDistOptions(slideData, catalogData, ui->checkSector->isChecked());


        if (ui->OneSlideRadioButton->isChecked())
        {
            ui->progressBar->setRange(0, 100);
            DistorsioData distData;
            if (ui->DistorsioCheck->isChecked())
            {
                distData.xDistorsioVector = xDistorsioVector;
                distData.yDistorsioVector = yDistorsioVector;
            }

            SlideParameters imgData = slideCreator->createStarSlide(slideData.focStart, ui->checkSector->isChecked(), ui->DistorsioCheck->isChecked(), distData);
            ui->progressBar->setValue(50);
            QString setableText = inscriptData.prefix + " " + "f=" + QString::number(slideData.focStart)+"mm"+", "+" "
                    +QString::number(imgData.countOfStars)+"zv." + ", " + " " + " cat."+" "
                    +QString::number(catalogData.alphaVec().size())+", " + " " + ui->comboBox->currentText()+", "
                    +" "+QChar(0x03B1) + "=" + QString::number(slideData.pointAlpha) + QChar(0x00B0) + ", "
                    +" "+QChar(0x03B4) + "=" + QString::number(slideData.pointBeta) + QChar(0x00B0) + ", "+" "
                    +"2w=" + QString::number(imgData.viewAngleX)+"x"
                    +QString::number(imgData.viewAngleY)+" "+inscriptData.suffix;

            if (ui->rasterRadioButton->isChecked())
            {
                optimalImage = slideCreator->getSlidePointer();
                makeInscription(optimalImage, setableText, inscriptData.fontX, inscriptData.fontY, inscriptData.fontSize);
            }

            prepareTextToSvg(setableText);
            svgSlide =
                    SlideCreator::createSvg (slideData, inscriptData , setableText, imgData.coordsOfStars);

            if (ui->showPreviewCheckBox->isChecked())
            {
                drawPreviewItem(svgSlide.toByteArray(), slideData.slideSizeX, slideData.slideSizeY);
            }
            ui->progressBar->setValue(100);
        }


        else if (ui->GroupSlideRadioButton->isChecked())
        {
            const int countOfSlides = groupImgData.countX * groupImgData.countY;
            const int countOfFoc = ((slideData.focEnd - slideData.focStart) / slideData.focStep) + 1;
            compareImageFocSize(countOfSlides, countOfFoc);

            int limitX = imageSizeLimit / slideData.slideSizeX;
            int limitY = imageSizeLimit / slideData.slideSizeY;

            if (ui->rasterRadioButton->isChecked())
                setImagesSizes(slideData.slideSizeX, slideData.slideSizeY, groupImgData);

            QVector <double> focusVector;
            for (double i = slideData.focStart; i <= slideData.focEnd; i += slideData.focStep) // формируем вектор фокусных расстояний
            {
                focusVector.append(i);
            }
            // если размерность матрицы больше длины массива фок.расстояний, дозабиваем массив значениями из его
            // начала

            int difference = countOfSlides - countOfFoc;
            if (difference >= 0)
            {
                for (int i = 0; i <= difference;i ++)
                {
                    focusVector.append(focusVector[i]);
                }

            }

            ui->progressBar->setRange(0, countOfSlides);

            DistorsioData distData;
            if (ui->DistorsioCheck->isChecked())
            {
                distData.xDistorsioVector = xDistorsioVector;
                distData.yDistorsioVector = yDistorsioVector;
            }

            QVector <QVector <QImage>> imageVector(countOfImages);
            QSharedPointer <QImage> bufImage;
            QVector <QVector <StarParameters> > starParamVecs;
            QVector <QString> textData;
            QVector<double>::const_iterator focusValue = focusVector.begin();
            for (int imgHeight = 0; imgHeight < groupImgData.countY; imgHeight++)
            {
                for (int imWidth = 0; imWidth < groupImgData.countX; imWidth++)
                {
                    qApp->processEvents();
                    ui->progressBar->setValue(imWidth + imgHeight * groupImgData.countX + 1);
                    SlideParameters imgData =
                            slideCreator->createStarSlide(*focusValue, ui->checkSector->isChecked(), ui->DistorsioCheck->isChecked(), distData);
                    starParamVecs.append(imgData.coordsOfStars);

                    QString setableText = inscriptData.prefix + " " + "f="+QString::number(*focusValue) +  "mm" + ", " + " "
                            + QString::number(imgData.countOfStars)+"zv." + ", " + " " + " cat." + " "
                            + QString::number(catalogData.alphaVec().size()) + ", " + " "+ui->comboBox->currentText() + ", "
                            + " "+QChar(0x03B1) + "=" + QString::number(slideData.pointAlpha) + QChar(0x00B0) + ", "
                            + " "+QChar(0x03B4) + "=" + QString::number(slideData.pointBeta) + QChar(0x00B0) + ", " + " "
                            + "2w = " +QString::number(imgData.viewAngleX) + "x"
                            + QString::number(imgData.viewAngleY) + " " + inscriptData.suffix;

                    if (ui->rasterRadioButton->isChecked())
                    {
                        bufImage = slideCreator->getSlidePointer();
                        makeInscription(bufImage, setableText, inscriptData.fontX, inscriptData.fontY, inscriptData.fontSize);
                        textData.append(setableText);

                        // расфасовываем слайды по изображениям
                        if (imgHeight <= limitY - 1 && imWidth <= limitX - 1) // если текущая ширина и высота не превышают лимит
                        {
                            imageVector[0].append(*bufImage);
                        }

                        else if (imgHeight > limitY - 1 && imWidth > limitX - 1) // если текущая ширина и высота превышают лимит
                        {
                            imageVector[3].append(*bufImage);
                        }

                        else if (imWidth > limitX - 1) // если текущая ширина превышает лимит
                        {
                            imageVector[1].append(*bufImage);
                        }

                        else if (imgHeight > limitY - 1) // если текущая высота превышает лимит
                        {
                            imageVector[2].append(*bufImage);
                        }
                        bufImage.reset();
                    }
                    ++focusValue;

                }
            }

            for (int i = 0; i < countOfImages; i++)
            {
                if (!images[i].isNull())
                {
                    int sizeY = images[i]->height()/(slideData.slideSizeY + groupImgData.space * 2);
                    int sizeX = images[i]->width()/(slideData.slideSizeX + groupImgData.space * 2);
                    drawSlide(images[i], imageVector[i], sizeX, sizeY, groupImgData.space, slideData.slideSizeX, slideData.slideSizeY);
                }
            }

            for(auto& str : textData)
            {
                prepareTextToSvg(str);
            }
            svgSlide =
                    SlideCreator::createFullSvg(starParamVecs, slideData.slideSizeX, slideData.slideSizeY, groupImgData, inscriptData, textData);

            if (ui->showPreviewCheckBox->isChecked())
            {
                drawPreviewItem(svgSlide.toByteArray(), slideData.slideSizeX * groupImgData.countX, slideData.slideSizeY * groupImgData.countY);
            }

        }
        setUIstate(false);
    }

    catch (exception &e)
    {
        setUIstate(false);
        QMessageBox::information(nullptr, "Ошибка", e.what());
        return;
    }


}




void MainWindow::createGrid()
{

    if (ui->DistorsioCheck->isChecked() && !distorsioIsRead)
    {
        QMessageBox::warning(nullptr, "Ошибка", "Выберите файл с коэффициентами дисторсии");
        return;
    }
    try
    {

        ui->progressBar->setValue(0);
        setUIstate(true);
        clearSceneAndImages();

        QScopedPointer <SlideCreator> slideCreator(new SlideCreator());
        if  (ui->vectorRadioButton->isChecked())
            slideCreator->setOnlyParameters(true);
        GridSlideData gridData(readInputGridSlideData());
        InscriptParams inscriptData(readInscriptionParams());

        DistorsioData distData;
        distData.xDistorsioVector = xDistorsioVector;
        distData.yDistorsioVector = yDistorsioVector;

        if (ui->OneSlideRadioButton->isChecked())
        {
            QVector<StarParameters> coordsOfStars = slideCreator->createGridSlide(gridData, ui->DistorsioCheck->isChecked(), distData);

            QString textForGrid = inscriptData.prefix+" "+"Разм.пикселя:" + " " + ui->comboBox->currentText()+ ", " + " " +
                    "Расстояние, пикс.:" + " " + QString::number(gridData.gridDistance) + " " + inscriptData.suffix;

            if (ui->rasterRadioButton->isChecked())
            {
                optimalImage = slideCreator->getSlidePointer();
                makeInscription(optimalImage, textForGrid, inscriptData.fontX, inscriptData.fontY, inscriptData.fontSize);
            }

            prepareTextToSvg(textForGrid);
            svgSlide =
                    SlideCreator::createSvg(gridData , inscriptData , textForGrid, coordsOfStars);
            if (ui->showPreviewCheckBox->isChecked())
            {
                drawPreviewItem(svgSlide.toByteArray(), gridData.slideSizeX, gridData.slideSizeY);
            }
            ui->progressBar->setValue(100);
        }

        else if (ui->GroupSlideRadioButton->isChecked())
        {
            ui->progressBar->setRange(0, 100);
            GroupImgParams groupImgData (readGroupImgParams());
            QSharedPointer <QImage> bufImage;

            if (ui->rasterRadioButton->isChecked())
                setImagesSizes(gridData.slideSizeX, gridData.slideSizeY, groupImgData);

            QVector <StarParameters> coordsOfStars = slideCreator->createGridSlide(gridData, ui->DistorsioCheck->isChecked(), distData);

            QString textForGrid = inscriptData.prefix+" "+"Size, pix:"+" "+ui->comboBox->currentText()+", "+" "+
                    "Distance, pix.:"+" "+QString::number(gridData.gridDistance)+" "+inscriptData.suffix;

            if (ui->rasterRadioButton->isChecked())
            {
                bufImage = slideCreator->getSlidePointer();
                makeInscription(bufImage, textForGrid, inscriptData.fontX, inscriptData.fontY, inscriptData.fontSize);
            }

            ui->progressBar->setValue(50);

            for (auto& image : images)
            {
                if (!image.isNull())
                {
                    int sizeY = image->height() / (gridData.slideSizeY + groupImgData.space * 2);
                    int sizeX = image->width() / (gridData.slideSizeX + groupImgData.space * 2);
                    drawGridSlides(image, *bufImage, sizeX, sizeY, groupImgData.space, gridData.slideSizeX, gridData.slideSizeY);
                }
            }

            int countOfSlides = groupImgData.countY * groupImgData.countX;
            QVector <QVector <StarParameters>> sParams (countOfSlides, coordsOfStars);
            QVector <QString> textData (countOfSlides, textForGrid);
            for(auto& str : textData)
            {
                prepareTextToSvg(str);
            }
            svgSlide =
                    SlideCreator::createFullSvg(sParams, gridData.slideSizeX, gridData.slideSizeY, groupImgData, inscriptData, textData);
            if (ui->showPreviewCheckBox->isChecked())
            {
                drawPreviewItem(svgSlide.toByteArray(), gridData.slideSizeX * groupImgData.countX, gridData.slideSizeY * groupImgData.countY);
            }
            ui->progressBar->setValue(100);

        }
        setUIstate(false);
    }

    catch (std::exception &e)
    {
        setUIstate(false);
        QMessageBox::information(nullptr, "Ошибка", e.what());
        return;
    }

}






void MainWindow::testForSlide()
{
    if (!catalogIsRead)// если каталог не считан
    {
        QMessageBox::warning(nullptr, "Ошибка", "Выберите каталог или подтвердите использование текущего");
        return;
    }
    else
    {
        try
        {
            QScopedPointer<SlideCreator> slideCreator(new SlideCreator());
            StarSlideData slideData = readInputStarSlideData();
            slideCreator->calculateAngularDistOptions(slideData, catalogData, ui->checkSector->isChecked());

            DistorsioData distData;
            distData.xDistorsioVector = xDistorsioVector;
            distData.yDistorsioVector = yDistorsioVector;


            TestSlideParameters testData=slideCreator->testStarSlide(ui->checkSector->isChecked(), ui->DistorsioCheck->isChecked(), distData);
            QString message;

            if (ui->OneSlideRadioButton->isChecked())
            {
                message = QString("Число звёзд на слайде :")+" "+ QString::number(testData.countOfStars)
                        +"\n"+ "Фокусное расстояние :"+" "+ QString::number(slideData.focStart)
                        +"\n"+ "Угол зрения слайда ШхВ, град :"+" "+ QString::number(testData.viewAngleX)+"x"+QString::number(testData.viewAngleY)
                        +"\n"+ "Размер слайда пикс. :"+" "+ QString::number(slideData.slideSizeX)+"x"+QString::number(slideData.slideSizeY)
                        +"\n"+ "Размер слайда мм. :"+" "+ QString::number(slideData.slideSizeMMx)+"x"+QString::number(slideData.slideSizeMMy);

            }

            else if (ui->GroupSlideRadioButton->isChecked())
            {
                GroupImgParams groupImageParams(readGroupImgParams());
                message = QString("ШхВ :")+" "+ QString::number(groupImageParams.countX) + "x" + QString::number(groupImageParams.countY)+
                        "\n"+"Число cлайдов :"+" "+ QString::number(groupImageParams.countX * groupImageParams.countY)+
                        "\n"+"Число фокусных расстояний :"+" "+ QString::number(((slideData.focEnd - slideData.focStart)/slideData.focStep) + 1)+
                        "\n"+"Размер мм. :"+" "+ QString::number(slideData.slideSizeMMx * groupImageParams.countX)+
                        "x"+ QString::number(slideData.slideSizeMMy * groupImageParams.countY)+
                        "\n"+"Размер пикс. :"+" "+ QString::number(slideData.slideSizeX * groupImageParams.countX)+
                        "x"+ QString::number(slideData.slideSizeY*groupImageParams.countY)
                        +"\n"+ "Фокусное расстояние первого слайда :"+" "+ QString::number(slideData.focStart)
                        +"\n"+ "Размер первого слайда мм. :"+" "+ QString::number(slideData.slideSizeMMx) + "x" + QString::number(slideData.slideSizeMMy)
                        +"\n"+ "Размер первого слайда пикс. :"+" "+ QString::number(slideData.slideSizeX) + "x" + QString::number(slideData.slideSizeY)
                        +"\n"+ "Угол зрения первого слайда ШхВ, град :"+" "+ QString::number(testData.viewAngleX)+"x" + QString::number(testData.viewAngleY)
                        +"\n"+ "Число звезд на первом слайде :"+" "+ QString::number(testData.countOfStars);


            }
            QMessageBox::information(nullptr, "Тест", message);


        }

        catch (exception &e)
        {
            QMessageBox::information(nullptr, "Ошибка", e.what());
            return;
        }

    }
}


void MainWindow::saveImage()
{

    QString filenameSave = QFileDialog::getSaveFileName(this,
                                                        tr("Save tiff"), ".",

                                                        tr("tiff files (*.tiff)"));
    if (ui->vectorRadioButton->isChecked())
    {
        QFile svgFile(filenameSave.remove(".tiff") + "_svg.svg");
        if(svgFile.open(QIODevice::WriteOnly)){
            QTextStream out(&svgFile);
            out << svgSlide.toString();
        }
        svgFile.close();
    }

    else
    {
        QImageWriter writer;
        writer.setFormat("tiff");
        writer.setFileName(filenameSave);
        if (ui->OneSlideRadioButton->isChecked())
        {
            if (!optimalImage.isNull())
            {
                if (!writer.write(*optimalImage))
                {
                    QMessageBox::information(nullptr, "Ошибка", writer.errorString());
                    return;
                }
                optimalImage.reset();
            }
            else
            {
                QMessageBox::information(nullptr, "Ошибка", "Нечего сохранять");
                return;
            }
        }
        else if (ui->GroupSlideRadioButton->isChecked())
        {
            QTime currentTime;
            for (int i = 0; i < images.size(); i ++)
            {
                if (!images[i].isNull())
                {
                    QString tempFileName = filenameSave;
                    QString filenameSaveAdd = tempFileName.remove(tempFileName.lastIndexOf("/") + 1, tempFileName.end() - tempFileName.begin());
                    filenameSaveAdd.append(currentTime.currentTime().toString("hhMM_ss_") + QString::number(i) + ".tiff");
                    writer.setFileName(filenameSaveAdd);
                    if (!writer.write(*images[i]))
                    {
                        QMessageBox::information(nullptr, "Ошибка", writer.errorString());
                        return;
                    }
                    images[i].reset();
                }
                else
                {
                    QMessageBox::information(nullptr, "Ошибка", "Нечего сохранять");
                    return;
                }
            }

        }
    }
    QMessageBox::information(nullptr, "Сохранение", "Успешно сохранено");
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
    if (QCursor::pos().x() > cPos.x())
    {
        ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        // шкала
        double scaleFactor = 1.15;
        if (event->delta() > 0)
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


