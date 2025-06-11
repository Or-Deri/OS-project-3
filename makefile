STAGES := PART_1 PART_2 PART_3 PART_4 PART_5 PART_6

.PHONY: all clean $(STAGES)


all: $(STAGES)


$(STAGES):
	@echo "=== Building $@ ==="
	$(MAKE) -C $@

clean:
	@for dir in $(STAGES); do \
		echo "=== Cleaning $$dir ==="; \
		$(MAKE) -C $$dir clean; \
	done