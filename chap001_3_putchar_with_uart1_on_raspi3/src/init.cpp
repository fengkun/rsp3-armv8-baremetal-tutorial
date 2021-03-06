#include <cstdio>
#include <driver/gpio/GPIO.h>
#include <runtime_def.h>
#include <new>

constexpr size_t GPIO_BASE = 0x3F200000 ;

extern "C" void init();

__asm__(
	".section .text.boot \n\t"
	"mrs     x0, mpidr_el1 \n\t"
	"and     x0, x0, #0b11 \n\t"
	"cmp     x0,#0 \n\t"
	"b.eq      2f \n\t"
	"1:  		\n\t"
	"wfi         \n\t"
	"b       1b	\n\t"
	"2: \n\t"
	"ldr     x0, =__stack_top \n\t"
	"mov     sp,x0 \n\t"
	"bl      init \n\t"
	"b       1b \n\t"
);
__attribute__((section(".text.boot")))
void init()
{
	new (&gpio) GPIO(GPIO_BASE);

	// DOCME 倘若去掉下面这两行，则树莓派不能输出信息
	//       当然，正确的输出还需要适当的初始化
	//       从这里，就可以看出QEMU模拟与树莓派
	//       的差异，前者无需额外设置，后者还需
	//       许多设置。
	gpio.selectAltFunction(14, GPIO::ALT_5);
	gpio.selectAltFunction(15, GPIO::ALT_5);

	size_t n=20; // 注意，连接树莓派3可能有延迟，因此循环输出
	while(n--)
	{
		putchar('A');
		delayCPU(1000,1000);
	}
}

