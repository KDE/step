include(CheckIncludeFiles)
include(CheckFunctionExists)

macro_optional_find_package(GSL)
macro_optional_find_package(Qalculate)
find_package(GMM)

macro_log_feature(QALCULATE_FOUND "Qualculate" "Qualculate is needed by Step" "http://qalculate.sourceforge.net/" FALSE "0.9.5" "")
macro_log_feature(GSL_FOUND "GSL" "Qualculate is needed by Step" "http://www.gnu.org/software/gsl/" FALSE "1.8" "")
