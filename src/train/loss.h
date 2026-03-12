#ifndef TRAIN_LOSS_H
#define TRAIN_LOSS_H

double cross_entropy_loss(const float *logits, int vocab_size, int target_index);
double dpo_loss(const char *prompt, const char *chosen, const char *rejected);

#endif /* TRAIN_LOSS_H */

