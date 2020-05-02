CC:=gcc -g
SUBDIRS:= src obj

OBJS:= main.o camera.o  serial.o pthread.o xfmt101.o stm32f103.o
BIN:=myapp
OBJS_DIR:=obj
BIN_DIR:=bin
export CC OBJS BIN OBJS_DIR BIN_DIR

all : CHECK_DIR $(SUBDIRS)

CHECK_DIR : 
	@mkdir -p $(BIN_DIR)
$(SUBDIRS) : ECHO
	@make -C $@
	@echo "***********successful.****************"
ECHO :
	@echo $(SUBDIRS)
	@echo "******************start***************";
run:
	./$(BIN_DIR)/$(BIN)

.PHONY:clean

clean :
	@$(RM) $(OBJS_DIR)/*.o
	@rm -rf $(BIN_DIR)


