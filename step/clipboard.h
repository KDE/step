/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit Clipboard(QObject* parent = nullptr);
    
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
