BRCM_SAI_VERSION = 3.4.4.1-8

export BRCM_SAI_VERSION

BRCM_SAI = libsaibcm_$(BRCM_SAI_VERSION)_amd64.deb
$(BRCM_SAI)_PATH = files/broadcom-binary

BRCM_SAI_DEV = libsaibcm-dev_$(BRCM_SAI_VERSION)_amd64.deb
$(BRCM_SAI_DEV)_PATH = files/broadcom-binary

SONIC_COPY_DEBS += $(BRCM_SAI) $(BRCM_SAI_DEV)
$(BRCM_SAI_DEV)_DEPENDS += $(BRCM_SAI)