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

#include <KApplication>
#include <K4AboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>

#include "mainwindow.h"

#define STEP_VERSION "0.1.0"

static const char description[] =
	I18N_NOOP("Interactive physical simulator");

static const char version[] = STEP_VERSION;

int main(int argc, char* argv[])
{
    K4AboutData aboutData("step", 0, ki18n("Step"), version, ki18n(description),
                     K4AboutData::License_GPL, ki18n("(C) 2007 Vladimir Kuznetsov"), KLocalizedString(), "http://edu.kde.org/step");
    aboutData.addAuthor(ki18n("Vladimir Kuznetsov"), ki18n("Original author"), "ks.vladimir@gmail.com");
    aboutData.addAuthor(ki18n("Carsten Niehaus"), ki18n("Code contributions"), "cniehaus@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[url]", ki18n( "Document to open" ));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
//     KGlobal::locale()->insertCatalog("step_qt");

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if(args->count() > 0) {
        //kDebug() << args->url(0) << endl;
        mainWindow->openFile(args->url(0));
    }

    return app.exec();
}

