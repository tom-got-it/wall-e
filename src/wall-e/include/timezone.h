typedef struct {
  DateTime when;
  int offset;
} timezone_change;

const timezone_change TIMEZONE_CHANGES[] = {
  {
    DateTime(2024,3,31,2,0,0), 1
  },
  {
    DateTime(2024,10,27,3,0,0), -1
  },
  {
    DateTime(2025,03,30,2,0,0), 1
  },
  {
    DateTime(2025,10,26,3,0,0), -1
  },
  {
    DateTime(2026,03,29,2,0,0), 1
  },
  {
    DateTime(2026,10,25,3,0,0), -1
  },
  {
    DateTime(2027,03,28,2,0,0), 1
  },
  {
    DateTime(2027,10,31,3,0,0), -1
  },
  {
    DateTime(2028,03,26,2,0,0), 1
  },
  {
    DateTime(2028,10,29,3,0,0), -1
  },
  {
    DateTime(2029,03,25,2,0,0), 1
  },
};
