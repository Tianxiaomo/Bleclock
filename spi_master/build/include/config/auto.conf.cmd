deps_config := \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/app_trace/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/aws_iot/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/bt/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/esp32/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/esp_adc_cal/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/ethernet/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/fatfs/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/freertos/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/heap/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/libsodium/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/log/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/lwip/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/mbedtls/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/openssl/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/pthread/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/spi_flash/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/spiffs/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/tcpip_adapter/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/wear_levelling/Kconfig \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/bootloader/Kconfig.projbuild \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/esptool_py/Kconfig.projbuild \
	/home/aithinker/project/esp32/esp32-idf/spi_master/main/Kconfig.projbuild \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/components/partition_table/Kconfig.projbuild \
	/cygdrive/e/AiThinkerIDE_V0.5/cygwin/home/aithinker/project/esp32/esp32-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
