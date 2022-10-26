/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_CHOICESMODEL_H
#define STEP_CHOICESMODEL_H

#include <QStandardItemModel>

class ChoicesModel: public QStandardItemModel
{
  Q_OBJECT
  
public:
    explicit ChoicesModel(QObject* parent = nullptr): QStandardItemModel(parent) {}
};

Q_DECLARE_METATYPE(ChoicesModel*)

#endif
