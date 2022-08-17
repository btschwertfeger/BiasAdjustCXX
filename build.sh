#!/bin/zsh

# @brief Implementation of Bias Adjustment methods
# @author Benjamin Thomas Schwertfeger
# @copyright Benjamin Thomas Schwertfeger
# @link https://b-schwertfeger.de
# @github https://github.com/btschwertfeger/Bias-Adjustment-Cpp

g++ -std=c++11 -Wall -v\
    src/main.cxx \
    -o Main.app \
    src/Utils.cxx \
    src/MyMath.cxx \
    src/CMethods.cxx \
    src/NcFileHandler.cxx \
    -I include \
    -lnetcdf-cxx4

echo "Done!"
exit 0
