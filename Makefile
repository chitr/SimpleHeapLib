#Author chitr
CFLAGS := -fPIC -O0 -g -ggdb -rdynamic -Wall -Wstrict-prototypes 
LDFLAGS:=-ldl -llog -lpthread 
CC := gcc
MAJOR := 0
MINOR := 0
NAME :=  simpleheap
OBJ  := heap.o
#VERSION := $(MAJOR)
VERSION := $(MAJOR).$(MINOR)
 
lib: lib$(NAME).so.$(VERSION)
 
test: $(NAME)_test
	LD_LIBRARY_PATH=. ./$(NAME)_test
 
$(NAME)_test: lib$(NAME).so
	$(CC) $(NAME)_test.c -o $@ -L. -l$(NAME)
 
lib$(NAME).so: lib$(NAME).so.$(VERSION)
	ldconfig -v -n .
	ln -s lib$(NAME).so.$(MAJOR) lib$(NAME).so
 
lib$(NAME).so.$(VERSION): $(OBJ)
	$(CC) -shared -Wl,-soname,lib$(NAME).so $^ -o $@
 
clean:
	$(RM) $(NAME)_test *.o *.so*
install:
	cp lib*.so* /usr/lib64
	cp lib*.so* /usr/lib64
	ln -sf /usr/lib64/lib$(NAME).so.$(VERSION) /usr/lib64/lib$(NAME).so
#	cp lib$(NAME).so.$(VERSION) /usr/lib64/lib$(NAME).so
	cp *.h /usr/include
