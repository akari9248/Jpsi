#include <iostream>
class ProcessBar {
private:
  TString time_Cal(clock_t startTime, clock_t endTime) {
    clock_t time0 = endTime - startTime;
    int time = time0 / CLOCKS_PER_SEC;
    int hours = time / 3600;
    int minutes = (time - hours * 3600) / 60;
    int seconds = time - hours * 3600 - minutes * 60;
    TString Time = "";
    if (hours != 0) {
      Time = Time + TString::Format("%dh ", hours);
    }
    if (minutes != 0) {
      Time = Time + TString::Format("%dm ", minutes);
    }

    Time = Time + TString::Format("%ds ", seconds);
    return Time;
  }

  void process_bar(int index, int sum = 1, TString prefix = "",
                   TString suffix = "", bool endline = false) {
    float progress = 0.0;
    // if(index==0) cout<<endl;
    progress = index / sum;
    int barWidth = 70;

    std::cout << prefix ;
    if (endline) {
      std::cout << " . . . "<< suffix;
      std::cout << std::endl;
    } else {
      std::cout << "[";
      int pos = barWidth * progress;
      for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
          std::cout << "=";
        else if (i == pos)
          std::cout << ">";
        else
          std::cout << " ";
      }
      std::cout << "] " << int(progress * 100.0) << " % " << suffix << " \r ";
      std::cout.flush();

      if (progress == 1) {
        std::cout << std::endl;
        // std::cout.flush();
      }
    }
  }

public:
  int entries;
  clock_t startTime;
  int count;
  TString lasttime;
  int lastk;
  ProcessBar(int _entries) : entries(_entries) {
    startTime = clock();
  }

  void show(int k) {
    clock_t endTime = clock();
    TString usedtime = time_Cal(startTime, endTime);
    if (usedtime == lasttime) {
      TString lefttime =
          time_Cal(startTime,
                   (endTime - startTime) * (entries - k) / (k + 1) + startTime);
      process_bar(k, entries, " Analyzing :",
                  "Used time: " + usedtime + "Expected left time: " + lefttime +
                      "         ");
    }

    lasttime = usedtime;
  }
  void show2(int k) {
    int unit_entry=entries/100;
    if (k  % unit_entry == 0&&k!=lastk) {
      clock_t endTime = clock();
      TString usedtime = time_Cal(startTime, endTime);
      TString lefttime =
          time_Cal(startTime,
                   (endTime - startTime) * (entries - k) / (k + 1) + startTime);
      process_bar(k, entries, " Analyzing ",
                  "Used time: " + usedtime + "Expected left time: " + lefttime +
                      "         ",
                  true);
    }
    lastk=k;
  }
};
