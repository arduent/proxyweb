#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QHash>
#include <QNetworkProxy>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_goButton_clicked();
    void on_historyTable_clicked(const QModelIndex &index);
    void on_clearHistoryButton_clicked();
    void on_proxyUrlEdit_textChanged(const QString &arg1);
    void on_proxyPortEdit_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *model;
    QSqlDatabase db;
    QHash<QString, QString> config;
    QNetworkProxy proxy;
    bool loading;
};
#endif // MAINWINDOW_H
