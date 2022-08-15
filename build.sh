#!/bin/zsh

# @brief Implementation of Bias Adjustment methods
# @author Benjamin Thomas Schwertfeger
# @copyright Benjamin Thomas Schwertfeger
# @link https://b-schwertfeger.de
# @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp

g++ -std=c++17 \
    src/main.cpp \
    -o DoBiasAdjustment \
    include/Utils.cpp \
    include/MyMath.cpp \
    include/CMethods.cpp \
    include/NCFileHandler.cpp \
    -lnetcdf-cxx4

echo "Done!"
exit 0