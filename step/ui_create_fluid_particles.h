#include <kdialog.h>
#include <klocale.h>

/********************************************************************************
** Form generated from reading ui file 'create_gas_particles.ui'
**
** Created: Wed May 6 00:48:34 2009
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CREATE_FLUID_PARTICLES_H
#define UI_CREATE_FLUID_PARTICLES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WidgetCreateFluidParticles
{
public:
    QVBoxLayout *vboxLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QLineEdit *lineEditVolume;
    QLabel *labelVolume;
    QLabel *label;
    QLineEdit *lineEditCount;
    QLabel *labelCount;
    QLabel *label_6;
    QLineEdit *lineEditConcentration;
    QLabel *labelConcentration;
    QLabel *label_2;
    QLineEdit *lineEditMass;
    QLabel *labelMass;
    QLabel *label_3;
    QLineEdit *lineEditTemperature;
    QLabel *labelTemperature;
    QLabel *label_4;
    QLineEdit *lineEditMeanVelocity;
    QLabel *labelMeanVelocity;

    void setupUi(QWidget *WidgetCreateFluidParticles)
    {
    if (WidgetCreateFluidParticles->objectName().isEmpty())
        WidgetCreateFluidParticles->setObjectName(QString::fromUtf8("WidgetCreateFluidParticles"));
    WidgetCreateFluidParticles->resize(277, 229);
    vboxLayout = new QVBoxLayout(WidgetCreateFluidParticles);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    groupBox = new QGroupBox(WidgetCreateFluidParticles);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    gridLayout = new QGridLayout(groupBox);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label_5 = new QLabel(groupBox);
    label_5->setObjectName(QString::fromUtf8("label_5"));

    gridLayout->addWidget(label_5, 0, 0, 1, 1);

    lineEditVolume = new QLineEdit(groupBox);
    lineEditVolume->setObjectName(QString::fromUtf8("lineEditVolume"));
    lineEditVolume->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    lineEditVolume->setReadOnly(true);

    gridLayout->addWidget(lineEditVolume, 0, 1, 1, 1);

    labelVolume = new QLabel(groupBox);
    labelVolume->setObjectName(QString::fromUtf8("labelVolume"));

    gridLayout->addWidget(labelVolume, 0, 2, 1, 1);

    label = new QLabel(groupBox);
    label->setObjectName(QString::fromUtf8("label"));

    gridLayout->addWidget(label, 1, 0, 1, 1);

    lineEditCount = new QLineEdit(groupBox);
    lineEditCount->setObjectName(QString::fromUtf8("lineEditCount"));
    lineEditCount->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(lineEditCount, 1, 1, 1, 1);

    labelCount = new QLabel(groupBox);
    labelCount->setObjectName(QString::fromUtf8("labelCount"));

    gridLayout->addWidget(labelCount, 1, 2, 1, 1);

    label_6 = new QLabel(groupBox);
    label_6->setObjectName(QString::fromUtf8("label_6"));

    gridLayout->addWidget(label_6, 2, 0, 1, 1);

    lineEditConcentration = new QLineEdit(groupBox);
    lineEditConcentration->setObjectName(QString::fromUtf8("lineEditConcentration"));
    lineEditConcentration->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(lineEditConcentration, 2, 1, 1, 1);

    labelConcentration = new QLabel(groupBox);
    labelConcentration->setObjectName(QString::fromUtf8("labelConcentration"));

    gridLayout->addWidget(labelConcentration, 2, 2, 1, 1);

    label_2 = new QLabel(groupBox);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    gridLayout->addWidget(label_2, 3, 0, 1, 1);

    lineEditMass = new QLineEdit(groupBox);
    lineEditMass->setObjectName(QString::fromUtf8("lineEditMass"));
    lineEditMass->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(lineEditMass, 3, 1, 1, 1);

    labelMass = new QLabel(groupBox);
    labelMass->setObjectName(QString::fromUtf8("labelMass"));

    gridLayout->addWidget(labelMass, 3, 2, 1, 1);

    label_3 = new QLabel(groupBox);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    gridLayout->addWidget(label_3, 4, 0, 1, 1);

    lineEditTemperature = new QLineEdit(groupBox);
    lineEditTemperature->setObjectName(QString::fromUtf8("lineEditTemperature"));
    lineEditTemperature->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(lineEditTemperature, 4, 1, 1, 1);

    labelTemperature = new QLabel(groupBox);
    labelTemperature->setObjectName(QString::fromUtf8("labelTemperature"));

    gridLayout->addWidget(labelTemperature, 4, 2, 1, 1);

    label_4 = new QLabel(groupBox);
    label_4->setObjectName(QString::fromUtf8("label_4"));

    gridLayout->addWidget(label_4, 5, 0, 1, 1);

    lineEditMeanVelocity = new QLineEdit(groupBox);
    lineEditMeanVelocity->setObjectName(QString::fromUtf8("lineEditMeanVelocity"));
    lineEditMeanVelocity->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout->addWidget(lineEditMeanVelocity, 5, 1, 1, 1);

    labelMeanVelocity = new QLabel(groupBox);
    labelMeanVelocity->setObjectName(QString::fromUtf8("labelMeanVelocity"));

    gridLayout->addWidget(labelMeanVelocity, 5, 2, 1, 1);


    vboxLayout->addWidget(groupBox);

    QWidget::setTabOrder(lineEditVolume, lineEditCount);
    QWidget::setTabOrder(lineEditCount, lineEditConcentration);
    QWidget::setTabOrder(lineEditConcentration, lineEditMass);
    QWidget::setTabOrder(lineEditMass, lineEditTemperature);
    QWidget::setTabOrder(lineEditTemperature, lineEditMeanVelocity);

    retranslateUi(WidgetCreateFluidParticles);

    QMetaObject::connectSlotsByName(WidgetCreateFluidParticles);
    } // setupUi

    void retranslateUi(QWidget *WidgetCreateFluidParticles)
    {
    groupBox->setTitle(tr2i18n("Options", 0));
    label_5->setText(tr2i18n("Area", 0));

#ifndef UI_QT_NO_TOOLTIP
    lineEditVolume->setToolTip(tr2i18n("The area the fluid takes", 0));
#endif // QT_NO_TOOLTIP

    labelVolume->setText(QString());
    label->setText(tr2i18n("Particle count:", 0));

#ifndef UI_QT_NO_TOOLTIP
    lineEditCount->setToolTip(tr2i18n("The number of particles of this fluid.", 0));
#endif // QT_NO_TOOLTIP


#ifndef UI_QT_NO_WHATSTHIS
    lineEditCount->setWhatsThis(tr2i18n("The number of particles of this fluid.", 0));
#endif // QT_NO_WHATSTHIS

    lineEditCount->setText(tr2i18n("20", 0));
    labelCount->setText(QString());
    label_6->setText(tr2i18n("Concentration:", 0));
    labelConcentration->setText(QString());
    label_2->setText(tr2i18n("Particle mass:", 0));

#ifndef UI_QT_NO_TOOLTIP
    lineEditMass->setToolTip(tr2i18n("The mass of one particle", 0));
#endif // QT_NO_TOOLTIP


#ifndef UI_QT_NO_WHATSTHIS
    lineEditMass->setWhatsThis(tr2i18n("The mass of one particle", 0));
#endif // QT_NO_WHATSTHIS

    lineEditMass->setText(tr2i18n("1", 0));
    labelMass->setText(QString());
    label_3->setText(tr2i18n("Temperature:", 0));
    lineEditTemperature->setText(tr2i18n("1e21", 0));
    labelTemperature->setText(QString());
    label_4->setText(tr2i18n("Mean velocity:", 0));
    lineEditMeanVelocity->setText(tr2i18n("(0,0)", 0));
    labelMeanVelocity->setText(QString());
    Q_UNUSED(WidgetCreateFluidParticles);
    } // retranslateUi

};

namespace Ui {
    class WidgetCreateFluidParticles: public Ui_WidgetCreateFluidParticles {};
} // namespace Ui

QT_END_NAMESPACE

#endif // CREATE_GAS_PARTICLES_H

