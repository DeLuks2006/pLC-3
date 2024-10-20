#include "../include/utils.h"

struct termios original_tio;

uint16_t sign_extend(uint16_t x, int bit_count) {
  if ((x >> (bit_count - 1)) & 1) {
    x |= (0xFFFF << bit_count);
  }
  return x;
}

uint16_t swap16(uint16_t x) { return (x << 8) | (x >> 8); }

int read_image(const char *image_path) {
  FILE *file = fopen(image_path, "rb");
  if (!file) {
    return 0;
  }
  read_image_file(file);
  fclose(file);
  return 1;
}

uint16_t mem_read(uint16_t address) {
  if (address == MR_KBSR) {
    if (check_key()) {
       memory[MR_KBSR] = (1 << 15);
       memory[MR_KBDR] = getchar();
    } else {
       memory[MR_KBSR] = 0;
    }
  }
  return memory[address];
}

void disable_input_buffering() {
  tcgetattr(STDIN_FILENO, &original_tio);
  struct termios new_tio = original_tio;
  new_tio.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering() {
  tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

uint16_t check_key() {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

void handle_interrupt() {
  restore_input_buffering();
  printf("\n");
  exit(-2);
}

void mem_write(uint16_t address, uint16_t val) { memory[address] = val; }

void read_image_file(FILE *file) {
  uint16_t origin;
  fread(&origin, sizeof(origin), 1, file);
  origin = swap16(origin);

  uint16_t max_read = MEMORY_MAX - origin;
  uint16_t *p = memory + origin;
  size_t read = fread(p, sizeof(uint16_t), max_read, file);

  while (read-- > 0) {
    *p = swap16(*p);
    ++p;
  }
}

void update_flags(uint16_t r) {
  if (reg[r] == 0) {
    reg[R_COND] = FL_ZRO;
  } else if (reg[r] >> 15) {
    reg[R_COND] = FL_NEG;
  } else {
    reg[R_COND] = FL_POS;
  }
}

int read_image_from_array(char *image_data, unsigned int image_size) {
    if (image_size < sizeof(uint16_t)) {
        return 0; // Not enough data to read origin
    }
    // Read the origin from the array
    uint16_t origin;
    origin = *(uint16_t *)image_data;
    origin = swap16(origin);

    // Calculate the maximum number of uint16_t values we can read
    uint16_t max_read = MEMORY_MAX - origin;
    if (image_size < sizeof(uint16_t) + max_read * sizeof(uint16_t)) {
        return 0; // Not enough data in the array
    }
    // Pointer to the memory location
    uint16_t *p = memory + origin;
    // Read the rest of the data from the array
    const uint16_t *data_ptr = (const uint16_t *)(image_data + sizeof(uint16_t));
    size_t read = (image_size - sizeof(uint16_t)) / sizeof(uint16_t);
    // Limit the read to max_read
    if (read > max_read) {
        read = max_read;
    }
    // Swap and store the data
    for (size_t i = 0; i < read; ++i) {
        *p = swap16(data_ptr[i]);
        ++p;
    }
    return 1; // Success
}
