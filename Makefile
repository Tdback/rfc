BINARY_NAME  := rfc
MAN_NAME     := rfc.1
# Include an if statment here to install to ``~/.local/bin`` if running as a
# "normal" user, and install to ``/usr/local/bin/`` if running as root.
# Also, man pages can only be installed if running as root.
INSTALL_PATH := /usr/local/bin/
MAN_PATH     := /usr/local/man/man1/

all: rfc

rfc:
	clang -O2 -Wall -Wextra -l curl main.c -o $(BINARY_NAME)

install:
	# Add a check to make sure binary has been created, or else throw an error.
	# mv $(BINARY_NAME) $(INSTALL_PATH)
	# mv $(MAN_NAME) $(MAN_PATH)
	echo $(INSTALL_PATH)

.PHONY: clean
clean:
	$(RM) $(binary_name)
