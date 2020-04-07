include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=mqtt_test
PKG_VERSION:=1.0
PKG_RELEASE:=1

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/home/qiao/Projects/playground/linkit7688/mqtt

include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/mqtt_test
	SECTION:=qiao
	CATEGORY:=Qiao
	TITLE:=Hello, MQTT!
	DEPENDS:=+libmosquitto
endef

# Package description; a more verbose description on what our package does
define Package/mqtt_test/description
	A simple "Hello, MQTT!" -application.
endef

# Package preparation instructions; create the build directory and copy the source code. 
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

# Package build instructions; invoke the GNU make tool to build our package
TARGET_LDFLAGS += -lmosquitto
define Build/Compile
	echo "compiling mqtt_test......"
	echo $(TARGET_LDFLAGS)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"		  
endef
# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/mqtt_test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt_test $(1)/usr/bin
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,mqtt_test))
