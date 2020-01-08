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
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QUrl>

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>

#include "mainwindow.h"

#include "step_version.h"

static const char description[] =
    I18N_NOOP("Interactive physical simulator");

int main(int argc, char* argv[])
{
    KLocalizedString::setApplicationDomain("step");

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("step"));
    QApplication::setApplicationVersion(STEP_VERSION_STRING);
    QApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QApplication::setApplicationDisplayName(i18n("Step"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("step")));

    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    KCrash::initialize();

    KAboutData aboutData(QStringLiteral("step"),
                         i18n("Step"),
                         STEP_VERSION_STRING,
                         i18n(description),
                         KAboutLicense::GPL,
                         i18n("(C) 2007 Vladimir Kuznetsov"),
                         i18n("https://edu.kde.org/step")
    );

    aboutData.addAuthor(
        QStringLiteral("Vladimir Kuznetsov"),
        i18n("Original author"),
        i18n("ks.vladimir@gmail.com")
    );

    aboutData.addAuthor(
        QStringLiteral("Carsten Niehaus"),
        i18n("Code contributions"),
        i18n("cniehaus@kde.org")
    );

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;

    QCommandLineOption option(QStringList() << QStringLiteral("+[url]") << i18n( "Document to open" ));
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
