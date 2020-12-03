EXE = sudoku
CFLAGS = -Wall -Wextra -std=c99 -g
CPPFLAGS = -I ../include/ -c
LDFLAGS = -lm

.PHONY : all clean help

all : $(EXE)

$(EXE) : sudoku.o preemptive_set.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

sudoku.o : sudoku.c ../include/preemptive_set.h sudoku.h 
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $<

preemptive_set.o : preemptive_set.c ../include/preemptive_set.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean :
	@rm -f *~ *.o $(EXE)

help :
	@echo -e "Usage :"
	@echo -e "  make all\t\tBuild the software"
	@echo -e "  make clean\t\tRemove all files created by make"
	@echo -e "  make help\t\tYou're already in help !"
