#include "multipleinputdialog.h"
#include "ui_multipleinputdialog.h"
#include "QLabel"
#include "QLineEdit"

MultipleInputDialog::MultipleInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultipleInputDialog)
{
    ui->setupUi(this);
    fieldsCount = 0;
}

MultipleInputDialog::~MultipleInputDialog()
{
    fieldList.clear();
    delete ui;
}

void MultipleInputDialog::on_buttonBox_accepted()
{
    QDialog::accept();

}

QVariant MultipleInputDialog::getValue(QString name)
{
    int i,pos;
    for(i=0;i<fieldsCount;i++)
        if(((QLineEdit*)fieldList.at(i))->accessibleName().compare(name) == 0)
        {
            QLineEdit * textBox =(QLineEdit*)fieldList.at(i);
            QString text = textBox->text();
            if((pos = text.indexOf("0x")) >= 0)
            {
                bool ok;
                long hexVal = text.right(text.length() - pos).toULong(&ok,16);// text.replace("0x","").toULong(&ok,16);
                if(ok)
                    return QVariant((qlonglong)hexVal);
            }
            return QVariant(textBox->text());
        }
    return QVariant();
}

void MultipleInputDialog::addField(QString name, QVariant defaultValue)
{
    ui->uiLayout->insertWidget(fieldsCount*2, new QLabel(name));
    QLineEdit * textBox =new QLineEdit();
//    textBox->set
    textBox->setAccessibleName(name);
    textBox->setText(defaultValue.toString());

    ui->uiLayout->insertWidget(fieldsCount*2+1,textBox);
    fieldList.append(textBox);
    fieldsCount++;
}

