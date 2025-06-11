.PHONY: all clean

all:
	$(MAKE) -C kernel
	$(MAKE) -C user

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C user clean