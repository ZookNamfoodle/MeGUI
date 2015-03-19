#include "HPC.h"
#include "ui_HPC.h"

QString photoLink = "";
QString pageLink = "";
QString fileName = "";
QString dir = "";
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
    model->setRootPath(QDir::currentPath());
    ui->tree->setModel(model);
    ui->tree->hideColumn(1);
    ui->tree->hideColumn(2);
    ui->tree->hideColumn(3);
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

     double max_dist = 0; double min_dist = 25;

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
    return bestPage;
}


void HPC::on_selectPhoto_clicked()
{
    fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));

    ui->loadedPhoto->setPixmap(fileName);
    ui->loadedPhoto->show();
}

void HPC::on_match_clicked()
{
    if(fileName == "" && dir == "") QMessageBox::information(this,tr("Error"),tr("No template or directory selected"));
    else if(fileName == "") QMessageBox::information(this,tr("Error"),tr("No template selected"));
    else if(dir == "") QMessageBox::information(this,tr("Error"),tr("No directory selected"));

    //Do matching algorithm

    QString result  = HPC::FlannLoop(fileName, dir);
    photoLink = fileName;
    pageLink = result;
    drawBoundingBox(photoLink,pageLink);
    QPixmap res = cvMatToQPixmap(match);

    HPC::exportFile(dir+"/testOut.csv");// Current Directory.../HPC-test/build-HPC-Desktop_Qt_5_4_0_clang_64bit-Debug/HPC.app/Contents/MacOS
    foreach(QString x, dir) QTextStream(stdout)<< x;
    ui->result->setPixmap(res);
    ui->result->show();

}

void HPC::on_tree_clicked(const QModelIndex &index)
{
    dir = model->filePath(index);
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
    QDir d(dir);
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
