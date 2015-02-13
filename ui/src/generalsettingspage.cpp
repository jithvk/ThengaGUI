
#include "generalsettingspage.h"
#include "ui_generalsettingspage.h"
#include "QSettings"
#include "mainwindow.h"
#include "QFileDialog"
#include "detailedlog.h"

#include "qtermwidget.h"

GeneralSettingsPage::GeneralSettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralSettingsPage)
{
    ui->setupUi(this);
    ui->colorSchemeList->addItems(QTermWidget::availableColorSchemes());

    ui->keyBindingList->addItems(QTermWidget::availableKeyBindings());



}

GeneralSettingsPage::~GeneralSettingsPage()
{
    delete ui;
}



void GeneralSettingsPage::on_colorSchemeList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QString color_scheme = current->text();
    MainWindow::setApplicationSettings("ColorScheme",color_scheme);

}

void GeneralSettingsPage::on_keyBindingList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QString keyBinding = current->text();
    MainWindow::setApplicationSettings("KeyBinding",keyBinding);
}
