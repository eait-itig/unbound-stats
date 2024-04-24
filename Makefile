PROG=unbound-stats
SRCS=unbound-stats.c
MAN=

WARNINGS=Yes
#DEBUG=-g

BINDIR=/opt/local/bin
BINOWN=root
BINGRP=wheel
BINMODE=2555

.include <bsd.prog.mk>
