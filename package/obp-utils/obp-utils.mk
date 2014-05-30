################################################################################
#
# obp-utils
#
################################################################################

OBP_UTILS_VERSION = dc87d231a46586ed8f726cb41c6ed21570c298e5
OBP_UTILS_SITE = git://github.com/atmel-maxtouch/obp-utils.git

OBP_UTILS_AUTORECONF = YES

$(eval $(autotools-package))
