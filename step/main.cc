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

#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QDebug>
#include <KLocalizedString>
#include <QUrl>
#include <QDir>

#include "mainwindow.h"

#define STEP_VERSION "0.1.0"

static const char description[] =
    I18N_NOOP("Interactive physical simulator");

static const char version[] = STEP_VERSION;

int main(int argc, char* argv[])
{
    KLocalizedString::setApplicationDomain("step");

    QApplication::setApplicationName("step");
    QApplication::setApplicationVersion(version);
    QApplication::setOrganizationDomain("kde.org");
    QApplication::setApplicationDisplayName(i18n("Step"));
    QApplication::setWindowIcon(QIcon::fromTheme("step"));

    QApplication app(argc, argv);

    KAboutData aboutData("step",
                         i18n("Step"),
                         version,
                         i18n(description),
                         KAboutLicense::GPL,
                         i18n("(C) 2007 Vladimir Kuznetsov"),
                         i18n("http://edu.kde.org/step")
    );

    aboutData.addAuthor(
        "Vladimir Kuznetsov",
        i18n("Original author"),
        i18n("ks.vladimir@gmail.com")
    );

    aboutData.addAuthor(
        "Carsten Niehaus",
        i18n("Code contributions"),
        i18n("cniehaus@kde.org")
    );

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption option(QStringList() << "+[url]" << i18n( "Document to open" ));
    parser.addOption(option);

    parser.process(app);

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    const QStringList args = parser.positionalArguments();

    if(args.count() > 0) {
        //qDebug() << args[0] << endl;
        // open the step files passed as arguments as relative or absolute paths
        mainWindow->openFile(
            (QUrl(args[0], QUrl::TolerantMode).isRelative() && !QDir::isAbsolutePath(args[0]))
            ? QUrl::fromLocalFile(QDir::current().absoluteFilePath(args[0]))
            : QUrl::fromUserInput(args[0])
        );
    }

    return app.exec();
}
