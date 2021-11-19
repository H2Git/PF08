#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"

// for tcp clnt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

// vewi01 login page

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->inputId->setStyleSheet("padding-left: 10px;");
    ui->inputPw->setStyleSheet("padding-left: 10px;");
    ui->signUpEmail->setStyleSheet("padding-left: 10px;");
    ui->signUpPw->setStyleSheet("padding-left: 10px;");
    ui->signUpPwCheck->setStyleSheet("padding-left: 10px;");
    ui->signUpNickname->setStyleSheet("padding-left: 10px;");

    ui->inputLocation->setStyleSheet("padding-left: 10px;");

    QPixmap mainLogoPic(":/img/62358.jpg");
    ui->mainLogo->setPixmap(mainLogoPic);
    ui->mainLogo->setPixmap(mainLogoPic.scaled(460,740, Qt::KeepAspectRatio));

    QPixmap menuPic(":/img/62359.jpg");
    ui->label_15->setPixmap(menuPic);
    ui->label_15->setPixmap(menuPic.scaled(460,740, Qt::KeepAspectRatio));

    ui->label_17->setPixmap(menuPic);
    ui->label_17->setPixmap(menuPic.scaled(460,740, Qt::KeepAspectRatio));

    ui->label_18->setPixmap(menuPic);
    ui->label_18->setPixmap(menuPic.scaled(460,740, Qt::KeepAspectRatio));

    QPixmap menu(":/img/menu.png");
    ui->label_20->setPixmap(menu);
    ui->label_20->setPixmap(menu.scaled(30, 30, Qt::KeepAspectRatio));

    QPixmap add(":/img/add.png");
    ui->label_21->setPixmap(add);
    ui->label_21->setPixmap(add.scaled(30, 30, Qt::KeepAspectRatio));

    QPixmap more(":/img/more.png");
    ui->label_22->setPixmap(more);
    ui->label_22->setPixmap(more.scaled(30, 30, Qt::KeepAspectRatio));

        ui->frame_2->setVisible(false);
        ui->frame_3->setVisible(false);
        ui->frame_4->setVisible(false);


    if ((clnt_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        qDebug() <<  "socket() fail";
        exit(1);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(atoi("3000"));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // IPv6 inet_pton ok, inet_aton() X, inet_addr() X
    ::connect(clnt_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_3_clicked()
{
    id = ui->inputId->text();
    pw = ui->inputPw->text();

    if (id == "test" && pw == "test") {
        QMessageBox::information(this, "Login", "ID and Passwod is correct :) Welcome");

        ui->frame_3->setVisible(true);

        c_tm = time(NULL);
        tm = *localtime(&c_tm);
        sprintf(buf_all,
                "%d %s %s %s [%02d:%02d:%02d] ",
                2,
                (const char*)id.toStdString().c_str(),
                (const char*)pw.toStdString().c_str(),
                "customer",
                tm.tm_hour, tm.tm_min, tm.tm_sec);

        qDebug() << buf_all; // check buf

        if (send(clnt_sock, buf_all, strlen(buf_all), 0) < 0)
            puts("Error : Write error on socket.");

    }
    else if (id == "manager" && pw == "manager")
    {

        QMessageBox::information(this, "Login", "ID and Passwod is correct :) Welcome");
        //ui->frame->setVisible(false);
        ui->frame_4->setVisible(true);

        c_tm = time(NULL);
        tm = *localtime(&c_tm);
        sprintf(buf_all,
                "%d %s %s %s [%02d:%02d:%02d] ",
                3,
                (const char*)id.toStdString().c_str(),
                (const char*)pw.toStdString().c_str(),
                "manager",
                tm.tm_hour, tm.tm_min, tm.tm_sec);

        qDebug() << buf_all; // check buf

        if (send(clnt_sock, buf_all, strlen(buf_all), 0) < 0)
            puts("Error : Write error on socket.");


    }
    else
    {
        QMessageBox::information(this, "Login", "ID and Passwod is incorrect T.T");
    }

    // T.T no checking logic of signup cuz time limits


}


void MainWindow::on_pushButton_5_clicked()
{
    //ui->frame->setVisible(false);
    ui->frame_2->setVisible(true);
}


void MainWindow::on_signUpButton_clicked()
{
    // T.T no checking logic of signup cuz time limits

    QString email = ui->signUpEmail->text();
    QString pw = ui->signUpPw->text();
    QString nickname = ui->signUpNickname->text();

    c_tm = time(NULL);
    tm = *localtime(&c_tm);
    sprintf(buf_all,
            "%d %s %s %s [%02d:%02d:%02d] ",
            1,
            (const char*)email.toStdString().c_str(),
            (const char*)pw.toStdString().c_str(),
            (const char*)nickname.toStdString().c_str(),
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    qDebug() << buf_all; // check buf

    if (send(clnt_sock, buf_all, strlen(buf_all), 0) < 0)
        puts("Error : Write error on socket.");


    //ui->frame_2->setVisible(false);
    ui->frame_3->setVisible(true);
}


void MainWindow::on_led_clicked()
{
    QMessageBox::information(this, "LED Start", "LED가 켜집니다.");

    c_tm = time(NULL);
    tm = *localtime(&c_tm);

    sprintf(buf_all,
            "%d %s %s %s [%02d:%02d:%02d] %s ",
            5,
            (const char*)id.toStdString().c_str(),
            (const char*)pw.toStdString().c_str(),
            "customer",
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            "LED");

    qDebug() << buf_all; // check buf

    if (send(clnt_sock, buf_all, strlen(buf_all), 0) < 0)
        puts("Error : Write error on socket.");

}


void MainWindow::on_speaker_clicked()
{
    QMessageBox::information(this, "Speaker Start", "Speaker가 켜집니다.");

    c_tm = time(NULL);
    tm = *localtime(&c_tm);
    sprintf(buf_all,
            "%d %s %s %s [%02d:%02d:%02d] %s",
            6,
            (const char*)id.toStdString().c_str(),
            (const char*)pw.toStdString().c_str(),
            "customer",
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            "Speaker");

    qDebug() << buf_all; // check buf

    if (send(clnt_sock, buf_all, strlen(buf_all), 0) < 0)
        puts("Error : Write error on socket.");

}


void MainWindow::on_lcd_clicked()
{
    QMessageBox::information(this, "LCD Start", "LCD가 켜집니다.");

    c_tm = time(NULL);
    tm = *localtime(&c_tm);
    sprintf(buf_all,
            "%d %s %s %s [%02d:%02d:%02d] %s",
            7,
            (const char*)id.toStdString().c_str(),
            (const char*)pw.toStdString().c_str(),
            "customer",
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            "LCD");

    qDebug() << buf_all; // check buf

    if (send(clnt_sock, buf_all, strlen(buf_all), 0) < 0)
        puts("Error : Write error on socket.");
}

void MainWindow::on_pushButton_12_clicked()
{
    c_tm = time(NULL);
    tm = *localtime(&c_tm);
    sprintf(buf_all,
            "%d %s %s %s [%02d:%02d:%02d] ",
            4,
            "manager",
            "manager",
            "manager",
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    qDebug() << buf_all; // check buf

    if (send(clnt_sock, buf_all, strlen(buf_all), 0) < 0)
        puts("Error : Write error on socket.");

    int nbyte;
    nbyte = recv(clnt_sock, buf_msg, MAX_CHAT, 0);
    buf_msg[nbyte] = 0;
    //printf("%s\n", buf_msg);

    ui->textEdit->append(buf_msg);

}


