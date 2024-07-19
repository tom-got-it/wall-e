typedef struct {
  int x;
  int y;
  int width;
  int height;
} rect;


const int ALARM_CLOCK_EACH_DAY = 1;
const int ALARM_CLOCK_WEEKDAYS = 2;
const int ALARM_CLOCK_WEEKEND = 3;
const int ALARM_CLOCK_MODES_LENGTH = 3;

const int ALARM_EXIT_NONE = -1;         //do nothing
const int ALARM_EXIT_TOUCH = 0;         //close the alarm
const int ALARM_EXIT_MISSED = 1;        //close the alarm and set the missed alarm flag
const int ALARM_EXIT_SNOOZE1 = 2;       //enter snooze mode using snooze1 option
const int ALARM_EXIT_SNOOZE2 = 3;       //enter snooze mode using snooze2 option
const int ALARM_EXIT_SNOOZE_END = 4;    //exit snooze-mode and go to alarm