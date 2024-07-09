#!/bin/bash

python3 gen_bin_for_raw.py
g++ gen_idats_new.cpp -lz -o get_idats_new.out
./get_idats_new.out
g++ fin_png.cpp -lz -o fin_png.out
./fin_png.out
g++ png_reader.cpp -lz -o png_reader.out
echo "fin.png" | ./png_reader.out
python3 gen_ios_png.py