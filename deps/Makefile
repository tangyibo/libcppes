#
#
dirs = testlib

all:
	if [ ! -d "lib" ]; then mkdir "lib" ; fi;
	for x in $(dirs); do (cd $$x; make); done
	cp testlib/liblut.a lib/

clean:
	for x in $(dirs); do (cd $$x; make clean); done

cleanall:
	make clean
	rm -f lib/*
#
#
