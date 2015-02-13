#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "QListWidgetItem"


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    stackedWidget = new QStackedWidget;    
    generalSettings = new GeneralSettingsPage;
    stackedWidget->addWidget(generalSettings);    
    ui->verticalLayout->insertWidget(0,stackedWidget);

//    QObject::connect(this,SIGNAL(accepted()),generalSettings,SLOT(onAccepted()));

}

SettingsDialog::~SettingsDialog()
{

    delete generalSettings;
    delete ui;
}

void SettingsDialog::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    stackedWidget->setCurrentIndex(ui->listWidget->row(current));
}

//void SettingsDialog::on_buttonBox_accepted()
//{

//    QWidget *current = stackedWidget->currentWidget();
//    if(current == generalSettings)
//        generalSettings->onAccepted();
////    emit accepted();

//}

void SettingsDialog::on_CloseButton_clicked()
{
    close();
}
