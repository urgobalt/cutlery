test:
  gcc -Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer -o tests tests.c
  exec ./tests
