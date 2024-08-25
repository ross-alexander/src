SDK		= /locker/git/pico-sdk

LD		= /opt/pico-riscv/bin/riscv32-unknown-elf-ld
CC		= /opt/pico-riscv/bin/riscv32-unknown-elf-gcc
CXX		= /opt/pico-riscv/bin/riscv32-unknown-elf-g++

LDFLAGS		= -Wl,--script=$(SDK)/src/rp2_common/pico_crt0/rp2350/memmap_default.ld \
-Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=malloc -Wl,--wrap=free \
-Wl,-z,max-page-size=4096 -Wl,--no-warn-rwx-segments -Wl,--gc-sections \
-Wl,--wrap=sprintf -Wl,--wrap=snprintf -Wl,--wrap=vsnprintf -Wl,--wrap=printf -Wl,--wrap=vprintf -Wl,--wrap=puts -Wl,--wrap=putchar -Wl,--wrap=getchar 

CFLAGS		=  -march=rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb -mabi=ilp32 -g -O3 -DNDEBUG -std=gnu11 -ffunction-sections -fdata-sections
CXXFLAGS	= -march=rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb -mabi=ilp32 -g -O3 -DNDEBUG -std=gnu++17 -fno-exceptions -fno-unwind-tables -fno-rtti -fno-use-cxa-atexit -ffunction-sections -fdata-sections

CPPFLAGS	= -DLIB_BOOT_STAGE2_HEADERS=1 -DLIB_PICO_ATOMIC=1 -DLIB_PICO_BIT_OPS=1 -DLIB_PICO_BIT_OPS_PICO=1 -DLIB_PICO_CLIB_INTERFACE=1 -DLIB_PICO_CRT0=1 -DLIB_PICO_CXX_OPTIONS=1 -DLIB_PICO_DIVIDER=1 -DLIB_PICO_DIVIDER_COMPILER=1 -DLIB_PICO_DOUBLE=1 -DLIB_PICO_DOUBLE_COMPILER=1 -DLIB_PICO_FLOAT=1 -DLIB_PICO_FLOAT_COMPILER=1 -DLIB_PICO_INT64_OPS=1 -DLIB_PICO_INT64_OPS_COMPILER=1 -DLIB_PICO_MALLOC=1 -DLIB_PICO_MEM_OPS=1 -DLIB_PICO_MEM_OPS_COMPILER=1 -DLIB_PICO_NEWLIB_INTERFACE=1 -DLIB_PICO_PLATFORM=1 -DLIB_PICO_PLATFORM_COMPILER=1 -DLIB_PICO_PLATFORM_PANIC=1 -DLIB_PICO_PLATFORM_SECTIONS=1 -DLIB_PICO_PRINTF=1 -DLIB_PICO_PRINTF_PICO=1 -DLIB_PICO_RUNTIME=1 -DLIB_PICO_RUNTIME_INIT=1 -DLIB_PICO_STANDARD_BINARY_INFO=1 -DLIB_PICO_STANDARD_LINK=1 -DLIB_PICO_STDIO=1 -DLIB_PICO_STDIO_UART=1 -DLIB_PICO_STDLIB=1 -DLIB_PICO_SYNC=1 -DLIB_PICO_SYNC_CRITICAL_SECTION=1 -DLIB_PICO_SYNC_MUTEX=1 -DLIB_PICO_SYNC_SEM=1 -DLIB_PICO_TIME=1 -DLIB_PICO_TIME_ADAPTER=1 -DLIB_PICO_UTIL=1 -DPICO_32BIT=1 -DPICO_BOARD=\"pico2\" -DPICO_BUILD=1 -DPICO_CMAKE_BUILD_TYPE=\"Release\" -DPICO_COPY_TO_RAM=0 -DPICO_CXX_ENABLE_EXCEPTIONS=0 -DPICO_NO_FLASH=0 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_PROGRAM_NAME=\"blinky\" -DPICO_PROGRAM_VERSION_STRING=\"0.1\" -DPICO_RISCV=1 -DPICO_RP2350=1 -DPICO_TARGET_NAME=\"blinky\" -DPICO_USE_BLOCKED_RAM=0 \
-I/home/ralexand/src/pico/blinky/no-usb -I/home/ralexand/src/pico/blinky/no-usb/.. -I$(SDK)/src/rp2_common/pico_atomic/include -isystem $(SDK)/src/common/pico_stdlib_headers/include -isystem $(SDK)/src/rp2_common/hardware_gpio/include -isystem $(SDK)/src/common/pico_base_headers/include -isystem /home/ralexand/src/pico/blinky-no-usb/build-rv32/generated/pico_base -isystem $(SDK)/src/boards/include -isystem $(SDK)/src/rp2350/pico_platform/include -isystem $(SDK)/src/rp2350/hardware_regs/include -isystem $(SDK)/src/rp2_common/hardware_base/include -isystem $(SDK)/src/rp2_common/pico_platform_compiler/include -isystem $(SDK)/src/rp2_common/pico_platform_panic/include -isystem $(SDK)/src/rp2_common/pico_platform_sections/include -isystem $(SDK)/src/rp2_common/hardware_dcp/include -isystem $(SDK)/src/rp2350/hardware_structs/include -isystem $(SDK)/src/rp2_common/hardware_rcp/include -isystem $(SDK)/src/common/hardware_claim/include -isystem $(SDK)/src/rp2_common/hardware_sync/include -isystem $(SDK)/src/rp2_common/hardware_sync_spin_lock/include -isystem $(SDK)/src/rp2_common/hardware_irq/include -isystem $(SDK)/src/rp2_common/hardware_hazard3/include -isystem $(SDK)/src/rp2_common/hardware_uart/include -isystem $(SDK)/src/rp2_common/hardware_resets/include -isystem $(SDK)/src/rp2_common/hardware_clocks/include -isystem $(SDK)/src/rp2_common/hardware_pll/include -isystem $(SDK)/src/rp2_common/hardware_vreg/include -isystem $(SDK)/src/rp2_common/hardware_watchdog/include -isystem $(SDK)/src/rp2_common/hardware_ticks/include -isystem $(SDK)/src/rp2_common/hardware_xosc/include -isystem $(SDK)/src/rp2_common/hardware_divider/include -isystem $(SDK)/src/common/pico_time/include -isystem $(SDK)/src/rp2_common/hardware_timer/include -isystem $(SDK)/src/common/pico_sync/include -isystem $(SDK)/src/common/pico_util/include -isystem $(SDK)/src/rp2_common/pico_time_adapter/include -isystem $(SDK)/src/rp2_common/pico_runtime/include -isystem $(SDK)/src/rp2_common/pico_runtime_init/include -isystem $(SDK)/src/common/pico_bit_ops_headers/include -isystem $(SDK)/src/common/pico_divider_headers/include -isystem $(SDK)/src/rp2_common/pico_double/include -isystem $(SDK)/src/rp2_common/pico_float/include -isystem $(SDK)/src/rp2_common/pico_malloc/include -isystem $(SDK)/src/common/pico_binary_info/include -isystem $(SDK)/src/rp2_common/pico_printf/include -isystem $(SDK)/src/rp2_common/pico_stdio/include -isystem $(SDK)/src/rp2_common/pico_stdio_uart/include -isystem $(SDK)/src/rp2_common/hardware_riscv/include -isystem $(SDK)/src/rp2_common/pico_bootrom/include -isystem $(SDK)/src/common/boot_picoboot_headers/include -isystem $(SDK)/src/rp2_common/hardware_boot_lock/include -isystem $(SDK)/src/rp2350/boot_stage2/include -isystem $(SDK)/src/common/boot_picobin_headers/include


SDK_OBJS		= stdlib.o gpio.o platform.o panic.o claim.o sync.o sync_spin_lock.o irq.o irq_handler_chain.o \
sem.o lock_core.o mutex.o critical_section.o time.o timeout_helper.o timer.o datetime.o pheap.o \
queue.o uart.o clocks.o pll.o vreg.o watchdog.o ticks.o xosc.o divider.o runtime.o runtime_init.o runtime_init_clocks.o runtime_init_stack_guard.o \
bit_ops_aeabi.o bootrom.o bootrom_lock.o boot_lock.o divider_compiler.o malloc.o atomic.o new_delete.o \
standard_binary_info.o printf.o crt0_riscv.o newlib_interface.o stdio.o stdio_uart.o

all	: blinky.uf2 hello_uart.uf2

hello_uart.uf2: hello_uart.elf
	picotool uf2 convert --quiet   $< $@ --family rp2350-riscv --abs-block

hello_uart.elf: $(SDK_OBJS) hello_uart.o bs_default_padded_checksummed.o
	$(CC) $(LDFLAGS) $^ -o $@

bs_default_padded_checksummed.o: bs2_default_padded_checksummed.S
	$(CC) -c $< -o $@

blinky.uf2: blinky.elf
	picotool uf2 convert --quiet $< $@ --family rp2350-riscv --abs-block

blinky.elf: $(SDK_OBJS) blinky.o bs2_default_padded_checksummed.S
	$(CC) -march=rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb -mabi=ilp32 -g -O3 -DNDEBUG -Wl,--build-id=none -Wl,-Map=blinky.elf.map \
--specs=nosys.specs -Wl,--wrap=__ctzdi2 -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=free -Wl,-L/home/ralexand/src/pico/blinky-no-usb -Wl,--script=$(SDK)/src/rp2_common/pico_crt0/rp2350/memmap_default.ld -Wl,-z,max-page-size=4096 -Wl,--gc-sections -Wl,--no-warn-rwx-segments -Wl,--wrap=sprintf -Wl,--wrap=snprintf -Wl,--wrap=vsnprintf -Wl,--wrap=printf -Wl,--wrap=vprintf -Wl,--wrap=puts -Wl,--wrap=putchar -Wl,--wrap=getchar $^ -o $@


# /opt/pico-riscv/bin/riscv32-unknown-elf-objcopy -Obinary /home/ralexand/src/pico/blinky-no-usb/build-rv32/blinky-no-usb.elf blinky-no-usb.bin

blinky.o: ../blinky.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

hello_uart.o: hello_uart.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<


# --------------------
# SDK code
# --------------------

compile_time_choice.o:  $(SDK)/src/rp2350/boot_stage2/compile_time_choice.S
	$(CC) -DLIB_BOOT_STAGE2_HEADERS=1 -DPICO_32BIT=1 -DPICO_BOARD=\"pico2\" -DPICO_BUILD=1 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1 -DPICO_RISCV=1 -DPICO_RP2350=1 \
-I$(SDK)/src/rp2350/boot_stage2/asminclude -isystem $(SDK)/src/rp2350/hardware_regs/include -isystem $(SDK)/src/rp2_common/hardware_base/include -isystem $(SDK)/src/common/pico_base_headers/include \
-isystem /home/ralexand/src/pico/blinky/no-usb/generated/pico_base -isystem $(SDK)/src/boards/include -isystem $(SDK)/src/rp2350/pico_platform/include -isystem $(SDK)/src/rp2_common/pico_platform_compiler/include -isystem $(SDK)/src/rp2_common/pico_platform_panic/include -isystem $(SDK)/src/rp2_common/pico_platform_sections/include -isystem $(SDK)/src/rp2_common/hardware_dcp/include -isystem $(SDK)/src/rp2350/hardware_structs/include -isystem $(SDK)/src/rp2_common/hardware_rcp/include -isystem $(SDK)/src/rp2350/boot_stage2/include -march=rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb -mabi=ilp32 -g -O3 -DNDEBUG -o $@ -c $^

bs2_default.elf: compile_time_choice.o
	$(CC) -march=rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb -mabi=ilp32 -g -O3 -DNDEBUG -Wl,--build-id=none --specs=nosys.specs -nostartfiles -Wl,--script=$(SDK)/src/rp2350/boot_stage2/boot_stage2.ld -Wl,-Map=bs2_default.elf.map $< -o $@

bs2_default.bin: bs2_default.elf
	/opt/pico-riscv/bin/riscv32-unknown-elf-objcopy -Obinary $< $@

bs2_default_padded_checksummed.S: bs2_default.bin
	/usr/bin/python3.12 $(SDK)/src/rp2350/boot_stage2/pad_checksum -s 0xffffffff -a riscv $< $@

stdlib.o:  $(SDK)/src/rp2_common/pico_stdlib/stdlib.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

gpio.o:  $(SDK)/src/rp2_common/hardware_gpio/gpio.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

platform.o:  $(SDK)/src/rp2350/pico_platform/platform.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

panic.o: $(SDK)/src/rp2_common/pico_platform_panic/panic.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

claim.o: $(SDK)/src/common/hardware_claim/claim.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

sync.o: $(SDK)/src/rp2_common/hardware_sync/sync.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

sync_spin_lock.o: $(SDK)/src/rp2_common/hardware_sync_spin_lock/sync_spin_lock.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

irq.o: $(SDK)/src/rp2_common/hardware_irq/irq.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

irq_handler_chain.o: $(SDK)/src/rp2_common/hardware_irq/irq_handler_chain.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

sem.o: $(SDK)/src/common/pico_sync/sem.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

lock_core.o: $(SDK)/src/common/pico_sync/lock_core.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

mutex.o: $(SDK)/src/common/pico_sync/mutex.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

critical_section.o: $(SDK)/src/common/pico_sync/critical_section.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

time.o:  $(SDK)/src/common/pico_time/time.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

timeout_helper.o: $(SDK)/src/common/pico_time/timeout_helper.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

timer.o: $(SDK)/src/rp2_common/hardware_timer/timer.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

datetime.o: $(SDK)/src/common/pico_util/datetime.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

pheap.o: $(SDK)/src/common/pico_util/pheap.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

queue.o: $(SDK)/src/common/pico_util/queue.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

uart.o: $(SDK)/src/rp2_common/hardware_uart/uart.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clocks.o: $(SDK)/src/rp2_common/hardware_clocks/clocks.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

pll.o: $(SDK)/src/rp2_common/hardware_pll/pll.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

vreg.o: $(SDK)/src/rp2_common/hardware_vreg/vreg.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

watchdog.o: $(SDK)/src/rp2_common/hardware_watchdog/watchdog.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

ticks.o:  $(SDK)/src/rp2_common/hardware_ticks/ticks.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

xosc.o: $(SDK)/src/rp2_common/hardware_xosc/xosc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

divider.o: $(SDK)/src/rp2_common/hardware_divider/divider.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

runtime.o:  $(SDK)/src/rp2_common/pico_runtime/runtime.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

runtime_init.o:  $(SDK)/src/rp2_common/pico_runtime_init/runtime_init.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

runtime_init_clocks.o: $(SDK)/src/rp2_common/pico_runtime_init/runtime_init_clocks.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

runtime_init_stack_guard.o:  $(SDK)/src/rp2_common/pico_runtime_init/runtime_init_stack_guard.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

bit_ops_aeabi.o:  $(SDK)/src/rp2_common/pico_bit_ops/bit_ops_aeabi.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

bootrom.o:  $(SDK)/src/rp2_common/pico_bootrom/bootrom.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

bootrom_lock.o: $(SDK)/src/rp2_common/pico_bootrom/bootrom_lock.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

boot_lock.o: $(SDK)/src/rp2_common/hardware_boot_lock/boot_lock.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

divider_compiler.o: $(SDK)/src/rp2_common/pico_divider/divider_compiler.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

malloc.o: $(SDK)/src/rp2_common/pico_malloc/malloc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

atomic.o: $(SDK)/src/rp2_common/pico_atomic/atomic.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

new_delete.o: $(SDK)/src/rp2_common/pico_cxx_options/new_delete.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

standard_binary_info.o: $(SDK)/src/rp2_common/pico_standard_binary_info/standard_binary_info.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

printf.o: $(SDK)/src/rp2_common/pico_printf/printf.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

crt0_riscv.o:  $(SDK)/src/rp2_common/pico_crt0/crt0_riscv.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

newlib_interface.o: $(SDK)/src/rp2_common/pico_clib_interface/newlib_interface.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

stdio.o: $(SDK)/src/rp2_common/pico_stdio/stdio.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

stdio_uart.o:  $(SDK)/src/rp2_common/pico_stdio_uart/stdio_uart.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

