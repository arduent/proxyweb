#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkProxy>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QStandardPaths>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* don't process updates when we are updating */
    loading = true;

    /* store sqlite3 database file in first available user app data path, defined by system */
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    QString dbpath = paths[0] + "/browser.sqlite3";

    /* Load SQLite driver - on FreeBSD the driver is qt5-sqldrivers-sqlite3 port */
    db = QSqlDatabase::addDatabase ("QSQLITE");
    db.setDatabaseName(dbpath);
    if (!db.open())
    {
        /* do not have write access to database file or did not load SQLite driver */
        QString err = db.lastError().text();
        QMessageBox msgBox;
        msgBox.setText("Could not open database. " + err);
        msgBox.exec();
        qApp->exit();
    }

    QSqlQuery query;
    int numRows;
    /* check if history table already exists */
    query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='history'");
    /* SQLite Driver does not support query.size() function so we have to get numrows less efficient way */
    query.last();
    numRows = query.at() + 1;

    if (numRows<1)
    {
        /* if new file then create history and config tables and insert initial data */
        query.exec("CREATE TABLE IF NOT EXISTS history (url TEXT, last_update INTEGER, count INTEGER)");
        query.exec("CREATE TABLE IF NOT EXISTS config (skey TEXT, svalue TEXT)");
        query.exec("INSERT INTO config (skey,svalue) VALUES ('proxyhost','')");
        /* default squid cache port */
        query.exec("INSERT INTO config (skey,svalue) VALUES ('proxyport','3128')");
        query.exec("INSERT INTO config (skey,svalue) VALUES ('lasturl','')");

    }

    /* connect database model (history table) to historyTable model */
    model = new QSqlTableModel(parent, db);
    model->setTable("history");
    model->select();
    ui->historyTable->setModel(model);
    /* expand columns to fill */
    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    /* enable user column sort by clicking on header */
    ui->historyTable->setSortingEnabled(true);
    /* disable editing / make table read-only */
    ui->historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    /* load config into hash. the hash array isn't actually useful at the moment but may be used in the future */
    query.exec("SELECT * FROM config");
    while (query.next())
    {
           QString skey = query.value(0).toString();
           QString sval = query.value(1).toString();
           config[skey]=sval;
    }
    /* load proxy host into lineedit */
    if (config.contains("proxyhost"))
    {
        ui->proxyUrlEdit->setText(config["proxyhost"]);
    }
    /* load proxy port into lineedit */
    if (config.contains("proxyport"))
    {
        ui->proxyPortEdit->setText(config["proxyport"]);
    }
    /* load last loaded url into lineedit */
    if (config.contains("lasturl"))
    {
        ui->urlEdit->setText(config["lasturl"]);
        on_goButton_clicked();
    }
    /* allow future updates to be processed and stored in database */
    loading = false;
}

MainWindow::~MainWindow()
{
    /* bye, bye */
    delete ui;
}


void MainWindow::on_goButton_clicked()
{
    /* only load URL if proxy fields have values. does not verify proxy actually works, or validate host and port */
    if ((ui->proxyUrlEdit->text()=="")||(ui->proxyPortEdit->text()==""))
    {
        QMessageBox msgBox;
        msgBox.setText("No Proxy Set");
        msgBox.exec();
    }
    else
    {
        /* sets proxy each time a url is loaded, at the moment it does not appear to cause delay.
         * if noticeable delay we need to only set proxy if changed or new values */

        try {
            proxy.setType(QNetworkProxy::HttpProxy);
            proxy.setHostName(ui->proxyUrlEdit->text());
            proxy.setPort(ui->proxyPortEdit->text().toInt()); //this could crash the program if not an int
            proxy.setCapabilities(QNetworkProxy::HostNameLookupCapability);
            QNetworkProxy::setApplicationProxy(proxy);
        }
        catch (...)
        {
            /* tell the user that their proxy info is not working and bail */
            QMessageBox msgBox;
            msgBox.setText("Problem with proxy settings. Check host and port.");
            msgBox.exec();
            return;
        }
    }

    /* load the URL in the browser. does not validate or verify URL. */
    ui->webEngineView->load(QUrl(ui->urlEdit->text()));

    /* store the loaded url in database config table. will load this url on next application run */
    QSqlQuery query;
    query.prepare("UPDATE config SET svalue=? WHERE skey='lasturl'");
    query.addBindValue(ui->urlEdit->text());
    query.exec();

    /* check if url is already in history table */
    int numRows;
    query.prepare("SELECT rowid FROM history WHERE url=?");
    query.addBindValue(ui->urlEdit->text());
    query.exec();
    /* SQLite Driver does not support query.size() function so we have to get numrows less efficient way */
    query.last();
    numRows = query.at() + 1;

    if (numRows<1)
    {
        /* a new url to add to history table.
            using epoch timestamps. perhaps not so human friendly - could be better $TODO */
        query.prepare("INSERT INTO history (url,last_update,count) VALUES (?, ?, 1)");
        query.addBindValue(ui->urlEdit->text());
        query.addBindValue(QDateTime::currentSecsSinceEpoch());
        if (!query.exec())
        {
            qDebug() << query.lastError();
        }
    }
    else
    {
        /* url is already in history table. update last load date and increment count
         * this gives user more interesting column sort options on history table */
        query.prepare("UPDATE history SET last_update=?, count=count+1 WHERE url=?");
        query.addBindValue(QDateTime::currentSecsSinceEpoch());
        query.addBindValue(ui->urlEdit->text());
        if (!query.exec())
        {
            qDebug() << query.lastError();
        }
    }
    model->select();
}

void MainWindow::on_historyTable_clicked(const QModelIndex &index)
{
    /* if the user clicks on a row in history table, load that URL into the browser */
    if (index.isValid())
    {
        QString url = ui->historyTable->model()->data(ui->historyTable->model()->index(index.row(),0)).toString();
        ui->urlEdit->setText(url);
    }
    on_goButton_clicked();
}

void MainWindow::on_clearHistoryButton_clicked()
{
    /* confirm and wipe history table. also empties current URL lineedit */
    QMessageBox msgBox;
    msgBox.setText("Confirm history delete.");
    msgBox.setInformativeText("Do you want to clear your history?");
    msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Discard);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Discard)
    {
            QSqlQuery query;
            query.exec("DELETE FROM history");
            ui->urlEdit->setText("");
            model->select();
    }
}

void MainWindow::on_proxyUrlEdit_textChanged(const QString &arg1)
{
    /* if the proxy host is changed then store in config table to remember it for next program run */

    /* if application startup is happening then don't update the database */
    if (loading) return;

    QSqlQuery query;
    query.prepare("UPDATE config SET svalue=? WHERE skey=?");
    query.addBindValue(arg1);
    query.addBindValue("proxyhost");
    if (!query.exec())
    {
        qDebug() << query.lastError();
    }
}

void MainWindow::on_proxyPortEdit_textChanged(const QString &arg1)
{
    /* if the proxy port is changed then store in config table to remember it for next program run */

    /* if application startup is happening then don't update the database */
    if (loading) return;

    QSqlQuery query;
    query.prepare("UPDATE config SET svalue=? WHERE skey=?");
    query.addBindValue(arg1);
    query.addBindValue("proxyport");
    if (!query.exec())
    {
        qDebug() << query.lastError();
    }
}
