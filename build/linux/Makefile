CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wno-unused-result -MMD -MP

ECHO := $(shell if echo -e "" | grep -q '^-e'; then echo "echo"; else echo "echo -e"; fi)
GREEN = \033[0;32m
NC = \033[0m

OPENSSL_INCLUDE = $(shell pkg-config --cflags openssl)
OPENSSL_LIB = $(shell pkg-config --libs openssl)

INCLUDES = -I../../src -I../../src/common -I../../src/third-party/zlib
INCLUDES += -I../../bridge -I../../bridge/include -I../../bridge/i18n
INCLUDES += $(OPENSSL_INCLUDE)

VERSION ?= 614caa8
CXXFLAGS += -DZSIGN_VERSION=$(VERSION)

LIBS = $(OPENSSL_LIB)

OBJDIR = .build
BINDIR = ../../bin

CORE_SRCS = \
	../../src/archo.cpp \
	../../src/bundle.cpp \
	../../src/certcheck.cpp \
	../../src/macho.cpp \
	../../src/metadata.cpp \
	../../src/openssl.cpp \
	../../src/signing.cpp \
	../../src/common/fs.cpp \
	../../src/common/json.cpp \
	../../src/common/sha.cpp \
	../../src/common/timer.cpp \
	../../src/common/util.cpp

BRIDGE_SRCS = \
	../../bridge/archive_overlay.cpp \
	../../bridge/archive_zip_progress.cpp \
	../../bridge/archive_cancel.cpp \
	../../bridge/fs_ipa_junk.cpp \
	../../bridge/verify_archo.cpp \
	../../bridge/verify_macho_map.cpp \
	../../bridge/verify_signed_bundle.cpp \
	../../bridge/log_overlay.cpp \
	../../bridge/i18n/zlog_i18n.cpp \
	../../bridge/check_cert_loader.cpp \
	../../bridge/macho_dylib_overlay.cpp \
	../../bridge/zsign_run.cpp \
	../../bridge/cli_main.cpp

SRCS = $(CORE_SRCS) $(BRIDGE_SRCS)
OBJS = $(SRCS:../../src/%.cpp=$(OBJDIR)/src/%.o)
OBJS := $(OBJS:../../bridge/%.cpp=$(OBJDIR)/bridge/%.o)
OBJS := $(OBJS:../../bridge/i18n/%.cpp=$(OBJDIR)/bridge/i18n/%.o)

ZLIB_SRCS = $(wildcard ../../src/third-party/zlib/*.c)
ZLIB_OBJS = $(ZLIB_SRCS:../../src/third-party/zlib/%.c=$(OBJDIR)/zlib/%.o)

MINIZIP_SRCS = ../../src/third-party/minizip/ioapi.c ../../src/third-party/minizip/zip.c ../../src/third-party/minizip/unzip.c
MINIZIP_OBJS = $(MINIZIP_SRCS:../../src/third-party/minizip/%.c=$(OBJDIR)/minizip/%.o)

TARGET = $(BINDIR)/zsign

all: $(TARGET)
	@$(ECHO) "$(GREEN)>>> Build OK!$(NC) -> $(abspath $(TARGET))"

$(TARGET): $(OBJS) $(ZLIB_OBJS) $(MINIZIP_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(OBJS) $(ZLIB_OBJS) $(MINIZIP_OBJS) $(LIBS) -o $(TARGET)

$(OBJDIR)/src/%.o: ../../src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/bridge/i18n/%.o: ../../bridge/i18n/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/bridge/%.o: ../../bridge/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/zlib/%.o: ../../src/third-party/zlib/%.c
	@mkdir -p $(dir $@)
	$(CC) -O3 -I../../src/third-party/zlib -c $< -o $@

$(OBJDIR)/minizip/%.o: ../../src/third-party/minizip/%.c
	@mkdir -p $(dir $@)
	$(CC) -O3 $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)
	@$(ECHO) "$(GREEN)>>> Clean OK!$(NC)"

-include $(OBJS:.o=.d)

.PHONY: all clean
