#!make
# -*- coding: utf-8 -*-
# Copyright (C) 2023 Benjamin Thomas Schwertfeger
# Github: https://github.com/btschwertfeger

PROJECT := BiasAdjustCXX
TEST_PROJECT := TestBiasAdjustCXX
TEST_DIR := tests
.PHONY := build rebuild dev redev install uninstall test changelog clean

### 	Building

##		Compile the BiasAdjustCXX command-line tool
##
build:
	cmake -S . -B build && cmake --build build --target $(PROJECT)

rebuild: clean build

##		Build and Compile the testsuite
##
dev:
	cmake -S . -B build && cmake --build build --target $(TEST_PROJECT)

redev: clean dev

##		Build the validation tool (not actively maintained!)
##
build-val:
	cmake -S validation/ -B validation/build && cmake --build validation/build

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
	cd build/tests && ctest

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
	rm -rf build tests/build validation/build \
		.ipynb_checkpoints
