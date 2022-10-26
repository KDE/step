/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_GASCREATIONDIALOG_H
#define STEP_GASCREATIONDIALOG_H

#include <QDialog>
//#include "gasgraphics.h"


namespace StepCore {
    class Gas;
}
namespace Ui {
    class WidgetCreateGasParticles;
}
class GasMenuHandler;


class GasCreationDialog: public QDialog
{
    Q_OBJECT
    
public:
    GasCreationDialog(GasMenuHandler* handler, StepCore::Gas *gas,
		      QWidget *parent=nullptr);

    Ui::WidgetCreateGasParticles *ui();

protected:
    Ui::WidgetCreateGasParticles *_ui;

    StepCore::Gas                *_gas;
    GasMenuHandler               *_handler;
};

#endif
