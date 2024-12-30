
# A user defined function to recursively search for a filename below a directory
#    $1 is the directory root of the recursive search (blank for current directory).
#    $2 is the file name to search for.
define rwildcard
$(strip $(foreach d,$(wildcard ${1}*),$(call rwildcard,${d}/,${2}) $(filter $(subst *,%,%${2}),${d})))
endef


define CUR_MK_FILE
$(lastword $(MAKEFILE_LIST)
endef

#$(abspath $(lastword $(MAKEFILE_LIST)))
define CUR_MK_DIR
$(dir $(lastword $(MAKEFILE_LIST)))
endef

