bin_PROGRAMS = temp
temp_SOURCES = mysql.c
temp_CFLAGS = -I/usr/include/mysql -DBIG_JOINS=1  -fno-strict-aliasing    -g -DNDEBUG
temp_LDFLAGS = -L/usr/lib/arm-linux-gnueabihf -lmysqlclient -lpthread -lz -lm -ldl

AUTOMAKE_OPTIONS = foreign
