extra_args := ""

[positional-arguments]
test *args="":
  gcc -Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer -o tests tests.c
  exec ./tests {{args}}

lint:
  cppcheck --enable=all --disable=unusedFunction --std=c99 --error-exitcode=1 --check-level=exhaustive *.h
