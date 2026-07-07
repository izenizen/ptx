KVER ?= $(shell uname -r)
KBUILD := /lib/modules/$(KVER)/build

# DKMS環境でもすべてのサブディレクトリのヘッダーを見つけられるように絶対・相対パスを指定
ccflags-y += -I$(src) -I$(src)/drivers/media/dvb-frontends -I$(src)/drivers/media/tuners -I$(src)/drivers/media/pci/ptx -O3 -Os -Wformat=2 -Wall
ccflags-y += -Wno-missing-prototypes -Wno-incompatible-pointer-types -Wno-implicit-function-declaration -Wno-missing-declarations

# ビルド対象モジュールの定義
obj-m += tc90522.o qm1d1c004x.o mxl301rf.o pt3.o nm131.o tda2014x.o pxq3pe.o

# 各モジュールを構成するオブジェクトファイル
tc90522-objs    := drivers/media/dvb-frontends/tc90522.o
qm1d1c004x-objs := drivers/media/tuners/qm1d1c004x.o
mxl301rf-objs   := drivers/media/tuners/mxl301rf.o
pt3-objs        := drivers/media/pci/ptx/pt3_pci.o drivers/media/pci/ptx/ptx_common.o
nm131-objs      := drivers/media/tuners/nm131.o
tda2014x-objs   := drivers/media/tuners/tda2014x.o
pxq3pe-objs     := drivers/media/pci/ptx/pxq3pe_pci.o drivers/media/pci/ptx/ptx_common.o

all:
	make -C $(KBUILD) M=`pwd` modules

clean:
	make -C $(KBUILD) M=`pwd` clean
