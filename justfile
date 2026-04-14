build_dir := "output"

test:
  mkdir -p {{build_dir}}/
  gcc -Wall -Wextra -g -fsanitize=address -o {{build_dir}}/test tests.c
  exec {{build_dir}}/test
