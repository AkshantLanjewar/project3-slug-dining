#include "dining.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct dining {
  int capacity;
  int current_students;
  int cleaning_in_progress;
  int cleaning_waiting;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} dining_t;

dining_t *dining_init(int capacity) {
  dining_t *dining = malloc(sizeof(dining_t));
  dining->capacity = capacity;
  dining->current_students = 0;
  dining->cleaning_in_progress = 0;
  dining->cleaning_waiting = 0;
  pthread_mutex_init(&dining->mutex, NULL);
  pthread_cond_init(&dining->cond, NULL);
  return dining;
}

void dining_destroy(dining_t **dining_ptr) {
  dining_t *dining = *dining_ptr;
  pthread_mutex_destroy(&dining->mutex);
  pthread_cond_destroy(&dining->cond);
  free(dining);
  *dining_ptr = NULL;
}

void dining_student_enter(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  while (dining->current_students >= dining->capacity ||
         dining->cleaning_in_progress || dining->cleaning_waiting) {
    pthread_cond_wait(&dining->cond, &dining->mutex);
  }
  dining->current_students++;
  pthread_mutex_unlock(&dining->mutex);
}

void dining_student_leave(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  dining->current_students--;
  pthread_cond_broadcast(&dining->cond);
  pthread_mutex_unlock(&dining->mutex);
}

void dining_cleaning_enter(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  dining->cleaning_waiting++;
  while (dining->current_students > 0 || dining->cleaning_in_progress) {
    pthread_cond_wait(&dining->cond, &dining->mutex);
  }
  dining->cleaning_waiting--;
  dining->cleaning_in_progress = 1;
  pthread_mutex_unlock(&dining->mutex);
}

void dining_cleaning_leave(dining_t *dining) {
  pthread_mutex_lock(&dining->mutex);
  dining->cleaning_in_progress = 0;
  pthread_cond_broadcast(&dining->cond);
  pthread_mutex_unlock(&dining->mutex);
}
