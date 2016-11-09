CC = gcc
CFLAGS  = -g -Wall
MAIN = sim
#SRCDIR = ../DSP_Code
#INCLUDES = -I./ ../DSP_Code

SRCS = main.c component.c controller.c test_components.c history.c wav.c reverb.c echo.c lftf.c nlms.c

OBJS = $(SRCS:.c=.o)

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS)	

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@
	
.PHONY: clean
	
clean: 
	$(RM) *.o *~ *.gch output1.wav output2.wav $(MAIN)
	
depend: $(SRCS)
	makedepend $(INCLUDES) $^