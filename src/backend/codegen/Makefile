# Makefile for codegen with Balerion.

subdir = src/backend/codegen
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

OBJS = init_codegen.o slot_projection_codegen.o

override CXXFLAGS := -std=c++11 $(CXXFLAGS) -fno-rtti

include $(top_srcdir)/src/backend/common.mk

# make `all' the default target
all:
	c++ -std=c++11  -lbalerion -fno-rtti -I../../../src/include -I/usr/local/include -I/usr/local/Cellar/llvm37/3.7.1/lib/llvm-3.7/include/ -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -g -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk/usr/include/libxml2 codegen_test.cpp init_codegen.o slot_projection_codegen.o
