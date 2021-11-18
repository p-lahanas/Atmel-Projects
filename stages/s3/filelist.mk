###################################################
# LIBRARY SOURCES
###################################################

# Set mylib folder path.
# IMPORTANT NOTE: change MYLIBTEMPLATE_PATH to your Driver name e.g. JOYSTICK_MYLIBPATH
# e.g. MYLIBPATH=$(SOURCELIB_ROOT)/../mylib
MYLIB_PATH=$(SOURCELIB_ROOT)/../mylib

# Set folder path with header files to include.
CFLAGS += -I$(MYLIB_PATH)

# List all c files locations that must be included (use space as separate e.g. LIBSRCS += path_to/file1.c path_to/file2.c)
LIBSRCS += $(MYLIB_PATH)/s4587423_reg_lta1000g.c  $(MYLIB_PATH)/s4587423_reg_iss.c $(MYLIB_PATH)/s4587423_reg_pb.c $(MYLIB_PATH)/s4587423_reg_keypad.c $(MYLIB_PATH)/s4587423_lib_hamming.c