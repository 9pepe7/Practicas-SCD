.SUFFIXES:
.PHONY: x1, x2, x3, clean

comp:= mpicxx -std=c++11 -Wall

x1: holamundo_exe
	mpirun -np 5 --oversubscribe ./$<

x2: sendrecv1_exe
	mpirun -np 2 ./$<

x3: sendrecv2_exe
	mpirun -np 5 --oversubscribe ./$<

x4: ejemplo_probe_exe
	mpirun -np 10 --oversubscribe ./$<

x5: ejemplo_iprobe_exe
	mpirun -np 10 --oversubscribe ./$<

x6: intercambio_sincrono_exe
	mpirun -np 8 --oversubscribe ./$<

x7: intercambio_nobloq_exe
	mpirun -np 8 --oversubscribe ./$<



%_exe: %.cpp
	$(comp) $< -o $@

clean:
	rm -f *_exe
