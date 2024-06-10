#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <asm/io.h>
#define IRQ_NUM 1

MODULE_LICENSE("GPL");

char * upper_codes[84] =  { " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "Backspace", 
                      "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Ctrl",
                      "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "\"", "'", "Shift (left)", "|", 
                      "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Shift (right)", 
                      "*", "Alt", "Space", "CapsLock", 
                      "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
                      "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
                      " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};

char * lower_codes[84] =  { " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "Backspace", 
                      "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Enter", "Ctrl",
                      "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "\"", "'", "Shift (left)", "|", 
                      "z", "x", "c", "v", "b", "n", "m", "<", ">", "?", "Shift (right)", 
                      "*", "Alt", "Space", "CapsLock", 
                      "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
                      "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
                      " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};

int flag = false;
int prev_code = 0;
struct tasklet_struct* interrupt_1;


void my_tasklet_function(unsigned long data)
{
    printk( ">> interrupt_1: Bottom handled time=%llu\n", ktime_get());
    int code = interrupt_1->data;
    if (interrupt_1->data < 84)
    {
        if ((flag && (prev_code == 42)) || (!flag && (prev_code != 42)))
        {
            printk(">> interrupt_1: Key is %s\n", lower_codes[code]);
        }
        else
        {
            printk(">> interrupt_1: Key is %s\n", upper_codes[code]);
        }
        prev_code = code;
    }
    printk(">> interrupt_1: ------------------------------------\n");
}

irqreturn_t my_handler(int irq, void *dev)
{
    int code;
    if (irq == IRQ_NUM)
    {
        code = inb(0x60);

 if (code & 0x80 && (code & 0x7F) < 84) {
  code &= 0x7F;

        if (code == 58)
        {
            flag = !flag;
        }

        interrupt_1->data = code;
        printk(">> interrupt_1: Key code is %d\n", code);
        tasklet_schedule(interrupt_1);
    printk(">> interrupt_1: Bottom sheduled time=%llu\n", ktime_get());
        return IRQ_HANDLED;
 }
    }
    return IRQ_HANDLED;
}

static int __init my_init(void)
{
    if (request_irq(IRQ_NUM, my_handler, IRQF_SHARED, "interrupt_1", &my_handler))
    {
        printk(">> interrupt_1: ERROR request_irq\n");
        return -1;
    }
    interrupt_1 = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    if (!interrupt_1)
    {
        printk(">> interrupt_1: ERROR kmalloc!\n");
        return -1;
    }
    tasklet_init(interrupt_1, my_tasklet_function, 0);

    printk(">> interrupt_1: init\n");
    return 0;
}

static void __exit my_exit(void)
{
    tasklet_kill(interrupt_1);
    kfree(interrupt_1);
    free_irq(IRQ_NUM, &my_handler);
    printk(">> interrupt_1: exit\n");
}

module_init(my_init)
module_exit(my_exit)
