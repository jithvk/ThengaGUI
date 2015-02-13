#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include <QWidget>
#include "QListWidgetItem"

namespace Ui {
    class GeneralSettingsPage;
}

class GeneralSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralSettingsPage(QWidget *parent = 0);
    ~GeneralSettingsPage();


private slots:


    void on_colorSchemeList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_keyBindingList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::GeneralSettingsPage *ui;
};

#endif // GENERALSETTINGSPAGE_H
