char *order_to_hour(int order) {
  switch (order) {
  case 1:
    return "7:10-7:55";
  case 2:
    return "8:00-8:45";
  case 3:
    return "8:50-9:35";
  case 4:
    return "9:40-10:25";
  case 5:
    return "10:40-11:25";
  case 6:
    return "11:30-12:15";
  case 7:
    return "12:20-13:05";
  case 8:
    return "13:10-13:55";
  case 9:
    return "14:00-14:45";
  case 10:
    return "14:50-15:35";
  case 11:
    return "15:40-16:35";
  case 12:
    return "16:40-17:25";
  case 13:
    return "17:30-18:15";
  default:
    return "";
  }
}

char *order_to_hour_shortened(int order) {
  switch (order) {
  case 1:
    return "7:10-7:40";
  case 2:
    return "7:45-8:15";
  case 3:
    return "8:20-8:50";
  case 4:
    return "8:55-9:25";
  case 5:
    return "9:30-10:00";
  case 6:
    return "10:05-10:35";
  case 7:
    return "10:40-11:10";
  case 8:
    return "11:15-11:45";
  case 9:
    return "11:50-12:20";
  case 10:
    return "12:25-12:55";
  case 11:
    return "13:00-13:30";
  case 12:
    return "13:35-14:05";
  case 13:
    return "14:10-14:40";
  default:
    return "";
  }
}
