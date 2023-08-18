#include <linux/input.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/termbits.h> // struct termbits
#include <stdlib.h> // atoi
#include <string.h> // memset
#include <time.h>
#include <stdint.h>

void rel_x(int f, int x){
  struct input_event e;

  gettimeofday(&e.time, 0);
  e.type = EV_REL;
  e.code = REL_X;
  e.value = x;
  write(f, &e, sizeof(e));


  gettimeofday(&e.time, 0);
  e.type = EV_SYN;
  e.code = SYN_REPORT;
  e.value = 0;
  write(f, &e, sizeof(e));
}

void rel_y(int f, int x){
  struct input_event e;

  gettimeofday(&e.time, 0);
  e.type = EV_REL;
  e.code = REL_Y;
  e.value = x;
  write(f, &e, sizeof(e));


  gettimeofday(&e.time, 0);
  e.type = EV_SYN;
  e.code = SYN_REPORT;
  e.value = 0;
  write(f, &e, sizeof(e));
}


void btn_left(int f, int x){
  struct input_event e;

  gettimeofday(&e.time, 0);
  e.type = EV_KEY;
  e.code = BTN_LEFT;
  e.value = x;
  write(f, &e, sizeof(e));


  gettimeofday(&e.time, 0);
  e.type = EV_SYN;
  e.code = SYN_REPORT;
  e.value = 0;
  write(f, &e, sizeof(e));
}


void btn_right(int f, int x){
  struct input_event e;

  gettimeofday(&e.time, 0);
  e.type = EV_KEY;
  e.code = BTN_RIGHT;
  e.value = x;
  write(f, &e, sizeof(e));


  gettimeofday(&e.time, 0);
  e.type = EV_SYN;
  e.code = SYN_REPORT;
  e.value = 0;
  write(f, &e, sizeof(e));

}

void wheel(int f, int x){
  struct input_event e;

  gettimeofday(&e.time, 0);
  e.type = EV_REL;
  e.code = REL_WHEEL;
  e.value = x;
  write(f, &e, sizeof(e));


  gettimeofday(&e.time, 0);
  e.type = EV_SYN;
  e.code = SYN_REPORT;
  e.value = 0;
  write(f, &e, sizeof(e));
}

int main(int argc, char** argv) {


  int t = open(argv[1], O_RDONLY | O_NOCTTY);
  if(t == -1) {
    perror("Could not open tty device");
    return 1;
  }


  int f = open(argv[2], O_WRONLY);
  if(f == -1) {
    perror("Could not open input device");
    return 1;
  }

  struct termios tio;
  
  if(ioctl(t, TCGETS, &tio)){
    perror("could not get tty configuration");
    return 1;
  }

  tio.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
  tio.c_oflag = 0;
  tio.c_iflag = 0;
  tio.c_lflag = 0;

  if(ioctl(t, TCSETS, &tio)){
    perror("could not set configuration");
    return 1;
  }


  unsigned char buff[32];
  unsigned long long value = 0;
  unsigned long long prev_value = 0;
  double mouse_speed = 0;
  uint64_t t1, t2;
  
  for(;;){
    unsigned char bit_len = 0;
    read(t, &bit_len, 1);
    if(bit_len == 0) continue;

    printf("bit_len: %d %.2x ", bit_len, bit_len);
    unsigned char byte_len = ceil(bit_len/8.0f);
    if(byte_len >= sizeof(buff)) byte_len = sizeof(buff);

    memset(buff, 0, sizeof(buff));
    read(t, buff, byte_len);

    value = 0;
    for(int i=0; i<byte_len; i++){
      value = (value<<8) | buff[i];
    }

    if(bit_len <= 8){
      printf("u8 %.2x \n", (unsigned char)value);
    } else if(bit_len <= 16){
      printf("u16 %.4x \n", (unsigned short)value);
    } else if(bit_len <= 32){
      printf("u32 %.8x \n", (unsigned int)value);
    } else if(bit_len <= 64){
      printf("u64 %.16llx \n", (unsigned long long)value);
    } else {
      printf("\n");
    }

    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    t1 = (tspec.tv_sec&63)*1000 + tspec.tv_nsec / 1.0e6;
    printf("%ld", t1 - t2);
    if(value == 1 || (value == prev_value && (t1 - t2) < 200 ) ){
      value = prev_value;
      mouse_speed += mouse_speed * 0.17;
    }else{
      mouse_speed = 10.0;
    }
    t2 = t1;
    prev_value = value;

    if(value == 0x0001400401005253){ // up
      rel_y(f, -(int)mouse_speed);
    }
    if(value == 0x000140040100d2d3){ // down
      rel_y(f, (int)mouse_speed);
    }
    if(value == 0x0001400401007273){ // left
      rel_x(f, -(int)mouse_speed);
    }
    if(value == 0x000140040100f2f3){ // right
      rel_x(f, (int)mouse_speed);
    }

    if(value == 0x0001400409002c25){ // wheel up
      wheel(f, 1);
    }
    if(value == 0x000140040900aca5){ // wheel down
      wheel(f, -1);
    }

    if(value == 0x0001400409000009){ // left btn click
      btn_left(f, 1);
      btn_left(f, 0);
    }
    if(value == 0x0001400409005059){ // right btn click
      btn_right(f, 1);
      btn_right(f, 0);
    }

    if(value == 0x000140040d00bcb1){ // exit (turn off button)
      break;
    }

  }





  


  close(f);
  close(t);

  return 0;

}
