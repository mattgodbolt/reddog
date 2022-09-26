#ifndef _LOADING_H
#define _LOADING_H
extern Bool NoLoadingScreen;

void LoadPoll (void);
void BeginLoading(void);
void LoadSetWadAmount(float percent);
void EndLoading (void);

#endif
