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
#include "infobrowser.moc"

#include "worldmodel.h"
#include "settings.h"

#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QAction>
#include <QFile>
#include <KToolBar>
#include <KHTMLPart>
#include <KStandardDirs>
#include <KLocale>
#include <KToolInvocation>
#include <KIO/Job>

InfoBrowser::InfoBrowser(WorldModel* worldModel, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(i18n("Context info"), parent, flags),
      _worldModel(worldModel), _wikiJob(NULL), _wikiFromHistory(false), _selectionChanged(false)
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

    _backAction = _toolBar->addAction(KIcon("go-previous"), i18n("Back"), this, SLOT(back()));
    _backAction->setEnabled(false);
    _forwardAction = _toolBar->addAction(KIcon("go-next"), i18n("Forward"), this, SLOT(forward()));
    _forwardAction->setEnabled(false);

    _toolBar->addSeparator();
    _syncAction = _toolBar->addAction(KIcon("goto-page"), i18n("Sync selection"), this, SLOT(syncSelection())); // XXX: icon
    _syncAction->setEnabled(false);
    _followAction = _toolBar->addAction(KIcon("note2"), i18n("Follow selection")/*, this, SLOT(syncSelection(bool))*/); // XXX: icon
    _followAction->setCheckable(true);
    _followAction->setChecked(true);

    _toolBar->addSeparator();
    _execAction = _toolBar->addAction(KIcon("system-run"), i18n("Open in browser"), this, SLOT(openInBrowser()));
    _execAction->setEnabled(false);

    _htmlPart = new KHTMLPart(widget);
    layout->addWidget(_htmlPart->widget());

    _htmlPart->setJavaEnabled(false);
    _htmlPart->setPluginsEnabled(false);
    _htmlPart->setJScriptEnabled(true);
    _htmlPart->setMetaRefreshEnabled(true);
    _htmlPart->setDNDEnabled(false);

    connect(_htmlPart->browserExtension(),
                SIGNAL(openUrlRequest(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)),
                this, SLOT(openUrl(const KUrl&)));

    connect(_worldModel->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                                           this, SLOT(worldCurrentChanged(const QModelIndex&, const QModelIndex&)));

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
        QModelIndex current = _worldModel->selectionModel()->currentIndex();
        openUrl(QString("objinfo:").append(current.data(WorldModel::ClassNameRole).toString()), true);
    }
}

void InfoBrowser::updateSyncSelection()
{
    if(_htmlPart->url().protocol() == "objinfo") {
        QModelIndex current = _worldModel->selectionModel()->currentIndex();
        if(_htmlPart->url().path() == current.data(WorldModel::ClassNameRole).toString()) {
            _syncAction->setEnabled(false);
            return;
        }
    }
    _syncAction->setEnabled(true);
}

void InfoBrowser::openUrl(const KUrl& url, bool clearHistory, bool fromHistory)
{
    // Cancel the old job
    if(_wikiJob) _wikiJob->kill();
    _wikiJob = NULL;

    if(clearHistory) {
        _forwardHistory.clear();
        _forwardAction->setEnabled(false);
        _backHistory.clear();
        _backAction->setEnabled(false);
        fromHistory = true;
    }

    if(url.protocol() == "objinfo") {
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
        QString fileName = KStandardDirs::locate("appdata", QString("objinfo/%1.html").arg(className));
        if(!fileName.isEmpty()) {
            QFile file(fileName);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                setHtml(QString::fromUtf8(file.readAll()), fromHistory, url /*KUrl(fileName)*/);
                return;
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
                        + i18n("Documentation for %1 not available.", className)
                        + i18n("You can help <a href=\"http://edu.kde.org/step\">Step</a> by writting it!") +
                        "</p></div>\n"
                    "</div>\n"
                "</div>\n"
                "</body></html>", fromHistory, url );
        return;
    } else if(url.protocol() == "http") {
        if(!Settings::wikiExternal() &&
                        QRegExp("[a-zA-Z-]+\\.wikipedia\\.org").exactMatch(url.host())) {
            setHtml(
                "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                "</head><body>\n"
                "<div id='wiki_box' class='box'>\n"
                    "<div id='wiki_box-header' class='box-header'>\n"
                        "<span id='wiki_box-header-title' class='box-header-title'>\n"
                        + i18n( "Wikipedia" ) +
                        "</span>\n"
                    "</div>\n"
                    "<div id='wiki_box-body' class='box-body'>\n"
                        "<div class='info'><p>\n" + i18n( "Fetching Wikipedia Information ..." ) + "</p></div>\n"
                    "</div>\n"
                "</div>\n"
                "</body></html>\n", fromHistory);

            _wikiUrl = url;
            _wikiFromHistory = fromHistory;
            _wikiJob = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
            connect(_wikiJob, SIGNAL(result(KJob*)), this, SLOT( wikiResult(KJob*)));
        } else {
            KToolInvocation::invokeBrowser(url.url());
        }
    }

    show();
}

void InfoBrowser::setHtml(const QString& data, bool fromHistory, const KUrl& url)
{
    if(!fromHistory) {
        _forwardAction->setEnabled(false);
        _forwardHistory.clear();

        QString oldUrl = _htmlPart->url().url();
        if(!oldUrl.isEmpty()) {
            _backHistory << oldUrl;
            _backAction->setEnabled(true);
        }
    }

    if(url.protocol() == "http") _execAction->setEnabled(true);
    else _execAction->setEnabled(false);

    _htmlPart->begin(url);
    _htmlPart->write( data );
    _htmlPart->end();
    
    updateSyncSelection();
}

void InfoBrowser::back()
{
    Q_ASSERT(!_backHistory.isEmpty());

    QString url(_backHistory.takeLast());
    if(_backHistory.isEmpty())
        _backAction->setEnabled(false);

    QString curUrl = _htmlPart->url().url();
    if(!curUrl.isEmpty()) {
        _forwardHistory << curUrl;
        _forwardAction->setEnabled(true);
    }

    openUrl(url, false, true);
}

void InfoBrowser::forward()
{
    Q_ASSERT(!_forwardHistory.isEmpty());

    QString url(_forwardHistory.takeLast());
    if(_forwardHistory.isEmpty())
        _forwardAction->setEnabled(false);

    QString curUrl = _htmlPart->url().url();
    if(!curUrl.isEmpty()) {
        _backHistory << curUrl;
        _backAction->setEnabled(true);
    }

    openUrl(url, false, true);
}

void InfoBrowser::openInBrowser()
{
    if(_htmlPart->url().protocol() == "http") {
        KToolInvocation::invokeBrowser(_htmlPart->url().url());
    }
}

void InfoBrowser::wikiResult(KJob* job)
{
    // inspired by amarok

    if(job != _wikiJob) return;

    if(job->error() != 0)
    {
        setHtml("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                "</head><body>\n"
                "<div id='wiki_box' class='box'>\n"
                    "<div id='wiki_box-header' class='box-header'>\n"
                        "<span id='wiki_box-header-title' class='box-header-title'>\n"
                            + i18n( "Wikipedia error" ) +
                        "</span>\n"
                    "</div>\n"
                    "<div id='wiki_box-body' class='box-body'>\n<div class='error'><p>\n"
                        + i18n( "Information could not be retrieved because the server was not reachable." ) +
                    "</div></p>\n</div>\n"
                "</div>\n"
                "</body></html>\n", _wikiFromHistory);

        return;
    }

    KIO::StoredTransferJob* const storedJob = static_cast<KIO::StoredTransferJob*>( job );
    QByteArray rawData = storedJob->data();
    QString data;

    // TODO: better regexp
    if(rawData.contains("charset=utf-8")) data = QString::fromUtf8(rawData.data());
    else data = QString(rawData);

    //if(data.find( "var wgArticleId = 0" ) != -1) // - article not found

    // remove the new-lines and tabs
    data.replace( '\n', ' ' );
    data.replace( '\t', ' ' );

    QString wikiLanguages;
    // Get the available language list
    if ( data.indexOf("<div id=\"p-lang\" class=\"portlet\">") != -1 )
    {
        wikiLanguages = data.mid( data.indexOf("<div id=\"p-lang\" class=\"portlet\">") );
        wikiLanguages = wikiLanguages.mid( wikiLanguages.indexOf("<ul>") );
        wikiLanguages = wikiLanguages.mid( 0, wikiLanguages.indexOf( "</div>" ) );
    }

    QString copyright;
    QString copyrightMark = "<li id=\"f-copyright\">";
    if ( data.indexOf( copyrightMark ) != -1 )
    {
        copyright = data.mid( data.indexOf(copyrightMark) + copyrightMark.length() );
        copyright = copyright.mid( 0, copyright.indexOf( "</li>" ) );
        copyright.remove( "<br />" );
        //only one br at the beginning
        copyright.prepend( "<br />" );
    }

    // Ok lets remove the top and bottom parts of the page
    data = data.mid( data.indexOf( "<h1 class=\"firstHeading\">" ) );
    data = data.mid( 0, data.indexOf( "<div class=\"printfooter\">" ) );

    // Adding back license information
    data += copyright;
    data.append( "</div>" );

    // Remove unnessesary sections (do it with style?)
    data.remove( QRegExp("<h3 *id=\"siteSub\">[^<]*</h3>") );

    data.remove( QRegExp( "<span class=\"editsection\"[^>]*>[^<]*<[^>]*>[^<]*<[^>]*>[^<]*</span>" ) );

    data.replace( QRegExp( "<a href=\"[^\"]*\" class=\"new\"[^>]*>([^<]*)</a>" ), "\\1" );

    // Remove anything inside of a class called urlexpansion, as it's pointless for us
    data.remove( QRegExp( "<span class= *'urlexpansion'>[^(]*[(][^)]*[)]</span>ttp inthttp" ) );

    // Remove hidden table rows as well
    QRegExp hidden( "<tr *class= *[\"\']hiddenStructure[\"\']>.*</tr>", Qt::CaseInsensitive );
    hidden.setMinimal( true ); //greedy behaviour wouldn't be any good!
    data.remove( hidden );

    // Remove jump-to-nav
    QRegExp jumpToNav( "<div *id= *[\"\']jump-to-nav[\"\']>.*</div>", Qt::CaseInsensitive );
    jumpToNav.setMinimal( true );
    data.remove( jumpToNav );

    // we want to keep our own style (we need to modify the stylesheet a bit to handle things nicely)
    //data.remove( QRegExp( "style= *\"[^\"]*\"" ) );
    //data.remove( QRegExp( "class= *\"[^\"]*\"" ) );

    // let's remove the form elements, we don't want them.
    data.remove( QRegExp( "<input[^>]*>" ) );
    data.remove( QRegExp( "<select[^>]*>" ) );
    data.remove( "</select>\n" );
    data.remove( QRegExp( "<option[^>]*>" ) );
    data.remove( "</option>\n" );
    data.remove( QRegExp( "<textarea[^>]*>" ) );
    data.remove( "</textarea>" );

    //first we convert all the links with protocol to external, as they should all be External Links.
    //data.replace( QRegExp( "href= *\"http:" ), "href=\"externalurl:" );
    //QString url = _wikiUrl.url();
    //QString baseUrl = url.mid(0, url.indexOf("wiki/"));
    //data.replace( QRegExp( "href= *\"/" ), "href=\"" + baseUrl );
    //data.replace( QRegExp( "href= *\"#" ), "href=\"" + baseUrl + '#' );

    data.prepend("<html><body>\n"
                    "<div id='wiki_box' class='box'>\n"
                        "<div id='wiki_box-header' class='box-header'>\n"
                            "<span id='wiki_box-header-title' class='box-header-title'>\n"
                            + i18n( "Wikipedia Information" ) +
                            "</span>\n"
                        "</div>\n"
                        "<div id='wiki_box-body' class='box-body'>\n");
    data.append(        "</div>\n"
                    "</div>\n");

    if (!wikiLanguages.isEmpty()) {
        data.append(
                "<br />"
                "<div id='wiki_box' class='box'>\n"
                    "<div id='wiki_box-header' class='box-header'>\n"
                        "<span id='wiki_box-header-title' class='box-header-title'>\n"
                        + i18n( "Wikipedia Other Languages" ) +
                        "</span>\n"
                    "</div>\n"
                    "<div id='wiki_box-body' class='box-body'>\n"
                        + wikiLanguages +
                    "</div>\n"
                "</div>\n"
                );
    }
    data.append( "</body></html>\n" );

    setHtml( data, _wikiFromHistory, _wikiUrl );

    _wikiJob = NULL;
}

