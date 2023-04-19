#!make
# -*- coding: utf-8 -*-
# Copyright (C) 2023 Benjamin Thomas Schwertfeger
# Github: https://github.com/btschwertfeger

PROJECT := BiasAdjustCXX
TEST_PROJECT := TestBiasAdjustCXX
TEST_DIR := tests
.PHONY := build rebuild dev redev install uninstall test build-val changelog pre-commit clean help

help:
	@grep "^##" Makefile | sed -e "s/##//"

## ======= B U I L D I N G =======
## build		Compile the BiasAdjustCXX command-line tool
##
build:
	cmake -S . -B build && cmake --build build --target $(PROJECT)

## rebuild	Rebuild BiasAdjustCXX
##
rebuild: clean build

## dev		Build and Compile the testsuite
##
dev:
	cmake -S . -B build && cmake --build build --target $(TEST_PROJECT)

## redev 		Rebuild the testsuite
##
redev: clean dev

## build-val	Build the validation tool (not actively maintained!)
##
build-val:
	cmake -S validation/ -B validation/build && cmake --build validation/build

## doc 		Build the documentation
##
doc:
	cd docs && make html

## redoc	Rebuild the documentation
##
redoc: clean doc

## ======= U N -/ I N S T A L L A T I O N =======
## install	Installation of the BiasAdjustCXX tool
##		(after a successfull the build)
##
install:
	cd build && make install

## uninstall	Uninstallation of the BiasAdjustCXX tool
##
uninstall:
	cd build && make uninstall

## ======= T E S T I N G =======
## test		Run the unit tests
##		(after executing the `dev`-target)
##
test:
	cd build/tests && ctest

## ======= M I S C E L A N I O U S =======
## changelog	Create the changelog
##
changelog:
	docker run -it --rm \
		-v $(PWD):/usr/local/src/your-app \
		githubchangeloggenerator/github-changelog-generator \
		-u btschwertfeger \
		-p BiasAdjustCXX \
		-t $(GHTOKEN)  \
		--breaking-labels Breaking \
		--enhancement-labels Feature

## pre-commit 	Run the pre-commit hooks
##
pre-commit:
	pre-commit run -a

## Clean		Delete the generated files
##
clean:
	rm -rf \
		build tests/build validation/build \
		.ipynb_checkpoints examples/.ipynb_checkpoints\
		docs/_build
