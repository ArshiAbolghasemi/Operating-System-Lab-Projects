#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");

int init_module(void)
{
	printk(KERN_INFO "Group 12:\nGolbol Rashidi\nErfan Ahmadi\nArshia Abolghasemi\n');
	return 0;
}

void cleanup_module(void) {}