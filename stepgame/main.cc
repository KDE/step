/* This file is part of Step.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>

#include "mainwindow.h"

#define STEP_VERSION "0.1.0"

static const char description[] =
	I18N_NOOP("Educational game for learning physics");

static const char version[] = STEP_VERSION;

int main(int argc, char* argv[])
{
    KAboutData aboutData("stepgame", 0, ki18n("Stepgame"), version, ki18n(description),
                     KAboutData::License_GPL, ki18n("(C) 2007 Aliona Kuznetsova"), KLocalizedString(), "http://edu.kde.org/step");
    aboutData.addAuthor(ki18n("Aliona Kuznetsova"), ki18n("Original author"), "aliona.kuz@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[url]", ki18n( "Document to open" ));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if(args->count() > 0) {
        //kDebug() << args->url(0) << endl;
        mainWindow->openFile(args->url(0));
    }

    return app.exec();
}

