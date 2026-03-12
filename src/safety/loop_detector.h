#ifndef SAFETY_LOOP_DETECTOR_H
#define SAFETY_LOOP_DETECTOR_H

int loop_detector_record(const char *tool_name, const char *args_hash);
int loop_detector_is_stuck(void);
int loop_detector_last_sequence(char *buf, int cap);

#endif /* SAFETY_LOOP_DETECTOR_H */

