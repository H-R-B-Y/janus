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
				./src/event_handlers/handle_stdin.c \
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
		@rm -rf ./janus.service || true
		@touch ./janus.service
		@echo "[Unit]" >> ./janus.service
		@echo "Description=JANUS" >> ./janus.service
		@echo "After=network-online.target" >> ./janus.service
		@echo "Wants=network-online.target" >> ./janus.service
		@echo "" >> ./janus.service
		@echo "[Service]" >> ./janus.service
		@echo "Type=simple" >> ./janus.service
		@echo "User=root" >> ./janus.service
		@echo "WorkingDirectory=${PWD}" >> ./janus.service
		@echo "# ExecStart opens FIFO RDWR to avoid startup blocking; stdout handled by StandardOutput" >> ./janus.service
		@echo "ExecStart=/bin/sh -c 'exec 3<>/tmp/janus.socket; if command -v stdbuf >/dev/null 2>&1; then exec stdbuf -oL -eL ${PWD}/${NAME} <&3; else exec ${PWD}/${NAME} <&3; fi'" >> ./janus.service
		@echo "# Ensure fresh FIFO each start" >> ./janus.service
		@echo "ExecStartPre=/usr/bin/sh -c '[ -p /tmp/janus.socket ] && rm -f /tmp/janus.socket || true'" >> ./janus.service
		@echo "ExecStartPre=/usr/bin/mkfifo -m 0666 /tmp/janus.socket" >> ./janus.service
		@echo "# Ensure log file exists and is readable" >> ./janus.service
		@echo "ExecStartPre=/bin/sh -c 'touch /var/log/janus.log'" >> ./janus.service
		@echo "ExecStartPre=/bin/sh -c 'chmod 0644 /var/log/janus.log'" >> ./janus.service
		@echo "ExecStopPost=/usr/bin/rm -f /tmp/janus.socket" >> ./janus.service
		@echo "ExecStopPost=/bin/sh -c 'rm -f /tmp/janus.socket'" >> ./janus.service
		@echo "StandardInput=null" >> ./janus.service
		@echo "StandardOutput=append:/var/log/janus.log" >> ./janus.service
		@echo "StandardError=journal" >> ./janus.service
		@echo "Restart=on-failure" >> ./janus.service
		@echo "RestartSec=1s" >> ./janus.service
		@echo "TimeoutStartSec=infinity" >> ./janus.service
		@echo "" >> ./janus.service
		@echo "[Install]" >> ./janus.service
		@echo "WantedBy=multi-user.target" >> ./janus.service

install: service
		@$(MAKE)
		# Install
		@echo "The following needs to be run with sudo:"
		sudo cp ./janus.service /etc/systemd/system/janus.service
		sudo systemctl daemon-reload
		sudo systemctl enable janus.service
		sudo systemctl start janus.service

uninstall:
		@rm -rf ./janus.service
		sudo systemctl stop janus.service || true
		sudo systemctl stop janus.socket || true
		sudo systemctl disable janus.service || true
		sudo rm -rf /etc/systemd/system/janus.service || true
		sudo systemctl daemon-reload || true

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


test_fifo: JANUS
	@set -e; \
	FIFO="janus.fifo"; \
	if [ -p "$$FIFO" ]; then rm -f "$$FIFO"; fi; \
	mkfifo "$$FIFO"; \
	trap 'rm -f "$$FIFO"' EXIT INT TERM; \
	echo "Starting JANUS with stdin redirected from $$FIFO"; \
	./JANUS < "$$FIFO"

.PHONY: all clean fclean re  submodule-status install-submodules pull-submodules pre post coverage test_fifo