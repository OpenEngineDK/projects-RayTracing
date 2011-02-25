once:
	mkdir -p build
	max_print_line=12000000	pdflatex -halt-on-error -output-directory=build master.tex

all:
	mkdir -p build
	max_print_line=12000000	pdflatex -halt-on-error -output-directory=build master.tex && \
	cd build && \
	export BIBINPUTS="..:" && \
	bibtex master.aux && \
	cd ..  &&  \
	max_print_line=12000000	pdflatex -halt-on-error -output-directory=build master.tex && \
	max_print_line=12000000	pdflatex -halt-on-error -output-directory=build master.tex

clean:
	rm -rf build

view:
	open build/master.pdf