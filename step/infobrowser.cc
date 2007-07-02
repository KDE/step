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

#include <QItemSelectionModel>
#include <KHTMLPart>
#include <KStandardDirs>
#include <KLocale>

#include "worldmodel.h"

InfoBrowser::InfoBrowser(WorldModel* worldModel, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(i18n("Context info"), parent, flags)
{
    _worldModel = worldModel;

    _htmlPart = new KHTMLPart(this);
    _htmlPart->setJavaEnabled(false);
    _htmlPart->setPluginsEnabled(false);
    _htmlPart->setJScriptEnabled(false);
    _htmlPart->setMetaRefreshEnabled(false);
    _htmlPart->setDNDEnabled(false);

    _htmlPart->openStream( "text/html", KUrl() );
    _htmlPart->writeStream( QByteArray( "<html><body><p>KHTML Hello World!</p></body></html>" ) );
    _htmlPart->closeStream();

    setWidget(_htmlPart->widget());

    worldCurrentChanged(_worldModel->worldIndex(), QModelIndex());

    QObject::connect(_worldModel->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                                           this, SLOT(worldCurrentChanged(const QModelIndex&, const QModelIndex&)));


    //setWidget(_treeView);
}

void InfoBrowser::showEvent(QShowEvent* event)
{
    QDockWidget::showEvent(event);
    QModelIndex current = _worldModel->selectionModel()->currentIndex();
    worldCurrentChanged(current, QModelIndex());
}

void InfoBrowser::worldCurrentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    if(isHidden()) return;
    QString className = current.data(WorldModel::ClassNameRole).toString();
    QString fileName = KStandardDirs::locate("data", QString("step/objinfo/%1.html").arg(className));
    if(!fileName.isEmpty()) {
        _htmlPart->openUrl(fileName);
    } else {
        _htmlPart->openStream( "text/html", KUrl() );
        _htmlPart->writeStream( i18n("<html><body><p>Documentation for %1 not found.</p></body></html>",
                        current.data(WorldModel::ClassNameRole).toString()).toAscii() );
        _htmlPart->closeStream();
    }
}

