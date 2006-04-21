CC	=	m68k-amigaos-gcc
DEBUG	=	-DDEBUG=1 -g
WARNS	=	-W -Wall -Winline
CFLAGS	=	-inox -m68060 -O3 -msmall-code -I. $(DEBUG) $(WARNS)
#LIBS	=	-lgnugetopt -ldebug -Wl,-Map,$@.map,--cref
LIBS	=	-Wl,-Map,$@.map,--cref
NOSTDLIB	=	-inox #-nostdlib

OBJDIR	=	.objs
OBJS	=	\
	$(OBJDIR)/read_tree.o	\
	$(OBJDIR)/read_envs.o	\
	$(OBJDIR)/open_oldlocale.o	\
	$(OBJDIR)/open_cd.o	\
	$(OBJDIR)/util.o	\
	$(OBJDIR)/localize.o	\
	$(OBJDIR)/Locky.o


all:	Locky
deb:	Locky.deb

$(OBJDIR)/%.o: locky.h
$(OBJDIR)/localize.o: localize.h

$(OBJDIR)/Locky.o: Locky.c
	$(CC) $(CFLAGS) -c $< -o $@ -DBUILD_SYSTEM=\"`uname -s`\"

$(OBJDIR)/%.o: %.c
	$(CC) -noixemul $(CFLAGS) -c $< -o $@

Locky:	$(OBJS)
	$(CC) $(NOSTDLIB) $(CFLAGS) -s -o $@ $(OBJS) $(LIBS)

Locky.deb:	$(OBJS)
	$(CC) $(NOSTDLIB) $(CFLAGS) -o $@ $(OBJS)


