#!/usr/bin/env bash
make clean build

./bin/edgeDetector -input datasets/Lena.pgm -output datasets/Lena_edgeDetector.pgm