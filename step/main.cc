/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("step");

    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);


    KAboutData aboutData(QStringLiteral("step"),
                         i18n("Step"),
                         STEP_VERSION_STRING,
                         i18n("Interactive physical simulator"),
                         KAboutLicense::GPL,
                         i18n("(C) 2007 Vladimir Kuznetsov"),
                         QString(),
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
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("step")));

    KCrash::initialize();

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
