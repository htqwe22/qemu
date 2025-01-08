CFLAGS :=
# CFLAGS += -nostdinc
# CFLAGS += -Werror
CFLAGS += -Wall
CFLAGS += -Wno-implicit-fallthrough
CFLAGS += -Wmissing-include-dirs
CFLAGS += -Wunused
CFLAGS += -Wdisabled-optimization
CFLAGS += -Wvla
CFLAGS += -Wshadow
CFLAGS += -Wredundant-decls
CFLAGS += -Wextra
CFLAGS += -Wno-trigraphs
CFLAGS += -Wno-missing-field-initializers
CFLAGS += -Wno-type-limits
CFLAGS += -Wno-sign-compare
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wunused-but-set-variable
CFLAGS += -Wmaybe-uninitialized
CFLAGS += -Wpacked-bitfield-compat
CFLAGS += -Wshift-overflow=2
CFLAGS += -Wlogical-op
CFLAGS += -Wno-error=deprecated-declarations
CFLAGS += -Wno-error=cpp
CFLAGS += -Wno-misleading-indentation
CFLAGS += -ffreestanding
CFLAGS += -Wa,--fatal-warnings
CFLAGS += -march=armv8-a