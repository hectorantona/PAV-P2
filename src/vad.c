#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pav_analysis.h"

#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */

int count;
int firstFrames;
float k0;
float k1;
float k2;
float offset1, offset2;
int undefinedFrames;
int framesMS, framesMV;


/* 
   As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
   only this labels are needed. You need to add all labels, in case
   you want to print the internal state in string format */

const char *state_str[] = {
  "UNDEF", "S", "V", "MV", "MS", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  feat.p = compute_power(x, N);
  feat.am = compute_am(x, N);
  feat.zcr = compute_zcr(x, N, (float)1000*N/FRAME_TIME);
  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;

  count = 0;
  firstFrames = 10;
  k0 = 0.0;

  undefinedFrames = 0;

  framesMS = 11;
  framesMV = 0;
  
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  if (vad_data->state == ST_MS) vad_data->state = ST_VOICE;
  if (vad_data->state == ST_MV) vad_data->state = ST_SILENCE;
  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */
  
  switch (vad_data->state) {
  case ST_INIT:
    if (count < firstFrames) {
      count++;
      k0 = k0 + pow(10, f.p / 10);
    }
    else {
      k0 = 10 * log10(k0 / firstFrames);
      offset1 = -0.1 * k0 + 2.0;
      offset2 = -0.033 * offset1 + 2.0;

      k1 = k0 + offset1;
      k2 = k1 + offset2;
      vad_data->state = ST_SILENCE;
    }
    break;

  case ST_SILENCE:
    undefinedFrames = 0;
    if (f.p > k1) vad_data->state = ST_MV;
    break;

  case ST_VOICE:
    undefinedFrames = 0;
    if (f.p < k1) vad_data->state = ST_MS;
    break;

  case ST_MS:
    if (undefinedFrames < framesMS) undefinedFrames++;
    else
      if (f.p < k1) vad_data->state = ST_SILENCE;
      else vad_data->state = ST_VOICE;
    break;

  case ST_MV:
    if (undefinedFrames < framesMV) undefinedFrames++;
    else
      if (f.p > k2) vad_data->state = ST_VOICE;
      else vad_data->state = ST_SILENCE;
    break;

  case ST_UNDEF:
    break;
  }
  

  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE ||
      vad_data->state == ST_MS || vad_data->state == ST_MV)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
