#!make
# -*- coding: utf-8 -*-
# Copyright (C) 2023 Benjamin Thomas Schwertfeger
# Github: https://github.com/btschwertfeger

.PHONY := build rebuild install uninstall test changelog clean

### 	Building

##		Compile the BiasAdjustCXX command-line tool
##
build:
	cmake -S . -B build && cmake --build build --target BiasAdjustCXX

rebuild: clean build

##		Build and Compile the testsuite
##
dev:
	cmake -S tests/ -B tests/build && cmake --build tests/build

redev: clean dev

### 	Un-/installation

##		Installation
##
install:
	cd build && make install

##		Uninstallation
##
uninstall:
	cd build && make uninstall

### 	Testing

##		Run the unit tests
##
test:
	cd tests/build && ctest

### 	Misc

## 		Create the changelog
##
changelog:
	docker run -it --rm \
		-v "$(pwd)":/usr/local/src/my-app \
		githubchangeloggenerator/github-changelog-generator \
		-u btschwertfeger \
		-p BiasAdjustCXX \
		-t $(GHTOKEN)  \
		--breaking-labels Breaking \
		--enhancement-labels Feature

pre-commit:
	pre-commit run -a

##		Clean the generated files
##
clean:
	rm -rf build .ipynb_checkpoints tests/build
