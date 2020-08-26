# tool marcros
CC := g++
CCFLAG := -Iout/ffmpeg/include
LIBPATH := -Lout/ffmpeg/lib -Lout/x264/lib
LIBS := -lavformat -lavdevice -lavcodec -lavutil -lpthread -lx264 #-lswresample -lswscale -lavfilter
DBGFLAG := -g
CCOBJFLAG := $(CCFLAG) -c

# path marcros
BIN_PATH := ./out/check_stream/bin
OBJ_PATH := ./out/check_stream/obj
SRC_PATH := ./src
DBG_PATH := ./out/check_stream/debug

# create out diresctory first
ifeq (,$(shell mkdir -p $(BIN_PATH) $(OBJ_PATH) $(SRC_PATH) $(DBG_PATH)))
endif

# compile marcros
TARGET_NAME := stream_check_tool
ifeq ($(OS),Windows_NT)
	TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)
MAIN_SRC := $(SRC_PATH)/chk_strm_entry.cpp

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST)

# default rule
default: all

# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAG) -o $@ $? $(LIBPATH) $(LIBS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c* $(SRC_PATH)/*.h
	$(CC) $(CCOBJFLAG) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c* $(SRC_PATH)/*.h
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CCFLAG) $(DBGFLAG) $? -o $@ $(LIBPATH) $(LIBS)

# phony rules
.PHONY: all
all: $(TARGET)

.PHONY: prebuild
prebuild:
	@echo ">>>>>>>>>>> create directory"
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(SRC_PATH) $(DBG_PATH)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)
