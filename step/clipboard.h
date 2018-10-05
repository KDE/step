/* This file is part of Step.
 *   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
 * 
 *   Step is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 * 
 *   Step is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with Step; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef STEP_CLIPBOARD_H
#define STEP_CLIPBOARD_H

#include <QList>
#include <QObject>

namespace StepCore
{
    class Factory;
    class Item;
}

class Clipboard : public QObject
{
public:
    explicit Clipboard(QObject* parent = 0);
    
    void copy(const QList<StepCore::Item*>& items);
    QList<StepCore::Item*> paste(const StepCore::Factory* factory);
    
    bool canPaste() const { return _canPaste; }
    
signals:
    void canPasteChanged(bool value);
    
private slots:
    void dataChanged();
    
private:
    bool hasData() const;
    
    bool _canPaste;
    
    Q_OBJECT
};

#endif
