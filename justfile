extra_args := ""
release := "false"
default_args := "-xc -std=c99 -Iinclude -Llibs -Istblibs"
args := if release == "true" { default_args + " -Wall -Wextra -Werror -O2" } else { default_args + " -g -Wall -Wextra -fno-omit-frame-pointer -fsanitize=address" }

alias default := compile_all
compile_all *args="": (compile "error")

[positional-arguments]
compile lib:
  mkdir -p libs
  tcc {{args}} {{extra_args}} -o libs/{{lib}}.o -c src/{{lib}}.c
  ar rv libs/lib{{lib}}.a libs/{{lib}}.o
  ranlib libs/lib{{lib}}.a

[positional-arguments]
test *test_args="":
  tcc {{args}} {{extra_args}} -o tests tests.c
  exec ./tests {{test_args}}

lint:
  cppcheck --enable=all --disable=unusedFunction --suppress=unusedStructMember --suppress=missingIncludeSystem --std=c99 --error-exitcode=1 --check-level=exhaustive *.h *.c
