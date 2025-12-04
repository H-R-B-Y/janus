NAME			= JANUS
CC				= gcc

CFLAGS			= -g3 -O -ffunction-sections -fdata-sections -Wall -Wextra -Werror -fPIC

# Check if TERMINAL_MODE is defined
ifdef TERMINAL_MODE
CFLAGS			+= -DJANUS_TERMINAL_MODE
LIBFLAGS		:= -lm
INCLUDES		:= -I./include
else
# gcc src/main.c -I./lib/Config -I./lib/GUI -I./lib/Fonts -I./lib/e-Paper -I include -I lib -L lib -lepd_2in13_v4 -lgui -lfonts -lconfig -llgpio -lm 
LIBFLAGS		:= -L lib -lepd_2in13_v4 -lgui -lfonts -lconfig -llgpio -lm
INCLUDES		:= -I./include -I./lib/Config -I./lib/GUI -I./lib/Fonts -I./lib/e-Paper
endif

# Flags for mlx because it needs to be compiled with specific flags for distro compatibility
EPDFLAGS		:=  -g -O -ffunction-sections -fdata-sections -Wall -fPIC

MAKEFLAGS		+= --no-print-directory

LIB_DIR			:= ./lib

SRC_DIR			:= ./src

SRCS			:= \
				./src/main.c \
				./src/janus/janus_run.c \
				./src/janus/scan_existing_interfaces.c \
				./src/event_handlers/handle_net_event.c \
				./src/event_handlers/handle_signal_event.c \
				./src/event_handlers/handle_event_fd_event.c \
				\


BIN				:= \
				\

TEST_SCRIPT		:=

OBJS			:= $(BIN:.bin=.o) ${SRCS:.c=.o}

all: $(NAME)

$(NAME): $(OBJS)
ifdef TERMINAL_MODE
		$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBFLAGS) -o $(NAME)
else
		$(MAKE) --directory ./lib/ clean
		$(MAKE) --directory ./lib/
		$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBFLAGS) -o $(NAME)
endif

terminal: 
		$(MAKE) TERMINAL_MODE=1

%.o: %.c
		@$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES)

%.o: %.bin
	@ld -r -b binary -o $@ $<

clean:
		@rm -rf $(OBJS)
		@find . -name '*.gcda' -delete
		@find . -name '*.gcno' -delete

rm:
		@$(MAKE) --directory ./lib/ clean
		@rm -rf $(NAME)

fclean: clean rm pre post

re: fclean all

coverage:
ifeq ($(TEST_SCRIPT),)
	@printf "There has been an issue running coverage: no script to run program\n"
else
	@rm -rf coverage_reports
	@find . -name '*.gcda' -delete
	@make --directory . re CFLAGS="-fprofile-arcs -ftest-coverage" CC=gcc-10
	@bash -c $(TEST_SCRIPT)
	@sleep 2
	@mkdir -p coverage_reports
	@find . -name '*.gcda' -exec mv -n {} . \;
	@find . -name '*.gcno' -exec mv -n {} . \;
	@find . -name '*.c' -exec gcov-10 -o . {} \;  >> coverage_reports.txt
	@find . -name '*.gcov' -exec mv {} coverage_reports/ \;
	@find . -name '*.gcda' -delete
	@find . -name '*.gcno' -delete
endif

.PHONY: all clean fclean re  submodule-status install-submodules pull-submodules pre post coverage