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
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

#include "mainwindow.h"

#define STEP_VERSION "0.0.3"

static const char description[] =
	I18N_NOOP("Interactive physical simulator");

static const char version[] = STEP_VERSION;

static KCmdLineOptions options[] =
{
    { "+[file]", I18N_NOOP( "Document to open" ), 0 },
    KCmdLineLastOption
};

int main(int argc, char* argv[])
{
    KAboutData aboutData("step", I18N_NOOP("Step"), version, description,
                     KAboutData::License_GPL, "(C) 2007 Vladimir Kuznetsov", 0, "http://stepcore.sf.net");
    aboutData.addAuthor("Vladimir Kuznetsov", 0, "ks.vladimir@gmail.com", 0);

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if(args->count() > 0) mainWindow->openFile(args->arg(0));

    return app.exec();
}

