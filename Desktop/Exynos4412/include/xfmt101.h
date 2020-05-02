#ifndef __XFMT101_H
#define __XFMT101_H

/*******XFMT101初始化************/
int XFMT101_Init();

/*******计算校验位****************/
unsigned char cal_check_bit(unsigned char *Command);


/*******温湿度语音合成*************/
void TemAndHum_Voicecps(int xfd, unsigned char tem, unsigned char hum);



/******烟雾警报语音合成*************/
void SmokeAlarm_Voicecps(int xfd, int smokeDensity);

/******防盗警报语音合成*************/
void BurglarAlarm_Voicecps(int xfd);


/*****开门提醒语音合成************/
void OpenDoor_Voicecps(int xfd);

/*****离家模式提醒语音合成************/
void Leave_Voicecps(int xfd);


/*****回家模式提醒语音合成************/
void GoHome_Voicecps(int xfd);

#endif