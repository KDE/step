/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "undobrowser.h"

#include "worldmodel.h"

#include <QIcon>
#include <QUndoView>
#include <QUrl>

#include <KLocalizedString>

UndoBrowser::UndoBrowser(WorldModel* worldModel, QWidget* parent)
    : QDockWidget(i18n("Undo History"), parent), _worldModel(worldModel)
{
    _undoView = new QUndoView(_worldModel->undoStack(), this);
    setWidget(_undoView);
}

void UndoBrowser::setEmptyLabel(const QString& label)
{
    _undoView->setEmptyLabel(label);
}

void UndoBrowser::setCurrentFileUrl(const QUrl& url)
{
    if (url.isEmpty())
        _undoView->setCleanIcon(QIcon());
    else
        _undoView->setCleanIcon(QIcon::fromTheme(QStringLiteral("document-save")));
}

void UndoBrowser::setUndoEnabled(bool enabled)
{
    _undoView->setEnabled(enabled);
}

#include "moc_undobrowser.cpp"
