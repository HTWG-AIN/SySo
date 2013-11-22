################################################################################
#
# uptime-info
#
################################################################################

UPTIME_INFO_VERSION = 1.0
UPTIME_INFO_SOURCE = uptime-info-$(UPTIME_INFO_VERSION).tar.gz
#UPTIME_INFO_SITE = https://github.com/StefanoD/SySo/tree/master/tarballs
UPTIME_INFO_SITE = ~/git-arm/tarballs
UPTIME_INFO_SITE_METHOD = file
UPTIME_INFO_LICENSE = GPLv3+
UPTIME_INFO_LICENSE_FILES = COPYING
#UPTIME_INFO_INSTALL_STAGING = YES
#UPTIME_INFO_CONFIG_SCRIPTS = libfoo-config
#UPTIME_INFO_DEPENDENCIES = host-libaaa libbbb

define UPTIME_INFO_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

#define UPTIME_INFO_INSTALL_STAGING_CMDS
#    $(INSTALL) -D -m 0755 $(@D)/libfoo.a $(STAGING_DIR)/usr/lib/libfoo.a
#    $(INSTALL) -D -m 0644 $(@D)/foo.h $(STAGING_DIR)/usr/include/foo.h
#    $(INSTALL) -D -m 0755 $(@D)/libfoo.so* $(STAGING_DIR)/usr/lib
#endef

define UPTIME_INFO_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/uptime-info-static $(STAGING_DIR)/bin/uptime-static
	$(INSTALL) -D -m 0755 $(@D)/uptime-info $(STAGING_DIR)/bin/uptime
endef

#define UPTIME_INFO_DEVICES
#	/dev/foo  c  666  0  0  42  0  -  -  -
#endef

#define UPTIME_INFO_PERMISSIONS
#	/bin/foo  f  4755  0  0  -  -  -  -  -
#endef

#define UPTIME_INFO_USERS
#	foo -1 libfoo -1 * - - - LibFoo daemon
#endef

$(eval $(generic-package))
