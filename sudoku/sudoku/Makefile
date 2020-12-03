EXE = sudoku

.PHONY : all clean help

all :
	@cd src && $(MAKE)
	@cp -f src/$(EXE) .

clean :
	@cd src && $(MAKE) clean
	@rm -f *~ $(EXE)

help :
	@echo -e "Usage :"
	@echo -e "  make all\t\tBuild"
	@echo -e "  make build\t\tBuild the software"
	@echo -e "  make help\t\t...Done"
