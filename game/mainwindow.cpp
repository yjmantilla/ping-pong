#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()//boton que configura el jugo como single player y muestra la ventana de este.
{
     S = new Game_Window(0,"",false,false,1,true,1,7);
     S->show();
     close();
}

void MainWindow::on_pushButton_2_clicked()//boton que configura el jugo como multiplayer y muestra la ventana de este.
{
   M = new Game_Window(0,"",false,false,1,false,1,7);
   M->show();
   close();
}


//solo bots
//cargar partida
