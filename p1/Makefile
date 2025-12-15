# Dodałem -Iinclude (żeby widział nagłówki) oraz -D_DEFAULT_SOURCE (żeby widział DT_DIR)
override CFLAGS=-std=c17 -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unused-const-variable -g -O0 -fsanitize=address,undefined,leak -Iinclude -D_DEFAULT_SOURCE

ifdef CI
# Tutaj również dodane flagi
override CFLAGS=-std=c17 -Wall -Wextra -Wshadow -Werror -Wno-unused-parameter -Wno-unused-const-variable -Iinclude -D_DEFAULT_SOURCE
endif

NAME=sop-backup

.PHONY: clean all

all: ${NAME}

SOURCES=$(shell find src -type f -iname '*.c')

OBJECTS=$(foreach x, $(basename $(SOURCES)), $(x).o)

$(NAME): $(OBJECTS)
	$(CC) $^ ${CFLAGS} -o $@

# Reguła czyszczenia (opcjonalnie można dodać usuwanie .o z src)
clean:
	rm -f $(NAME) $(OBJECTS)