build_dir := "output"

test:
  mkdir -p {{build_dir}}/
  gcc -Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer -o {{build_dir}}/test tests.c
  ASAN_OPTIONS=detect_leaks=0 exec {{build_dir}}/test
