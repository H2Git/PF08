#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <arpa/inet.h>

// vewi01 login page
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    #define MAX_CHAT 1000 // max chatting length
    #define MAX_NAME 23	 // max nickname length

    char buf_msg[MAX_CHAT];			   // each chatting message buffer
    char buf_all[MAX_CHAT + MAX_NAME]; // total string for send to serv
    int select_fds;					   // sockets for select maxfd
    int clnt_sock;
    fd_set read_fds;
    time_t c_tm;
    struct tm tm;

    QString id;
    QString pw;


    const char *exit_strings = "/EXIT /Exit /exit /q /Q "; // strings for exit, check one of them

   private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_signUpButton_clicked();

    void on_led_clicked();

    void on_speaker_clicked();

    void on_lcd_clicked();


    void on_pushButton_12_clicked();

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
