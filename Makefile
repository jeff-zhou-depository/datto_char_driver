obj-m += datto_char_drv.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

PHONY:clean

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
