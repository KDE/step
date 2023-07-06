/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gascreationdialog.h"

#include <float.h>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <stepcore/gas.h>

#include "gasgraphics.h"
#include "ui_create_gas_particles.h"


GasCreationDialog::GasCreationDialog(GasMenuHandler* handler, StepCore::Gas *gas,
				     QWidget *parent)
    : QDialog(parent)
    , _gas(gas)
    , _handler(handler)
{
    setWindowTitle(i18nc("@title:window", "Create Gas Particles"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);

    // Create the actual UI.
    _ui = new Ui::WidgetCreateGasParticles;
    _ui->setupUi(mainWidget);

    // Create validators for all input fields.
    _ui->lineEditMass->setValidator(
	    new QDoubleValidator(0, HUGE_VAL, DBL_DIG,
				 _ui->lineEditMass));
    _ui->lineEditCount->setValidator(
	     new QIntValidator(0, INT_MAX,
			       _ui->lineEditCount));
    _ui->lineEditConcentration->setValidator(
             new QDoubleValidator(0, HUGE_VAL, DBL_DIG,
				  _ui->lineEditConcentration));
    _ui->lineEditTemperature->setValidator(
	     new QDoubleValidator(0, HUGE_VAL, DBL_DIG,
				  _ui->lineEditTemperature));
    _ui->lineEditMeanVelocity->setValidator(
	     new QRegularExpressionValidator(QRegularExpression("^\\([+-]?\\d+(\\.\\d*)?([eE]\\d*)?,[+-]?\\d+(\\.\\d*)?([eE]\\d*)?\\)$"),
				  _ui->lineEditMeanVelocity));

    _ui->lineEditVolume->setText(QString::number(_gas->rectVolume()));
}


Ui::WidgetCreateGasParticles *GasCreationDialog::ui()
{
    return _ui;
}

#include "moc_gascreationdialog.cpp"
