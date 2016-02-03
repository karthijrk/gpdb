# Makefile for codegen with Balerion.

subdir = src/backend/codegen
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

OBJS = init_codegen.o

override CXXFLAGS := -std=c++11 $(CXXFLAGS)

include $(top_srcdir)/src/backend/common.mk
