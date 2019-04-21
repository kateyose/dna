#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DELAY 800
#define N 1000
#define BONDS 3
#define DEFAULT_SCREEN 50
#define SCREEN_X 400
#define SCREEN_Y 100
#define DIST 5
#define UNCONNECTED -1
#define CIRCLE 1
#define CROSS 0
#define OUTPUT "a.dat" 

const double MOVE_R = 0.9;
const double LINK = 0.9;
const double UNLINK = 0.1;
const int ds[2][4] = {{1, -1, 0, 0}, {0, 0, 1, -1}};
const int nd = 4;

typedef struct nucleobase {
  int v;
  int x;
  int y;
  int bond[3];
} nucleobase;
nucleobase adjust(nucleobase n);

double uniform_random() { return (double)rand() / (double)RAND_MAX; }

void nmove(nucleobase n[N]) {
  int i, j, l, k;
  for (i = 0; i < N; i++) {
    double r = uniform_random();
    if (r < MOVE_R) {
      for (l = 0; l < nd; l++) {
        if (r < (l + 1) * MOVE_R / nd) {
          n[i].x += ds[0][l];
          n[i].y += ds[1][l];
          for (k = 0; k < BONDS; k++) {
            if (n[i].bond[k] == -1) {
              continue;
            }
            if (abs(n[n[i].bond[k]].x - n[i].x) > DIST ||
                abs(n[n[i].bond[k]].y - n[i].y) > DIST) {
              n[i].x -= ds[0][l];
              n[i].y -= ds[1][l];
            }
          }
          break;
        }
      }
      n[i] = adjust(n[i]);
    }
  }
}

void print(nucleobase n[N]) {
  int i, j;
  for (i = 0; i < N; i++) {
    int b = 0;
    for (j = 0; j < BONDS; j++) {
      if (n[i].bond[j] > UNCONNECTED) {
        b += 1 << j;
      }
    }
    mvprintw(n[i].y, n[i].x, "%d%d", b, n[i].v);
  }
}

nucleobase adjust(nucleobase n) {
  if (n.x < 0) {
    n.x = 0;
  } else if (n.x >= SCREEN_X) {
    n.x = SCREEN_X - 1;
  }
  if (n.y < 0) {
    n.y = 0;
  } else if (n.y >= SCREEN_Y) {
    n.y = SCREEN_Y - 1;
  }
  return n;
}

int partner_index(int i) {
  if (i == 0) {
    return 1;
  }
  if (i == 1) {
    return 0;
  }
  if (i == 2) {
    return 2;
  }
  exit(1);
}

void link_unlink(nucleobase n[N]) {
  int i, j, l, k;
  for (i = 0; i < N; i++) {
    for (j = i + 1; j < N; j++) {
      if (abs(n[i].x - n[j].x) <= DIST && abs(n[i].y - n[j].y) <= DIST) {
        double r = uniform_random();
        if (r < LINK) {
          for (k = 0; k < BONDS; k++) {
            if (r < (k + 1) * LINK / BONDS && n[i].bond[k] == UNCONNECTED &&
                n[j].bond[partner_index(k)] == UNCONNECTED) {
              if (k == 2 && n[i].v == n[j].v){
		break;
              }
              n[i].bond[k] = j;
              n[j].bond[partner_index(k)] = i;
              break;
            }
          }
        }
      }
    }
  }

  for (i = 0; i < N; i++) {
    double r = uniform_random();
    if (r < UNLINK) {
      for (k = 0; k < BONDS; k++) {
        if (r < (k + 1) * UNLINK / BONDS) {
          int partner = n[i].bond[k];
          if (partner != UNCONNECTED) {
            n[partner].bond[partner_index(k)] = UNCONNECTED;
            n[i].bond[k] = UNCONNECTED;
            break;
          }
        }
      }
    }
  }
}

void analysis(nucleobase n[N]){
  int i;
  int flag[N] = {0};
  int len[N] = {0};
  for (i = 0; i < N; i++) {
    int count = 0;
    if(!flag[i]){
      int j = i;
      do{
	flag[j] = 1;
 	count++;
        j = n[j].bond[0];
      } while (j != UNCONNECTED && !flag[j]);
      len[count] ++;
    }
  }
  FILE*fd = fopen(OUTPUT, "a+");
  for (i = 0; i < 50; i++) {
     fprintf(fd, "%d ", len[i]);
  }
  fprintf(fd, "\n");
  fclose(fd);
}

int main(int argc, char *argv[]) {
  nucleobase n[N];

  int i, j;
  for (i = 0; i < N; i++) {
    n[i].v = i % 2 ? CIRCLE : CROSS;
    n[i].x = random() % SCREEN_X;
    n[i].y = random() % SCREEN_Y;
    for (j = 0; j < BONDS; j++) {
      n[i].bond[j] = -1;
    }
  }

  initscr();
  noecho();
  curs_set(FALSE);

  srand(time(NULL));
  fclose(fopen(OUTPUT, "w+"));

  while (1) {
    nmove(n);
    analysis(n);
    link_unlink(n);

    #ifdef DRAW
    clear();
    print(n);
    refresh();
    usleep(DELAY);
    #endif
  }

  endwin();
}
