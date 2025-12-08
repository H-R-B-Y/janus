NAME			= JANUS
CC				= gcc

CFLAGS			?= -g3 -O -ffunction-sections -fdata-sections -Wall -Wextra -Werror -fPIC

INTERFACE_1		?= wlan0
INTERFACE_2		?= eth0

# Check if TERMINAL_MODE is defined
ifdef TERMINAL_MODE
CFLAGS			+= -DJANUS_TERMINAL_MODE
LIBFLAGS		:= -lm -lpthread
INCLUDES		:= -I./include
else
# gcc src/main.c -I./lib/Config -I./lib/GUI -I./lib/Fonts -I./lib/e-Paper -I include -I lib -L lib -lepd_2in13_v4 -lgui -lfonts -lconfig -llgpio -lm 
LIBFLAGS		:= -L lib -lepd_2in13_v4 -lgui -lfonts -lconfig -llgpio -lm -lpthread
INCLUDES		:= -I./include -I./lib/Config -I./lib/GUI -I./lib/Fonts -I./lib/e-Paper
endif
CFLAGS			+= -DINTERFACE_1=\"$(INTERFACE_1)\"
CFLAGS			+= -DINTERFACE_2=\"$(INTERFACE_2)\"

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
				./src/event_handlers/timer_wheel.c \
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
		$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBFLAGS) -o $(NAME)
endif

terminal: 
		$(MAKE) TERMINAL_MODE=1

service:
		@rm ./janus.service || true
		@touch ./janus.service
		@echo "[Unit]" >> ./janus.service
		@echo "Description=JANUS" >> ./janus.service
		@echo "After=network-online.target" >> ./janus.service
		@echo "Wants=network-online.target" >> ./janus.service
		@echo "" >> ./janus.service
		@echo "[Service]" >> ./janus.service
		@echo "Type=oneshot" >> ./janus.service
		@echo "User=root" >> ./janus.service
		@echo "WorkingDirectory=${PWD}" >> ./janus.service
		@echo "ExecStart=${PWD}/${NAME}" >> ./janus.service
		@echo "RemainAfterExit=yes" >> ./janus.service
		@echo "StandardOutput=journal" >> ./janus.service
		@echo "StandardError=journal" >> ./janus.service
		@echo "" >> ./janus.service
		@echo "[Install]" >> ./janus.service
		@echo "WantedBy=multi-user.target" >> ./janus.service

install: service
		@$(MAKE)
		# Install
		@echo "The following needs to be run with sudo:"
		sudo cp ./janus.service /etc/systemd/system/janus.service
		sudo systemctl enable janus.service
		sudo systemctl start janus.service &

uninstall:
		@rm -rf ./janus.service
		sudo systemctl stop janus.service
		sudo systemctl disable janus.service
		sudo rm -rf /etc/systemd/system/janus.service

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
		@rm -rf janus.service

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