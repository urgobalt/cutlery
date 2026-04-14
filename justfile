build_dir := "build"

test:
  mkdir -p {{build_dir}}/
  clang -Wall -Wextra -g -fsanitize=address -o {{build_dir}}/test tests.c
