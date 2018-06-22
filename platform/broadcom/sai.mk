BRCM_SAI = libsaibcm_3.2.1.1_amd64.deb
$(BRCM_SAI)_PATH = files/broadcom-binary

BRCM_SAI_DEV = libsaibcm-dev_3.2.1.1_amd64.deb
$(BRCM_SAI_DEV)_PATH = files/broadcom-binary

SONIC_COPY_DEBS += $(BRCM_SAI) $(BRCM_SAI_DEV)
$(BRCM_SAI_DEV)_DEPENDS += $(BRCM_SAI)
