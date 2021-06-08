DIRNAME = ep2-lucas_lucas
CLIENTOBJS = ep2-client.o util.o client-functionality.o client-cript.o packages.o
SERVEROBJS = ep2-server.o util.o server-functionality.o server-io.o server-cript.o packages.o
CPPFLAGS = -Wall -g -pedantic -Wno-unused-result
COMPILER = g++

default : ep2-client ep2-server

ep2-client : $(CLIENTOBJS)
	$(COMPILER) $(CPPFLAGS) $(CLIENTOBJS) -o ep2-client

ep2-server : $(SERVEROBJS)
	$(COMPILER) $(CPPFLAGS) $(SERVEROBJS) -o ep2-server

%.o: %.cpp %.hpp
	$(COMPILER) $(CPPFLAGS) -c $<

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
