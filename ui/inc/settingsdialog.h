#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "QListWidgetItem"
#include "QStackedWidget"
#include "generalsettingspage.h"

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_CloseButton_clicked();

signals:
    void onAccept();
    void onClose();

private:
    Ui::SettingsDialog *ui;
    QStackedWidget *stackedWidget;    
    GeneralSettingsPage *generalSettings;
};

#endif // SETTINGSDIALOG_H
