# This file is for Clang specific options.

# The macro STEPCORE_OBJECT expands to include the definition of a virtual function. This
# function definition does not specify 'override', so where STEPCORE_OBJECT is used in
# derived classes, the compiler complains that the function is not defined with 'override'
# and throws a warning. This will suppress that warning, pending a better way to do it
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
