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

#include "latexformula.h"
#include <QTextStream>
#include <QProcess>
#include <QColor>
#include <QFile>
#include <QRegExp>
#include <QApplication>
#include <KTempDir>
#include <KGlobal>
#include <KStandardDirs>
#include <KLocale>

namespace {

bool executeCommand(const QString& cmd, const QStringList& args,
                    const QString& workDir, const QString& resultFile, QString* error)
{
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setWorkingDirectory(workDir);
    proc.start(cmd, args);

    if(!proc.waitForStarted()) {
        *error = i18n("can not launch %1", cmd); return false;
    }

    proc.closeWriteChannel();

    while(proc.state() == QProcess::Running) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if(proc.exitStatus() != QProcess::NormalExit) {
        *error = i18n("error running %1", cmd); return false;
    }

    if(proc.exitCode() != 0) { // XXX TODO: verbose error message
        *error = i18n("%1 reported an error (exit status %2):\n%3",
                    cmd, proc.exitCode(), QString::fromLocal8Bit(proc.readAll())); return false;
    }
    
    if(!QFile::exists(resultFile)) {
        *error = i18n("%1 did not created output file", cmd); return false;
    }

    return true;
}

}

bool LatexFormula::isLatexInstalled()
{
    if(KStandardDirs::findExe("latex").isEmpty()) return false;
    if(KStandardDirs::findExe("dvips").isEmpty()) return false;
    if(KStandardDirs::findExe("gs").isEmpty()) return false;
    return true;
}

bool LatexFormula::compileFormula(const QString& formula, QByteArray* result, QString* error)
{
    KTempDir tempDir;
    QString baseFileName = tempDir.name() + "formula";

    QFile latexFile(baseFileName + ".tex");
    if(!latexFile.open(QIODevice::WriteOnly)) {
        *error = i18n("can not open temporary file");
        return false;
    }

    QTextStream latexStream(&latexFile);
    latexStream << "\\documentclass[12pt]{article}\n"
                << "\\pagestyle{empty}\n"
                << "\\usepackage[utf8]{inputenc}\n"
                << "\\usepackage[dvips]{graphicx}\n"
                << "\\usepackage[dvips]{color}\n"
                << "\\usepackage{amssymb,amsmath}\n"
                << "\\begin{document}\n"
                << "\\begin{displaymath}"
                << formula 
                << "\\end{displaymath}\n"
                << "\\end{document}\n";

    latexFile.close();

    if(!executeCommand("latex", QStringList() << "--interaction=nonstopmode" << baseFileName + ".tex",
                            tempDir.name(), baseFileName+".tex", error)) return false;

    if(!executeCommand("dvips", QStringList() << "-E" << baseFileName+".dvi" << "-o" << baseFileName+".eps",
                            tempDir.name(), baseFileName+".eps", error)) return false;

    QStringList gsArgs;
    gsArgs << "-dNOPAUSE" << "-dSAFER" << "-dEPSCrop" << "-r100"
           << "-dTextAlphaBits=4" << "-dGraphicsAlphaBits=4" << "-sDEVICE=pngalpha"
           << "-sOutputFile="+baseFileName+".png" << "-q" << "-dBATCH" << baseFileName+".eps";
    if(!executeCommand("gs", gsArgs, tempDir.name(), baseFileName+".png", error)) return false;

    QFile pngFile(baseFileName + ".png");
    if(!pngFile.open(QIODevice::ReadOnly)) {
        *error = i18n("can not open result file");
        return false;
    }

    *result = pngFile.readAll();

    return true;
}

