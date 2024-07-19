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
  {
    DateTime(2029,10,28,3,0,0), -1
  },
  {
    DateTime(2030,03,31,2,0,0), 1
  },
  {
    DateTime(2030,10,27,3,0,0), -1
  },
  {
    DateTime(2031,03,30,2,0,0), 1
  },
  {
    DateTime(2031,10,26,3,0,0), -1
  },
  {
    DateTime(2032,03,28,2,0,0), 1
  },
  {
    DateTime(2032,10,31,3,0,0), -1
  },
  {
    DateTime(2033,03,27,2,0,0), 1
  },
  {
    DateTime(2033,10,30,3,0,0), -1
  },
  {
    DateTime(2034,03,26,2,0,0), 1
  },
  {
    DateTime(2034,10,29,3,0,0), -1
  },
  {
    DateTime(2035,03,25,2,0,0), 1
  },
  {
    DateTime(2035,10,28,3,0,0), -1
  },
  {
    DateTime(2036,03,30,2,0,0), 1
  },
  {
    DateTime(2036,10,26,3,0,0), -1
  },
  {
    DateTime(2037,03,29,2,0,0), 1
  },
  {
    DateTime(2037,10,25,3,0,0), -1
  }
};
