target: ../$(OBJS_DIR)/main.o \
		../$(OBJS_DIR)/camera.o \
		../$(OBJS_DIR)/serial.o \
		../$(OBJS_DIR)/xfmt101.o \
		../$(OBJS_DIR)/stm32f103.o \
		../$(OBJS_DIR)/pthread.o 

../$(OBJS_DIR)/main.o: main.c
	@$(CC) -c $^ -o $@
../$(OBJS_DIR)/camera.o: camera.c
	@$(CC) -c $^ -o $@
../$(OBJS_DIR)/serial.o: serial.c
	@$(CC) -c $^ -o $@
../$(OBJS_DIR)/xfmt101.o: xfmt101.c
	@$(CC) -c $^ -o $@
../$(OBJS_DIR)/stm32f103.o: stm32f103.c
	@$(CC) -c $^ -o $@
../$(OBJS_DIR)/pthread.o: pthread.c
	@$(CC) -c $^ -o $@	
