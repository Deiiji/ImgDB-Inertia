#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QString>
#include <QHash>
#include <QTextStream>
#include <QDir>
#include <QDirIterator>
#include <QPixmap>
#include <QTime>
#include <Qt>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    testDB(new ImgDB())

{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete testDB;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_btnImage_clicked()
{
    QString strFilename = QFileDialog::getOpenFileName(this, tr("Choose Search Path"),
                                                     QDir::currentPath(),
                                                     tr("Images (*.png *.xpm *.jpg)"));

    QImage image;

    if(!strFilename.isNull())
    {
        if (image.load(strFilename))
        {
            ui->fileImage->setText(strFilename);

            image = image.scaled(QSize(251, 191), Qt::KeepAspectRatio);

            ui->imgImage->setPixmap(QPixmap::fromImage(image));
            ui->imgImage->show();
        }
    }
}

void MainWindow::on_btnFindDuplicates_clicked()
{
    float threshold = 0.016;

    if(!ui->fileImage->text().isEmpty() && !ui->fileSearchPath->text().isEmpty())
    {
	ui->tableDuplicates->setRowCount(0);

	ui->tableDuplicates->clearContents();

        QDirIterator directory_walker(ui->fileSearchPath->text(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

        testDB->initDbase(1);

        testDB->addImage(1, 1, (char*)ui->fileImage->text().toStdString().c_str());

        int imgIdent = 1;
	int dupeNum = 0;

        while(directory_walker.hasNext())
        {
            directory_walker.next();

            if(directory_walker.fileInfo().suffix() == "jpg")
            {
                // then we take a filename and display it to a listWidget like the code below:
                testDB->addImage(1, ++imgIdent, (char*)directory_walker.filePath().toStdString().c_str());
		float difference = testDB->calcAvglDiff(1, 1, imgIdent);

                if(difference < threshold && directory_walker.filePath() != ui->fileImage->text())
                {
		    ui->tableDuplicates->insertRow(dupeNum);
		    ui->tableDuplicates->setItem(dupeNum, 0, new QTableWidgetItem(directory_walker.filePath()));
		    ui->tableDuplicates->setItem(dupeNum, 1, new QTableWidgetItem(diffFormat(difference)));

		    ++dupeNum;
                }
            }
        }

        testDB->closeDbase();
    }
}

void MainWindow::on_btnSearchPath_clicked()
{
    QString strFilename = QFileDialog::getExistingDirectory(this, tr("Open File"),
                                                     QDir::currentPath());

    if(!strFilename.isNull())
    {
        ui->fileSearchPath->setText(strFilename);
    }
}

QString MainWindow::diffFormat(float difference)
{
    QString quickOutput;
    QTextStream quickOutputStream(&quickOutput, QIODevice::ReadWrite);
    quickOutputStream.setRealNumberNotation(QTextStream::FixedNotation);
    quickOutputStream.setRealNumberPrecision(2);

    quickOutputStream << 100.0 - (difference * 100.0);

    return(quickOutput);
}

void MainWindow::on_tableDuplicates_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if(currentRow != previousRow) //we don't really care what column was picked
    {
	QImage image;
	if(currentRow != -1 && image.load(ui->tableDuplicates->item(currentRow, 0)->text()))
	{
	    image = image.scaled(QSize(251, 191), Qt::KeepAspectRatio);

	    ui->imgDuplicate->setPixmap(QPixmap::fromImage(image));
	    ui->imgDuplicate->show();
	} else {
	    ui->imgDuplicate->clear();
	}
    }
}
