#!/bin/zsh

# @brief Implementation of Bias Adjustment methods
# @author Benjamin Thomas Schwertfeger
# @copyright Benjamin Thomas Schwertfeger
# @link https://b-schwertfeger.de
# @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp

g++ -std=c++11 -Wall -v\
    src/main.cxx \
    -o BiasAdjustCXX \
    src/Utils.cxx \
    src/MathUtils.cxx \
    src/CMethods.cxx \
    src/NcFileHandler.cxx \
    -I include \
    -lnetcdf-cxx4

echo "Done!"
exit 0