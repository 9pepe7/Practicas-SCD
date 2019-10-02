.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

//Hacer make pcf (prodconsFIFO), pcl (prodconsLIFO) o fu(fumadores)
compilador:=g++

pcf: prodconsFIFO_exe
	./$<

pcl: prodconsLIFO_exe
	./$<

fu: fumadores_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -pthread -o $@ $< Semaphore.cpp

clean:
	rm -f *_exe
