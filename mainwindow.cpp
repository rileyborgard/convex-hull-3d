#include "mainwindow.h"
#include <QSplitter>
#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include "convexhull3d.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    glWidget = new GLWidget(splitter);
    splitter->addWidget(glWidget);

    QWidget *formWidget = new QWidget(splitter);
    splitter->addWidget(formWidget);
    QFormLayout *formLayout = new QFormLayout;
    formWidget->setLayout(formLayout);

    QPushButton *loadPointsButton = new QPushButton("Load Points");
    connect(loadPointsButton, SIGNAL(clicked()), this, SLOT(onLoadPoints()));
    formLayout->addRow(loadPointsButton);

    setCentralWidget(splitter);
}

MainWindow::~MainWindow() {
}

void MainWindow::onLoadPoints() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Point Cloud"), "/", tr("Point Cloud (*.xyz)"));
    QFile file(fileName);
    std::vector<QVector3D> points;
    if(file.open(QFile::ReadOnly)) {
        QTextStream in(&file);
        QString line;
        while(in.readLineInto(&line)) {
            if (line.isEmpty() || line[0] == '#') {
                continue;
            }
            QStringList list = line.split(' ');
            std::array<double, 3> coords;
            int index = 0;
            for (int i = 0; i < list.length() && index < 3; i++) {
                if (list[i].isEmpty()) {
                    continue;
                }
                list[i].remove(',');
                coords[index++] = list[i].toDouble();
            }
            if (index < 3) {
                continue;
            }
            points.push_back(QVector3D(coords[0], coords[1], coords[2]));
        }
    }

    Mesh *mesh = ConvexHull3D::compute(points);
    glWidget->setMesh(mesh);
}
