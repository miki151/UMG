

CFLAGS = -Wall -std=c++14 -Wno-sign-compare -Wno-unused-variable -Wno-shift-count-overflow -Wno-tautological-constant-out-of-range-compare -Wno-mismatched-tags -ftemplate-depth=512 -Wmissing-field-initializers -Wno-deprecated-declarations

ifndef GCC
GCC = clang++
endif
CC = $(GCC)
LD = $(CC)

CFLAGS += -Werror -g

OBJDIR = obj

NAME = umgl

ROOT = ./
TOROOT = ./../
IPATH = -I.

CFLAGS += $(IPATH)

LDFLAGS += -L/usr/local/lib

SRCS = $(shell ls -t src/*.cpp)

INSTALL_DIR = $(shell pwd)
CFLAGS += "-DINSTALL_DIR=\"$(INSTALL_DIR)\""

LIBS = -L/usr/lib/x86_64-linux-gnu -lstdc++fs ${LDFLAGS}


OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
DEPS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.d))

##############################################################################

all:
	@$(MAKE) --no-print-directory info
	@$(MAKE) --no-print-directory compile

compile: $(NAME)

$(OBJDIR)/%.o: %.cpp 
	$(CC) -MMD $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(LD) $(CFLAGS) -o $@ $^ $(LIBS)

info:
	@$(CC) -v 2>&1 | head -n 2

clean:
	$(RM) $(OBJDIR)/src/*.o
	$(RM) $(OBJDIR)/src/*.d
	$(RM) $(OBJDIR)/test
	$(RM) $(OBJDIR)-opt/*.o
	$(RM) $(OBJDIR)-opt/*.d
	$(RM) $(NAME)
	$(RM) $(OBJDIR)/stdafx.h.*

-include $(DEPS)
