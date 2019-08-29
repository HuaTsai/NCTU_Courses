TARGETS := hw1 hw2 hw3 hw4

.PHONY: all
all:
	@for t in $(TARGETS); do $(MAKE) -C $$t; done

.PHONY: clean
clean:
	@for t in $(TARGETS); do $(MAKE) -C $$t clean; done

