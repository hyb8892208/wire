#!/bin/sh
make -f Makefile
echo "make simemusvr done"
make -f Makefile.callEventHdl
echo "make calleventhdl done"
make -f Makefile.probe_emu
echo "make probe_emu done"
