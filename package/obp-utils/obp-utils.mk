################################################################################
#
# obp-utils
#
################################################################################

OBP_UTILS_VERSION = 4cdc89ab4d7b73e1c7b670e1ab832c8ad639f5d8
OBP_UTILS_SITE = git://github.com/noglitch/obp-utils.git

OBP_UTILS_AUTORECONF = YES
OBP_UTILS_CONF_OPT += --disable-manpages


$(eval $(autotools-package))
