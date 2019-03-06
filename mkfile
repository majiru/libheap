</$PLAN9/src/mkhdr

P=heap

LIB=lib$P.$O.a
OFILES=$P.$O
HFILES=$PLAN9/include/$P.h

<$PLAN9/src/mksyslib

install:V:	$LIB
	cp $LIB $PLAN9/lib/lib$P.a
	cp $P.h $PLAN9/include/$P.h

uninstall:V:
	rm -f /$objtype/lib/lib$P.a /sys/include/$P.h

nuke:V:
	mk uninstall

clean:V:
	rm -f *.[$OS] [$OS].* $LIB

$O.%:	%.$O $OFILES $LIB $TESTLIB
	$LD $LDFLAGS -o $target $prereq
