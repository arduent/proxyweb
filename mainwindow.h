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
    /* go ahead and load the URL when the button is clicked */
    void on_goButton_clicked();
    /* load the URL when the user clicks on a row in the history table */
    void on_historyTable_clicked(const QModelIndex &index);
    /* wipe the history table */
    void on_clearHistoryButton_clicked();
    /* update the config database table when the proxy host value is edited */
    void on_proxyUrlEdit_textChanged(const QString &arg1);
    /* update the config database table when the proxy port value is edited */
    void on_proxyPortEdit_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    /* a model connected to the database linked to the table widget */
    QSqlTableModel *model;
    /* the db connection */
    QSqlDatabase db;
    /* config key/value hash that is not yet necessary */
    QHash<QString, QString> config;
    /* proxy info */
    QNetworkProxy proxy;
    /* we set loading = true during startup so the update triggers are not executed */
    bool loading;
};
#endif // MAINWINDOW_H
