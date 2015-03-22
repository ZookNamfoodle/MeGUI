
#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <qfiledialog.h>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QStandardItem>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <opencv2/legacy/compat.hpp>
#include <opencv2/flann/flann.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <qfiledialog.h>
#include <QDir>
#include <QFile>
#include <QTextStream>


using namespace cv;
using namespace std;


namespace Ui {
class HPC;
}

class HPC : public QMainWindow
{
    Q_OBJECT

public:
    explicit HPC(QWidget *parent = 0);
    ~HPC();

private slots:

    void resizeimage(int height,int width,int treeno);

    void on_iconsize_currentIndexChanged(const int &arg1);

    void on_tree_clicked(const QModelIndex &index);

    void exportFile(QString outputFileName);

    QString FlannLoop( QString tempAddress , QString sourceAddress );

    void on_tree_2_clicked(const QModelIndex &index);

    void on_match_clicked();

private:
    Ui::HPC *ui;
};

