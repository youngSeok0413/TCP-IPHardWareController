#ifndef SEVENSEG_H
#define SEVENSEG_H

void segMain(int write_fd, int read_fd);

const int gpiopins[4] = {4, 1, 16, 15};
const int number[10][4] = {{0, 0, 0, 0},  /* 0 */
                     {0, 0, 0, 1},  /* 1 */
                     {0, 0, 1, 0},  /* 2 */
                     {0, 0, 1, 1},  /* 3 */
                     {0, 1, 0, 0},  /* 4 */
                     {0, 1, 0, 1},  /* 5 */
                     {0, 1, 1, 0},  /* 6 */
                     {0, 1, 1, 1},  /* 7 */
                     {1, 0, 0, 0},  /* 8 */
                     {1, 0, 0, 1}}; /* 9 */

#endif
