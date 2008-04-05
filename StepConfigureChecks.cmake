include(CheckIncludeFiles)
include(CheckFunctionExists)

macro_optional_find_package(GSL)
macro_optional_find_package(Qalculate)
find_package(GMM)

macro_log_feature(QALCULATE_FOUND "Qualculate" "A multi-purpose desktop calculator" "http://qalculate.sourceforge.net/" FALSE "0.9.5" "Enable unit conversion support in Step")
macro_log_feature(GSL_FOUND "GSL" "The GNU Scientific Library, a numerical library for C and C++" "http://www.gnu.org/software/gsl/" FALSE "1.8" "Enables GSL-powered features in Step")
