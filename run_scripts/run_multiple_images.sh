#!/usr/bin/env bash
cd ../
make clean build

input_folder=datasets/aerials_pgm
output_folder=datasets/aerials_out

mkdir -p ${output_folder}

for i in $(ls ${input_folder})
do
./bin/edgeDetector -input ${input_folder}/$i -output ${output_folder}/$i
done