test:
  gcc -Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer -o tests tests.c
  ASAN_OPTIONS=detect_leaks=0 exec ./tests
