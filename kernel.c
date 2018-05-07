#include <stddef.h>
#include <stdint.h>

char *bad_input = "Bad input!\n";
char *goodby = "Goodby\n";

// Available memory. 1GB. 0x00000000 to 0x3FFFFFFF
// IO address range 0x3F000000 to 0x3FFFFFFF
extern void *__heap_start;
uint32_t __heap_end = 0x3FFFFFFF;

struct Node {
  unsigned int x;
  char z[5];
};

typedef struct Node Node;

typedef int bool;
enum {
  true = 1,
  false = 0,
};

bool eval(unsigned int *o);
bool subexpr(unsigned int left, unsigned int *o);

static void mmio_write(uint32_t reg, uint32_t data) {
  *(volatile uint32_t*)reg = data;
}

static uint32_t mmio_read(uint32_t reg) {
  return *(volatile uint32_t*)reg;
}

static void delay(int32_t count) {
  asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
	       : "=r"(count): [count]"0"(count) : "cc");
}


enum {
  // The GPIO registers base address.
  GPIO_BASE = 0x3F200000, // for raspi2 & 3, 0x20200000 for raspi1
  
  // The offsets for reach register.
  
  // Controls actuation of pull up/down to ALL GPIO pins.
  GPPUD = (GPIO_BASE + 0x94),
  
  // Controls actuation of pull up/down for specific GPIO pin.
  GPPUDCLK0 = (GPIO_BASE + 0x98),
  
  // The base address for UART.
  UART0_BASE = 0x3F201000, // for raspi2 & 3, 0x20201000 for raspi1
  
  // The offsets for reach register for the UART.
  UART0_DR     = (UART0_BASE + 0x00),
  UART0_RSRECR = (UART0_BASE + 0x04),
  UART0_FR     = (UART0_BASE + 0x18),
  UART0_ILPR   = (UART0_BASE + 0x20),
  UART0_IBRD   = (UART0_BASE + 0x24),
  UART0_FBRD   = (UART0_BASE + 0x28),
  UART0_LCRH   = (UART0_BASE + 0x2C),
  UART0_CR     = (UART0_BASE + 0x30),
  UART0_IFLS   = (UART0_BASE + 0x34),
  UART0_IMSC   = (UART0_BASE + 0x38),
  UART0_RIS    = (UART0_BASE + 0x3C),
  UART0_MIS    = (UART0_BASE + 0x40),
  UART0_ICR    = (UART0_BASE + 0x44),
  UART0_DMACR  = (UART0_BASE + 0x48),
  UART0_ITCR   = (UART0_BASE + 0x80),
  UART0_ITIP   = (UART0_BASE + 0x84),
  UART0_ITOP   = (UART0_BASE + 0x88),
  UART0_TDR    = (UART0_BASE + 0x8C),
};

void uart_init() {
  mmio_write(UART0_CR, 0x00000000);

  mmio_write(GPPUD, 0x00000000);

  delay(150);

  mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
  delay(150);

  mmio_write(GPPUDCLK0, 0x00000000);
  mmio_write(UART0_ICR, 0x7FF);

  mmio_write(UART0_IBRD, 1);
  mmio_write(UART0_FBRD, 40);

  mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

  mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	     (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

  mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c) {
  while(mmio_read(UART0_FR) & (1 << 5)) {}
  mmio_write(UART0_DR, c);
}

unsigned char uart_getc() {
  while(mmio_read(UART0_FR) & (1 << 4)) {}
  return mmio_read(UART0_DR);
}

void uart_puts(const char* str) {
  for (size_t i = 0; str[i] != '\0'; i++) {
    uart_putc((unsigned char)str[i]);
  }
}

unsigned char getc_printc() {
  unsigned char c = uart_getc();
  uart_putc(c);
  return c;
}

bool uchar_to_uint(unsigned char c, unsigned int *o) {
  if (c >= '0' && c <= '9') {
    *o = (unsigned int)(c - '0');
    return true;
  } 	
  return false; 
}

bool div(unsigned int o1, unsigned int o2, unsigned int *o) {
  if (o2 == 0) {
    return false;
  }
  unsigned int count = 0;
  while (o1 >= o2) {
    o1 -= o2;
    count++;
  }
  *o = count;
  return true;
}

bool mod(unsigned int o1, unsigned int o2, unsigned int *o) {
  if (o2 == 0) {
    return false;
  }

  while (o1 > o2) {
    o1 -= o2;
  }
  *o = o1;
  return true;
}

void uart_put_uint(unsigned int i) {
  // short circuit on zero
  if (i == 0) {
    uart_puts("0\n");
    return;
  }
  
  bool encountered_significant = false;
  unsigned int n = 1000000000;
  do {
    unsigned int digit;
    div(i, n, &digit);
    if (!encountered_significant) {
      if (digit == 0) {
	n /= 10;
	continue;
      } else {
	encountered_significant = true;
      }
    }
    uart_putc('0' + (unsigned char)digit);
    i -= (digit * n);
    n /= 10;
  } while (n >= 1);

  uart_putc('\n');
}

bool is_operator(unsigned char op) {
  return op == '+' || op == '-' || op == '*' || op == '/';
}

bool do_operation(unsigned char op, unsigned int *o1, unsigned int o2) {
  switch (op) {
  case '+':
    *o1 += o2;
    break;
  case '-':
    *o1 -= o2;
    break;
  case '*':
    *o1 *= o2;
    break;
  case '/':
    return div(*o1, o2, o1);
  default:
    return false;
  }
  return true;
}

void ghetto_print() {
  Node *n = (Node *)__heap_start;
  uart_put_uint(n->x);
  uart_puts(n->z);
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
  (void) r0;
  (void) r1;
  (void) atags;

  uart_init();

  Node *n = (Node *)__heap_start;
  *n = (Node) {
    .x = 98273,
    .z = "1245\n"
  };

  uart_put_uint(0xFFFFFFFF);

  ghetto_print();
  
  char *x = "0123456789\n";
  char *s = (char*)__heap_start;
  
  do {
    *s++ = *x++;
  } while (*x != '\0');
  
  uart_puts((char*)__heap_start);
  ghetto_print();

  for (;;) {
    uart_puts("> ");
    unsigned int o = 0;
    unsigned char oc = getc_printc();
    if (oc == ';') {
      goto exit;
    }
    if (!uchar_to_uint(oc, &o)) {
      break;
    }

    unsigned char op = getc_printc();
    unsigned char o2c = getc_printc();
    unsigned int o2;
    if(!(uchar_to_uint(o2c, &o2))) {
      break;
    }
    
    if (!do_operation(op, &o, o2)) {
      break;
    }
    
    for (;;) {
      op = getc_printc();
      if (op == ';') {
  	break;
      } else if (!is_operator(op)) {
  	goto bad_exit;
      }

      o2c = getc_printc();
      if (!(uchar_to_uint(o2c, &o2))) {
  	goto bad_exit;
      }
      
      if (!(do_operation(op, &o, o2))) {
  	goto bad_exit;
      }
    }
    uart_putc('\n');
    uart_put_uint(o);
    uart_putc('\n');
  }

 bad_exit:
  uart_putc('\n');
  uart_puts(bad_input);

 exit:
  uart_putc('\n');
  uart_puts(goodby);
}
