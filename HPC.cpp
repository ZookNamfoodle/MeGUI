#include "HPC.h"
#include "ui_HPC.h"

QString photoLink = "";
QString pageLink = "";
//QString photos = "";
QList<QString> *dir = new QList<QString>;
QString currentDir = "";
int current = 0;
QFileSystemModel *model = new QFileSystemModel;

vector<KeyPoint> KeyPointsTemp;
vector<KeyPoint> KeyPointsSource;
vector<DMatch> GoodMatches;
int Max;
Mat match;


HPC::HPC(QWidget *parent) : 
    QMainWindow(parent),
    ui(new Ui::HPC)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/resources/splash.png"));
    model->setRootPath(QDir::currentPath());
    ui->tree->setModel(model);
    ui->tree->hideColumn(1);
    ui->tree->hideColumn(2);
    ui->tree->hideColumn(3);
    //ui->tree_2->setModel(model);
    //ui->tree_2->hideColumn(1);
    //ui->tree_2->hideColumn(2);
    //ui->tree_2->hideColumn(3);
}

HPC::~HPC()
{
    delete ui;
}

inline QImage  cvMatToQImage( const cv::Mat &inMat )
 {
    switch ( inMat.type() )
    {
       // 8-bit, 4 channel
       case CV_8UC4:
       {
          QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB32 );

          return image;
       }

       // 8-bit, 3 channel
       case CV_8UC3:
       {
          QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888 );

          return image.rgbSwapped();
       }

       // 8-bit, 1 channel
       case CV_8UC1:
       {
          static QVector<QRgb>  sColorTable;

          // only create our color table once
          if ( sColorTable.isEmpty() )
          {
             for ( int i = 0; i < 256; ++i )
                sColorTable.push_back( qRgb( i, i, i ) );
          }

          QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8 );

          image.setColorTable( sColorTable );

          return image;
       }
    }

    return QImage();
 }

 inline QPixmap cvMatToQPixmap( const cv::Mat &inMat )
 {
    return QPixmap::fromImage( cvMatToQImage( inMat ) );
 }

void drawBoundingBox(QString tempAddress, QString sourceAddress)
{
    vector<Point2f> tempV;
    vector<Point2f> sceneV;
    match = imread(sourceAddress.toStdString(),CV_LOAD_IMAGE_GRAYSCALE);
    Mat temp = imread(tempAddress.toStdString(),CV_LOAD_IMAGE_GRAYSCALE);
    for( int i = 0; i < GoodMatches.size(); i++ )
    {
      //-- Get the keypoints from the good matches
      tempV.push_back( KeyPointsTemp[ GoodMatches[i].queryIdx ].pt );
      sceneV.push_back( KeyPointsSource[ GoodMatches[i].trainIdx ].pt );
    }

    Mat H = findHomography( tempV, sceneV, CV_RANSAC );

    //-- Get the corners from the template
    vector<Point2f> tempCorners(4);
    tempCorners[0] = cvPoint(0,0); tempCorners[1] = cvPoint( temp.cols, 0 );
    tempCorners[2] = cvPoint( temp.cols, temp.rows ); tempCorners[3] = cvPoint( 0, temp.rows );
    vector<Point2f> sceneCorners(4);

    perspectiveTransform( tempCorners, sceneCorners, H);

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    line(match, sceneCorners[0] , sceneCorners[1] , Scalar(0, 255, 0),  4 );
    line(match, sceneCorners[1] , sceneCorners[2] , Scalar( 0, 255, 0), 4 );
    line(match, sceneCorners[2] , sceneCorners[3] , Scalar( 0, 255, 0), 4 );
    line(match, sceneCorners[3] , sceneCorners[0] , Scalar( 0, 255, 0), 4 );
    //imshow("l", match);
}

// The main function!
bool FLANN(cv::Mat source, cv::Mat temp)
{
    cv::initModule_nonfree();//THIS LINE IS IMPORTANT
    // Create an instance of SIFT
    cv::SiftFeatureDetector detector(0.05, 5.0);
    cv::SiftDescriptorExtractor extractor(3.0);


    std::vector<cv::KeyPoint> keyS, keyT;
    detector.detect( source, keyS );
    detector.detect( temp, keyT );

    cv::Mat featT,featS;
    cv::drawKeypoints(temp,keyT,featT,cv::Scalar(255, 255, 255),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::drawKeypoints(source,keyS,featS,cv::Scalar(255, 255, 255),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

     //-- Step 2: Calculate descriptors (feature vectors)

     cv::Mat descriptorS, descriptorT;

     extractor.compute( temp, keyT, descriptorT );
     extractor.compute( source, keyS, descriptorS );

     //-- Step 3: Matching descriptor vectors using FLANN matcher
     cv::FlannBasedMatcher matcher;
     std::vector< cv::DMatch > matches;
     matcher.match( descriptorT,descriptorS, matches );

     double max_dist = 0; double min_dist = 35;

     //-- Quick calculation of max and min distances between keypoints
     for( int i = 0; i < descriptorT.rows; i++ )
     { double dist = matches[i].distance;
       if( dist < min_dist ) min_dist = dist;
       if( dist > max_dist ) max_dist = dist;
     }
     printf("-- Max dist : %f \n", max_dist );
     printf("-- Min dist : %f \n", min_dist );

     //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
     //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
     //-- small)
     //-- PS.- radiusMatch can also be used here.
     std::vector< cv::DMatch > good_matches;

     for( int i = 0; i < descriptorT.rows; i++ )
     {
         if( matches[i].distance <= std::max(2*min_dist, 0.02) )
       { good_matches.push_back( matches[i]); }
     }

    //-- Show detected matches
    //   cv::imshow( "Good Matches", img_matches );


    //   for( int i = 0; i < (int)good_matches.size(); i++ )
    //   {
    //       printf( "-- Good Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i, good_matches[i].queryIdx, good_matches[i].trainIdx );
    //    }
    bool better = false;
     if(good_matches.size() > Max){
         Max = good_matches.size();
         GoodMatches = good_matches;
         KeyPointsSource = keyS;
         KeyPointsTemp = keyT;
         better=true;
     }
     cout << better;
     return better;
}

QString HPC::FlannLoop( QString tempAddress , QString sourceAddress )
{
    QDir dir(sourceAddress);
    dir.setNameFilters(QStringList()<<"*.png"<< "*.jpg");
    QFileInfoList fileNames = dir.entryInfoList();
    cv::Mat temp  = cv::imread(tempAddress.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
    int max = 0;
    Max = 0;
    bool better = false;
    QString bestPage("");

    for(QFileInfo currentFile:fileNames){
        QString filePath = currentFile.filePath();
        cv::Mat source  = cv::imread(filePath.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
        std::cout << max;
        better = FLANN(source, temp);
        if(better)
        {
            bestPage = filePath;
        }
    }

    cout << "File name is " << bestPage.toStdString() << " and the number of good matches = " << Max;
    cout << "\n";

    return bestPage;
}




void HPC::on_match_clicked()
{
    //if(photos == "" && dir == "") QMessageBox::information(this,tr("Error"),tr("Album and photos not selected"));
    //else if(photos == "") QMessageBox::information(this,tr("Error"),tr("Photos not selected"));
    //else if(dir == "") QMessageBox::information(this,tr("Error"),tr("Album not selected"));
    int total = 0;
    int iterations = 1;
    QList<QFileInfoList> *listFileInfoList = new QList<QFileInfoList>;
    QStandardItemModel *smodel = new QStandardItemModel();
    QStandardItemModel *smodel2 = new QStandardItemModel();
    ui->matchProgress->setMaximum(total);


    for(int i = 0; i < dir->size(); i++)
    {
        QDir p(dir->value(i) + "/photos");
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.bmp" << "*.jpeg";
        QFileInfoList fileInfoList = p.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);
        total = total + fileInfoList.size();
        listFileInfoList->append(fileInfoList);
    }

    ui->matchProgress->setMaximum(total);

    for(int i = 0; i < dir->size(); i++)
    {
        QString as = dir->value(i) + "/album";
        QString ps = dir->value(i) + "/photos";
        QDir a(as);
        QDir p(ps);

        for(int j = 0; j < listFileInfoList->value(i).size(); j++)
        {
            //Do matching algorithm
            QFileInfo file = listFileInfoList->value(i).value(j);
            QString path = file.filePath();
            QStandardItem *item;
            QStandardItem *item2;
            QString index = QString::number(iterations);

            QString result  = HPC::FlannLoop(path, as);
            photoLink = path;
            pageLink = result;
            drawBoundingBox(photoLink,pageLink);
            QImage album_match = cvMatToQImage(match);
            QImage path_match(path);
            QSize size(300,300);
            album_match = album_match.scaled(size,Qt::KeepAspectRatio, Qt::FastTransformation);
            path_match = path_match.scaled(size,Qt::KeepAspectRatio, Qt::FastTransformation);
            item = new QStandardItem();
            item2 = new QStandardItem();
            item->setData(album_match,Qt::DecorationRole);
            item->setData(index,Qt::DisplayRole);
            item->setEditable(false);
            item2->setData(path_match,Qt::DecorationRole);
            item2->setData(index,Qt::DisplayRole);
            item2->setEditable(false);

            smodel->appendRow(item);
            smodel2->appendRow(item2);
            //ui->matchProgress->update();
            /*HPC::exportFile(dir+"/testOut.csv");// Current Directory.../HPC-test/build-HPC-Desktop_Qt_5_4_0_clang_64bit-Debug/HPC.app/Contents/MacOS
            foreach(QString x, dir) QTextStream(stdout)<< x;*/
            //ui->result->setPixmap(res);
            //ui->result->show();
            ui->matchProgress->setValue(ui->matchProgress->value() + 1);
            iterations++;
        }
    }
    ui->list_2->setModel(smodel);
    ui->list_4->setModel(smodel2);

}

void HPC::on_tree_clicked(const QModelIndex &index)
{
    currentDir = model->filePath(index);
    int index0 = ui->iconsize->currentIndex();
    if (index0 == 0) resizeimage(50,50);
    if (index0 == 1) resizeimage(75,75);
    if (index0 == 2) resizeimage(100,100);
    if (index0 == 3) resizeimage(150,150);
    if (index0 == 4) resizeimage(200,200);
    if (index0 == 5) resizeimage(300,300);
    if (index0 == 6) resizeimage(500,500);

    //std::string utf8_text = S.toStdString();
    //std::cout << utf8_text+"\n";
}

/*
void HPC::on_tree_2_clicked(const QModelIndex &index)
{
    photos = model->filePath(index);
    current = 1;
    int index0 = ui->iconsize->currentIndex();
    if (index0 == 0) resizeimage(50,50,current);
    if (index0 == 1) resizeimage(75,75,current);
    if (index0 == 2) resizeimage(100,100,current);
    if (index0 == 3) resizeimage(150,150,current);
    if (index0 == 4) resizeimage(200,200,current);
    if (index0 == 5) resizeimage(300,300,current);
    if (index0 == 6) resizeimage(500,500,current);
}*/

void HPC::on_iconsize_currentIndexChanged(const int &arg1)
{
    if (arg1 == 0) resizeimage(50,50);
    if (arg1 == 1) resizeimage(75,75);
    if (arg1 == 2) resizeimage(100,100);
    if (arg1 == 3) resizeimage(150,150);
    if (arg1 == 4) resizeimage(200,200);
    if (arg1 == 5) resizeimage(300,300);
    if (arg1 == 6) resizeimage(500,500);
}

void HPC::resizeimage(int height,int width)
{
    QStandardItemModel *smodel = new QStandardItemModel();
    QDir d;
    d = QDir(currentDir);

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.bmp" << "*.jpeg";
    QFileInfoList fileInfoList = d.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);

    for(int i = 0; i < fileInfoList.size(); i++)
    {
        QFileInfo file = fileInfoList.value(i);
        QString path = file.filePath();
        QString name = file.fileName();
        QStandardItem *item;
        QImage image(path);
        QSize size(height,width);
        image = image.scaled(size,Qt::KeepAspectRatio, Qt::FastTransformation);
        item = new QStandardItem();
        item->setData(name,Qt::DisplayRole );
        item->setData(image,Qt::DecorationRole );
        item->setEditable(false);
        smodel->appendRow(item);
    }
    ui->list->setModel(smodel);
    //else if(treeno == 1) ui->list_3->setModel(smodel);
}

void HPC::exportFile(QString outputFileName){
    QFile file(outputFileName);

    if (file.open(QFile::WriteOnly|QFile::Truncate))
    {
        QTextStream out(&file);
        out << "Photo, Page" << endl;
        out << photoLink <<", "<<pageLink << endl;
        file.close();
    }

}


void HPC::on_tree_doubleClicked(const QModelIndex &index)
{
    bool contains = false;
    int i;
    for(i = 0; i < dir->size(); i++) {
        if(dir->value(i) == model->filePath(index)) contains = true;
    }
    if(!contains){
        dir->append(model->filePath(index));
        displayAlbums();
    }
}

void HPC::on_selected_doubleClicked(const QModelIndex &index)
{
    dir->removeAt(index.row());
    displayAlbums();
}

void HPC::displayAlbums(){
    QStandardItemModel *smodel = new QStandardItemModel();
    for(int i = 0; i < dir->size(); i++)
    {
        QStandardItem *item = new QStandardItem();
        item->setData(dir->value(i),Qt::DisplayRole);
        item->setEditable(false);
        smodel->appendRow(item);
    }
    ui->selected->setModel(smodel);
}
