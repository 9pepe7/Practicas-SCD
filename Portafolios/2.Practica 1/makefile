.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

// Hacer make pc o make fu
compilador:=g++

pc: prodcons_exe
	./$<

fu: fumadores_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -pthread -o $@ $< Semaphore.cpp 

clean:
	rm -f *_exe
