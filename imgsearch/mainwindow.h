#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <imgdb.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    ImgDB *testDB;
    QString diffFormat(float difference);

private slots:
    void on_tableDuplicates_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_btnSearchPath_clicked();
    void on_btnImage_clicked();
    void on_btnFindDuplicates_clicked();
};

#endif // MAINWINDOW_H
