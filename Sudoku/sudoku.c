/* Boicu Ionut 335 CA - Tema 3 APD */

#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MASTER   0
#define LINE_SIZE 40
#define TAG 1
#define StopRecv 100
#define DoesntExist 99

/* space to save all solutions for self and children */
int ****save, **solution, haveSol = 0;
int vector[16];

int test(int n, int size, int sudoku[size][size], int elem, int line, int col, int cornerI, int cornerJ) {
  int i, j;
  /* test line */
  j = col;
  for(i = 0; i < size; i++)
    if(sudoku[i][j] != 0 && sudoku[i][j] == elem)
      return 0;
  /* test column */
  i = line;
  for(j = 0; j < size; j++)
    if(sudoku[i][j] != 0 && sudoku[i][j] == elem)
      return 0;
  /* test square */
  for(i = cornerI; i < n + cornerI; i++)
    for(j = cornerJ; j < n + cornerJ; j++)
      if(sudoku[i][j] == elem)
      return 0;
  return 1;
}
/* Backtracking - n => square size | line/col => modified square | i/j => corner of bigger square */
void doSudokuLeaf (int n, int size, int sudoku[size][size], int line, int col, int cornerI, int cornerJ, int parent) {
  int k;
  /* empty square */
  if(sudoku[line][col] == 0) {
    for(k = 1; k <= size; k++) {
      /* pass test (row-col, in square) */
      if(test(n, size, sudoku, k, line, col, cornerI, cornerJ) == 1) {
        sudoku[line][col] = k;
        /* last square => send solution to parent */
        if(line == cornerI + n - 1 && col == cornerJ + n - 1)
          MPI_Send(sudoku, size*size, MPI_INT, parent, TAG, MPI_COMM_WORLD);
        /* go next line */
        else if(col == cornerJ + n - 1)
          doSudokuLeaf(n, size, sudoku, line + 1, cornerJ, cornerI, cornerJ, parent);
        /* go next column */
        else 
          doSudokuLeaf(n, size, sudoku, line, col + 1, cornerI, cornerJ, parent);
      }
    }
    sudoku[line][col] = 0;
  }
  /* last square => send solution to parent */
  else if(line == cornerI + n - 1 && col == cornerJ + n - 1)
    MPI_Send(sudoku, size*size, MPI_INT, parent, TAG, MPI_COMM_WORLD);
  /* go next line */
  else if(col == cornerJ + n - 1)
    doSudokuLeaf(n, size, sudoku, line + 1, cornerJ, cornerI, cornerJ, parent);
  /* go next column */
  else 
    doSudokuLeaf(n, size, sudoku, line, col + 1, cornerI, cornerJ, parent);
}

void doSudokuInter(int n, int where, int size, int sudoku[size][size], int count[where+1], int total[where+1], int line, int col, int cornerI, int cornerJ) {
   int k,l,i,j;
  /* empty square */
  if(sudoku[line][col] == 0) {
    for(k = 1; k <= size; k++) {
      /* pass test (row-col, in square) */
      if(test(n, size, sudoku, k, line, col, cornerI, cornerJ) == 1) {
        sudoku[line][col] = k;
        /* last square => send solution to parent */
        if(line == cornerI + n - 1 && col == cornerJ + n - 1) {
          /* check space available */
          if(count[where] == total[where]) {
          /* realloc double space */
            total[where] *= 2;
            save[where] = realloc(save[where], total[where] * sizeof(int**));
            for(l = total[where] / 2; l < total[where]; l++) {
              save[where][l] = malloc(size * sizeof(int *));
              for(i = 0; i < size;i++)
                save[where][l][i] = malloc(size * sizeof(int));
            }
          }
          /* save solution */
          for(i = 0; i < size; i++)
            for(j = 0; j < size; j++)
              save[where][count[where]][i][j] = sudoku[i][j];

          count[where] ++;
        }
        /* go next line */
        else if(col == cornerJ + n - 1)
          doSudokuInter(n, where, size, sudoku, count, total, line + 1, cornerJ, cornerI, cornerJ);
        /* go next column */
        else 
          doSudokuInter(n, where, size, sudoku, count, total, line, col + 1, cornerI, cornerJ);
      }
    }
    sudoku[line][col] = 0;
  }
  /* last square => send solution to parent */
  else if(line == cornerI + n - 1 && col == cornerJ + n - 1) {
    /* check space available */
    if(count[where] == total[where]) {
    /* realloc double space */
      total[where] *= 2;
      save[where] = realloc(save[where], total[where] * sizeof(int**));
      for(l = total[where] / 2; l < total[where]; l++) {
        save[where][l] = malloc(size * sizeof(int *));
        for(i = 0; i < size;i++)
          save[where][l][i] = malloc(size * sizeof(int));
      }
    }
    /* save solution */
    for(i = 0; i < size; i++)
      for(j = 0; j < size; j++)
        save[where][count[where]][i][j] = sudoku[i][j];
    count[where] ++;
  }
  /* go next line */
  else if(col == cornerJ + n - 1)
    doSudokuInter(n, where, size, sudoku, count, total, line + 1, cornerJ, cornerI, cornerJ);
  /* go next column */
  else 
    doSudokuInter(n, where, size, sudoku, count, total, line, col + 1, cornerI, cornerJ);
}

/* test just row-column, insquare property is estabelished from previous backtracking */
int testBKT(int size, int sudoku[size][size]) {
  int i, j, liniecol;
  /* test row */
  for(liniecol = 0; liniecol < size; liniecol++)
    for(i = 0; i < size; i++)
      for(j = 0; j < size; j++)
        if(i != j && sudoku[liniecol][i] == sudoku[liniecol][j] && sudoku[liniecol][i] != 0 && sudoku[liniecol][j] != 0)
           return 0;
          
  /* test column */
  for(liniecol = 0; liniecol < size; liniecol++)
    for(i = 0; i < size; i++)
      for(j = 0; j < size; j++)
        if(i != j && sudoku[i][liniecol] == sudoku[j][liniecol] && sudoku[i][liniecol] != 0 && sudoku[j][liniecol] != 0)
          return 0;
  return 1;
}

void BKT(int size, int step, int total, int parent, int count[total]) {
  /* solution already found */
  if(haveSol == 1)
    return;
  int i, j, k, p, l;
  /* solution */
  if(step == total + 1) {
    int auxSudoku[size][size];
    for(i = 0; i < size; i++)
        for(j = 0; j < size; j++)
          auxSudoku[i][j] = 0;
    for(p = 0; p <= total; p++)
      for(i = 0; i < size; i++)
        for(j = 0; j < size; j++)
          if(save[p][vector[p]][i][j] != 0)
            auxSudoku[i][j] = save[p][vector[p]][i][j];

    if(testBKT(size, auxSudoku) == 1) {
      /* if there is a parent send partial solution to him*/
      if(parent != -1)
        MPI_Send(auxSudoku, size * size, MPI_INT, parent, TAG, MPI_COMM_WORLD);
      else {
        /* reached root and save final solution */
        if(haveSol == 0) {
            solution = malloc (sizeof(int *) * size);
            for(i = 0; i < size; i++) {
              solution[i] = malloc (sizeof(int) * size);
              for(j =0;j < size;j++)
                solution[i][j] = auxSudoku[i][j];
            }
            haveSol = 1;
          }
        }
    }

  } else {
    for(k = 0; k < count[step]; k++) {
      vector[step] = k;
      BKT(size, step + 1, total, parent, count);
      vector[step] = -1;
    }
  }
}

int main(int argc, char *argv[]) {
  /* needed variables */
  int  numtasks, taskid, i, j, parent = -1, minimum = DoesntExist, msg[1];
  FILE *topology, *input, *output;
  int numChildren = 0, totChildren = 0, ACK[1] = {1}, NACK[1] = {0};
  char token_line[LINE_SIZE], *token;

  /* initialize MPI */
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  /*Phase 1: Read topology */
  topology = fopen(argv[1], "r");

  /* current node's topolgy */
  int halfT[numtasks+1];
  halfT[0] = 0;
  /* full topology */
  int fullT[numtasks][numtasks], message[numtasks][numtasks];
  for(i = 0; i < numtasks; i++)
    for(j = 0; j < numtasks; j++)
      fullT[i][j] = -1; /* initialize matrix */

  for(i = 0; i < numtasks && i - 1 != taskid; i++)
    if(fgets(token_line, LINE_SIZE, topology) != NULL);
  fclose(topology);
  /* get the first token */
  token = strtok(token_line, " ");
  /* walk through other tokens */
  while( token != NULL ) 
  {
    token = strtok(NULL, " ");
    
    /* check if token exists */
    if(token != NULL) {
      if(isdigit(token[0]) != 0) {
        /* save topology in halfT */
        halfT[++halfT[0]] = atoi(token);
       // printf("proc%d -> %d\n",taskid,halfT[halfT[0]]);
        /* get minimum node as parent */
        if(halfT[halfT[0]] < minimum)
          minimum = halfT[halfT[0]];
        numChildren ++;
      }
    }
  }
  /* if there is no minimum => terminal node */
  if(minimum != DoesntExist) {
    /* MASTER is root */
    if(taskid != MASTER) {
      /* send ack to 'minimum' parent => no cycle created */
      MPI_Send(ACK, 1, MPI_INT, minimum, TAG, MPI_COMM_WORLD);
      parent = minimum;
    }
    /* send nack to other nodes */
    for(i = 1; i <= halfT[0]; i++)
      if(halfT[i] != minimum || taskid == MASTER) {
        MPI_Send(NACK, 1, MPI_INT, halfT[i], TAG, MPI_COMM_WORLD);
      }
  }
  /* Receive ACK & NACK messages from neighbours */
  while(numChildren > 0) {
    MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
    if(msg[0] == ACK[0]) {
      /* source became taskid's child */
      fullT[taskid][status.MPI_SOURCE] = status.MPI_SOURCE;
      /* true children */
      totChildren++;
    }
    numChildren--;
  }
  /* wait for all to know their children */
  MPI_Barrier(MPI_COMM_WORLD);

  /* terminal node has no child */
  if(totChildren == 0) {
    for(i = 0; i < numtasks; i++)
      if(i != taskid)
        fullT[taskid][i] = parent;
    /* send current topology to parent */
    MPI_Send(fullT, numtasks*numtasks, MPI_INT, parent, totChildren, MPI_COMM_WORLD);
  } else {
    int k = 0;
    numChildren = totChildren;
    /* receive from every child */
    while(k < totChildren) {
      MPI_Recv(&message, numtasks*numtasks, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      /* save total branches children */
      numChildren += status.MPI_TAG;
      /* copy information from children */
      for(i = 0; i < numtasks; i++)
        for(j = 0; j < numtasks && i != taskid; j++)
          if(fullT[i][j] == -1)
            fullT[i][j] = message[i][j];
      /* unknown location form children */
      for(i = 0; i < numtasks; i++)
        if((fullT[taskid][i] == -1 || fullT[taskid][i] == parent)
          && i != taskid && message[status.MPI_SOURCE][i] != -1
          && message[status.MPI_SOURCE][i] != taskid)
          fullT[taskid][i] = status.MPI_SOURCE;

      /* link the parent to unused locations (-1) */
      for(i = 0; i < numtasks; i++)
        if(fullT[taskid][i] == -1 && i != taskid)
          fullT[taskid][i] = parent;
      k++;
    }
    /* send to parent after all receives*/
    if(taskid != MASTER) {
      MPI_Send(fullT, numtasks*numtasks, MPI_INT, parent, numChildren, MPI_COMM_WORLD);
    }
  }
  /* wait for all to finish routing */
  MPI_Barrier(MPI_COMM_WORLD);

  /* clear sudoku */
  int sudoku[numtasks][numtasks], sentSudoku[numtasks][numtasks];
  for(i = 0; i < numtasks; i++)
    for(j = 0; j < numtasks; j++)
      {sudoku[i][j] = sentSudoku[i][j] = 0;}

  /*Phase 2: root starts to split sudoku into pieces */
  input = fopen(argv[2], "r");
  int n, k, line, col;
  fscanf(input, "%d", &n);
  if(taskid == MASTER) {
    int x;
    for(i = 0; i < numtasks; i++)
      for(j = 0; j < numtasks; j++)
        fscanf(input, "%d ", &sudoku[i][j]);
    // exclude root
    for(k = 1; k < numtasks; k++) {
      for(i = 0; i < numtasks; i++)
        for(j = 0; j < numtasks; j++)
          sentSudoku[i][j] = sudoku[i][j];
      /* send sudoku square to process k; TAG = destination */
      MPI_Send(sentSudoku, numtasks*numtasks, MPI_INT, fullT[taskid][k], k, MPI_COMM_WORLD);
    }
  } else {
    /* everyone has to receive numChildren + 1(for itself) messages */
    numChildren ++;
    while(numChildren > 0) {

      MPI_Recv(&sentSudoku, numtasks*numtasks, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      /* forward message until tag == taskid*/
      if(status.MPI_TAG != taskid)
        MPI_Send(sentSudoku, numtasks*numtasks, MPI_INT, fullT[taskid][status.MPI_TAG], status.MPI_TAG, MPI_COMM_WORLD);
      else {
        /* save sentSudoku in "sudoku" */
        for(i = 0; i < numtasks; i++)
          for(j = 0; j < numtasks; j++)
            sudoku[i][j] = sentSudoku[i][j];
      }
      numChildren--;
    }
  }
  fclose(input);
  MPI_Barrier(MPI_COMM_WORLD);

  /*Phase 3: Solve each piece of sudoku and send solution to parent */
  /* additional resource for counting */
  int count[totChildren + 1];
  /* terminal node has no child */
  if(totChildren == 0) {
    /* all sudoku posiblities */
    line = taskid / n * n;
    col = taskid % n * n;
    doSudokuLeaf(n, numtasks, sudoku, line, col, line, col, parent);
    msg[0] = StopRecv;
    /* Send signal to parent of no more possible solutions */
    MPI_Send(msg, 1, MPI_INT, parent, TAG, MPI_COMM_WORLD);
  } else {
    /* final solution combined in root */
    int p,l;
    save = malloc((totChildren+ 1 )*sizeof(int***));
    /* keep order of children and their messages */
    int order[totChildren + 1], where;
    /* count children solutions */
    int total[totChildren + 1];
    for(i = 0; i <= totChildren; i++){
      order[i] = 0;
      count[i] = 0;
      /* initial size may be of 16 solutions */
      total[i] = 16;
    }
    /* allocate space for 16 solutions */
    for(p = 0; p <= totChildren; p++) {
      save[p] = malloc(total[0] * sizeof(int**));
      for(l = 0; l < total[0]; l++) {
        save[p][l] = malloc(numtasks * sizeof(int *));
        for(i = 0; i < numtasks;i++)
          save[p][l][i] = malloc(numtasks * sizeof(int));
      }
    }
    order[totChildren] = taskid;
    k = 0;
    while(k < totChildren) {
      MPI_Recv(&sentSudoku, numtasks*numtasks, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
      /* solution message */
      if(sentSudoku[0][0] != StopRecv) {
        /* find where to inset */
        for(i = 0; i < totChildren; i++)
          if(order[i] == status.MPI_SOURCE)
            {where = i; break;}
          else if(order[i] == 0) {
            order[i] = status.MPI_SOURCE;
            where = i;
            break;
          }
        /* if it doesn't fit then double the size and realloc */
        if(count[where] == total[where]) {
          /* double the size */
          total[where] *= 2;
          save[where] = realloc(save[where], total[where] * sizeof(int**));
          for(l = total[where]/2; l < total[where]; l++) {
            save[where][l] = malloc(numtasks * sizeof(int *));
            for(i = 0; i < numtasks;i++)
              save[where][l][i] = malloc(numtasks * sizeof(int));
          }
        }
        /* now there is enough size to save partial solutions */
        for(i = 0; i < numtasks; i++)
          for(j = 0; j < numtasks; j++)
            save[where][count[where]][i][j] = sentSudoku[i][j];
        count[where] ++;
      }
      /* 1 child has stopped sending solutions */
      if(sentSudoku[0][0] == StopRecv) 
        k++;
    }
    /* make partial solutions for my own square */
    line = taskid / n * n;
    col = taskid % n * n;
    doSudokuInter(n, totChildren, numtasks, sudoku, count, total, line, col, line, col);   

    /* combine all partial solutions using backtracking */
    BKT(numtasks, 0, totChildren, parent, count);

    if(taskid != MASTER) {
      msg[0] = StopRecv;
      /* Send signal to parent of no more possible solutions */
      MPI_Send(msg, 1, MPI_INT, parent, TAG, MPI_COMM_WORLD);
    }
  } 
  if(taskid == MASTER) {
    output = fopen(argv[3], "w");
    fprintf(output,"%d\n", n);
    for(i = 0; i < numtasks; i++) {
      for(j = 0; j < numtasks; j++)
        fprintf(output, "%d ", solution[i][j]);
      fprintf(output, "\n");
    }
    /* free resources */
    fclose(output);
    for(i = 0; i < numtasks; i++)
        free(solution[i]);
    free(solution);
  }
  /* free resources */
  MPI_Finalize();
  for(i = 0; i < totChildren; i++) {
    for(j = 0; j < count[i]; j++) {
      for(k = 0; k < numtasks; k++)
        free(save[i][j][k]);
      free(save[i][j]);
    }
    free(save[i]);
  }
  free(save);
  return 0;
}