/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "infobrowser.h"

#include "worldmodel.h"
#include "settings.h"

#include <QAction>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QIcon>
#include <QItemSelectionModel>
#include <QStandardPaths>
#include <QVBoxLayout>

#include <KIO/Job>
#include <KLocalizedString>
#include <KToolBar>

InfoBrowser::InfoBrowser(WorldModel* worldModel, QWidget* parent)
    : QDockWidget(i18n("Context info"), parent),
      _worldModel(worldModel), _selectionChanged(false)
{
    QWidget* widget = new QWidget(this);
    setWidget(widget);

    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    _toolBar = new KToolBar(widget);
    layout->addWidget(_toolBar);
    _toolBar->setMovable(false);
    _toolBar->setFloatable(false);
    _toolBar->setIconDimensions(16);
    _toolBar->setContextMenuPolicy(Qt::NoContextMenu);
    _toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    _backAction = _toolBar->addAction(QIcon::fromTheme(QStringLiteral("go-previous")), i18n("Back"), this, SLOT(back()));
    _backAction->setEnabled(false);
    _forwardAction = _toolBar->addAction(QIcon::fromTheme(QStringLiteral("go-next")), i18n("Forward"), this, SLOT(forward()));
    _forwardAction->setEnabled(false);

    _toolBar->addSeparator();
    _syncAction = _toolBar->addAction(QIcon::fromTheme(QStringLiteral("goto-page")), i18n("Sync selection"), this, SLOT(syncSelection())); // XXX: icon
    _syncAction->setEnabled(false);
    _followAction = _toolBar->addAction(QIcon::fromTheme(QStringLiteral("note2")), i18n("Follow selection")/*, this, SLOT(syncSelection(bool))*/); // XXX: icon
    _followAction->setCheckable(true);
    _followAction->setChecked(true);

    _toolBar->addSeparator();
    _execAction = _toolBar->addAction(QIcon::fromTheme(QStringLiteral("system-run")), i18n("Open in browser"), this, SLOT(openInBrowser()));
    _execAction->setEnabled(false);

    _htmlBrowser = new QTextBrowser(widget);
    _htmlBrowser->setOpenLinks(false);
    layout->addWidget(_htmlBrowser);

    connect(_htmlBrowser, &QTextBrowser::anchorClicked, this, [=](const QUrl &url){
        openUrl(url);
    });

    connect(_worldModel->selectionModel(), &QItemSelectionModel::currentChanged,
                                           this, &InfoBrowser::worldCurrentChanged);

    syncSelection();
}

void InfoBrowser::showEvent(QShowEvent* event)
{
    QDockWidget::showEvent(event);
    if(_selectionChanged) {
        _selectionChanged = false;
        QModelIndex current = _worldModel->selectionModel()->currentIndex();
        worldCurrentChanged(current, QModelIndex());
    }
}

void InfoBrowser::worldCurrentChanged(const QModelIndex& /*current*/, const QModelIndex& /*previous*/)
{
    if(isVisible()) {
        if(_followAction->isChecked()) syncSelection();
        else updateSyncSelection();
    } else {
        _selectionChanged = true;
    }
}

void InfoBrowser::syncSelection(bool checked)
{
    if(checked) {
        const QModelIndex current = _worldModel->selectionModel()->currentIndex();
        const QUrl url(QStringLiteral("objinfo:").append(current.data(WorldModel::ClassNameRole).toString()));
        openUrl(url, true);
    }
}

void InfoBrowser::updateSyncSelection()
{
    if(_htmlBrowser->source().scheme() == QLatin1String("objinfo")) {
        QModelIndex current = _worldModel->selectionModel()->currentIndex();
        if(_htmlBrowser->source().path() == current.data(WorldModel::ClassNameRole).toString()) {
            _syncAction->setEnabled(false);
            return;
        }
    }
    _syncAction->setEnabled(true);
}

void InfoBrowser::openUrl(const QUrl& url, bool clearHistory, bool fromHistory)
{
    if(url.scheme() == QLatin1String("objinfo")) {
        if(clearHistory) {
            _forwardHistory.clear();
            _forwardAction->setEnabled(false);
            _backHistory.clear();
            _backAction->setEnabled(false);
            fromHistory = true;
        }
        QString className = url.path();
        if(className.isEmpty()) {
            setHtml("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                    "</head><body>\n"
                    "<div id='doc_box' class='box'>\n"
                        "<div id='doc_box-header' class='box-header'>\n"
                            "<span id='doc_box-header-title' class='box-header-title'>\n"
                            + i18n( "Documentation" ) +
                            "</span>\n"
                        "</div>\n"
                        "<div id='doc_box-body' class='box-body'>\n"
                            "<div class='info'><p>\n"
                            + i18n("No current object.") +
                            "</p></div>\n"
                        "</div>\n"
                    "</div>\n"
                    "</body></html>", fromHistory, url );
            return;
        }
        QString fileName = KLocalizedString::localizedFilePath(QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral("objinfo/%1.html").arg(className.toLower())));
        if(!fileName.isEmpty()) {
            QFile file(fileName);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                setHtml(QString::fromUtf8(file.readAll()), fromHistory, url);
                return;
            } else {
                qWarning() << "Could not open help file at location:" << fileName;
            }
        }
        setHtml("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                "</head><body>\n"
                "<div id='doc_box' class='box'>\n"
                    "<div id='doc_box-header' class='box-header'>\n"
                        "<span id='doc_box-header-title' class='box-header-title'>\n"
                        + i18n( "Documentation error" ) +
                        "</span>\n"
                    "</div>\n"
                    "<div id='doc_box-body' class='box-body'>\n"
                        "<div class='error'><p>\n"
                        + i18n("Documentation for %1 not available. ", QCoreApplication::translate("ObjectClass", className.toUtf8().constData()))
                        + i18n("You can help <a href=\"https://edu.kde.org/step\">Step</a> by writing it!") +
                        "</p></div>\n"
                    "</div>\n"
                "</div>\n"
                "</body></html>", fromHistory, url );
        show();
    } else if(url.scheme() == QLatin1String("https") || url.scheme() == QLatin1String("http")) {
        // do not clear history when open external URL
        QDesktopServices::openUrl(url);
    } else {
        qWarning() << "Unknown URL scheme detected, skipping:" << url;
    }
}

void InfoBrowser::setHtml(const QString& data, bool fromHistory, const QUrl& url)
{
    if(!fromHistory) {
        _forwardAction->setEnabled(false);
        _forwardHistory.clear();

        QString oldUrl = _htmlBrowser->source().url();
        if(!oldUrl.isEmpty()) {
            _backHistory << oldUrl;
            _backAction->setEnabled(true);
        }
    }

    if(url.scheme() == QLatin1String("http")) {
        _execAction->setEnabled(true);
    }
    else {
        _execAction->setEnabled(false);
    }

    _htmlBrowser->setSource(url);
    _htmlBrowser->setHtml(data);

    updateSyncSelection();
}

void InfoBrowser::back()
{
    Q_ASSERT(!_backHistory.isEmpty());

    QString url(_backHistory.takeLast());
    if(_backHistory.isEmpty())
        _backAction->setEnabled(false);

    QString curUrl = _htmlBrowser->source().url();
    if(!curUrl.isEmpty()) {
        _forwardHistory << curUrl;
        _forwardAction->setEnabled(true);
    }

    openUrl(QUrl(url), false, true);
}

void InfoBrowser::forward()
{
    Q_ASSERT(!_forwardHistory.isEmpty());

    QString url(_forwardHistory.takeLast());
    if(_forwardHistory.isEmpty())
        _forwardAction->setEnabled(false);

    QString curUrl = _htmlBrowser->source().url();
    if(!curUrl.isEmpty()) {
        _backHistory << curUrl;
        _backAction->setEnabled(true);
    }

    openUrl(QUrl(url), false, true);
}

void InfoBrowser::openInBrowser()
{
    if(_htmlBrowser->source().scheme() == QLatin1String("https")) {
        QDesktopServices::openUrl(_htmlBrowser->source());
    }
}
