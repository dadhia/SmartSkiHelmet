#ifndef EMIC
#define EMIC

void initialize_emic();
void emic_slope_speak(char *run);
void emic_no_gps_fix();
void emic_CO_speak(int current_CO_level);
void emic_CO_no_data();

#endif