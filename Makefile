# カーネルビルドシステムのディレクトリ
KDIR := /lib/modules/$(shell uname -r)/build

# モジュール名
MODULE_NAME := open_hook

# ソースファイル
obj-m += $(MODULE_NAME).o

# デフォルトのターゲット
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# クリーンアップターゲット
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
