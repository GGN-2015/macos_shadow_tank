all: fin_png.out png_reader.out gen_idats_new.out

clean:
	rm *.out

fin_png.out: fin_png.cpp
	g++ fin_png.cpp -lz -o fin_png.out

png_reader.out: png_reader.cpp
	g++ png_reader.cpp -lz -o png_reader.out

gen_idats_new.out: gen_idats_new.cpp
	g++ gen_idats_new.cpp -lz -o gen_idats_new.out

.PHONY: clean all