DIRNAME = ep2-lucas_lucas
EPOBJS = ep2.o
CFLAGS = -Wall -g -pedantic -Wno-unused-result -fsanitize=address
COMPILER = gcc

default : ep2

ep2 : $(EPOBJS)
	$(COMPILER) $(CFLAGS) $(EPOBJS) -o ep2

%.o: %.c
	$(COMPILER) $(CFLAGS) -c $<

tar:
	rm -Rf ../$(DIRNAME)
	rm -f ../$(DIRNAME).tar.gz
	mkdir ../$(DIRNAME)

	# copia os slides, o LEIAME e o Makefile
	cp ../LEIAME ../$(DIRNAME)/LEIAME
	cp Makefile ../$(DIRNAME)/Makefile
	cp ../slides/slides.pdf ../$(DIRNAME)/slides.pdf

	# copia o codigo fonte
	for a in *.cpp; do \
		cp $$a ../$(DIRNAME)/$$a; \
	done

	# (se houverem .h)
	for a in *.hpp; do \
		cp $$a ../$(DIRNAME)/$$a; \
	done

	# gera ../$(DIRNAME).tar.gz
	tar -czvf ../$(DIRNAME).tar.gz ../$(DIRNAME)
	rm -Rf ../$(DIRNAME)


clean:
	rm *.o ep2
