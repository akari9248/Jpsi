#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <cmath>

class ProgressBar
{
private:
  int progress;
  int total;
  int width;
  int lastOutputPercent;
  std::chrono::steady_clock::time_point startTime;
  std::chrono::steady_clock::time_point lastUpdateTime;
  std::chrono::milliseconds updateInterval;
  bool forceComplete; // Flag to force completion at the end

  double getElapsedSeconds() const
  {
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration elapsedTime = currentTime - startTime;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count();
  }

  double getEstimatedTotalSeconds() const
  {
    double elapsedSeconds = getElapsedSeconds();
    double percent = static_cast<double>(progress) / total;
    return elapsedSeconds / percent;
  }

  double getEstimatedRemainingSeconds() const
  {
    double elapsedSeconds = getElapsedSeconds();
    double estimatedTotalSeconds = getEstimatedTotalSeconds();
    return estimatedTotalSeconds - elapsedSeconds;
  }

public:
  ProgressBar(int total, int width = 50, int updateIntervalMillis = 500)
      : progress(0), total(total), width(width), updateInterval(updateIntervalMillis), forceComplete(false), lastOutputPercent(-1)
  {
    startTime = std::chrono::steady_clock::now();
    lastUpdateTime = startTime;
  }

  ~ProgressBar()
  {
    std::cout << std::endl;
  }

  void update(int progress)
  {
    if (forceComplete && progress < total)
    {
      progress = total; // Force progress to reach total if forceComplete flag is set
    }

    this->progress = progress;

    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration elapsedDuration = currentTime - lastUpdateTime;
    std::chrono::milliseconds elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedDuration);

    if (elapsedMillis < updateInterval && progress != total)
    {
      return;
    }

    lastUpdateTime = currentTime;

    float percent = static_cast<float>(progress) / total;
    int filledWidth = static_cast<int>(std::round(percent * width));

    std::string progressBar(filledWidth, '>');
    std::string remainingSpace(width - filledWidth, ' ');

    double elapsedSeconds = getElapsedSeconds();
    double estimatedRemainingSeconds = getEstimatedRemainingSeconds();

    int elapsedHours = static_cast<int>(elapsedSeconds / 3600);
    int elapsedMinutes = static_cast<int>((elapsedSeconds - elapsedHours * 3600) / 60);
    int elapsedSecs = static_cast<int>(elapsedSeconds - elapsedHours * 3600 - elapsedMinutes * 60);

    int remainingHours = static_cast<int>(estimatedRemainingSeconds / 3600);
    int remainingMinutes = static_cast<int>((estimatedRemainingSeconds - remainingHours * 3600) / 60);
    int remainingSecs = static_cast<int>(estimatedRemainingSeconds - remainingHours * 3600 - remainingMinutes * 60);

    std::cout << "\r"
              << "Progress: [" << progressBar << remainingSpace << "] " << static_cast<int>(percent * 100.0) << "%"
              << " Elapsed: " << elapsedHours << "h " << elapsedMinutes << "m " << elapsedSecs << "s"
              << " Remaining: " << remainingHours << "h " << remainingMinutes << "m " << remainingSecs << "s"
              << std::flush;
  }

  void update2(int progress)
  {
    // static int lastOutputPercent = -1; // 记录上一次输出的百分比
    if (forceComplete && progress < total)
    {
      progress = total; // 如果 forceComplete 为 true，强制进度等于 total
    }
    this->progress = progress;
    // 计算当前百分比
    int currentPercent = static_cast<int>(static_cast<float>(progress) / total * 100);
    // 如果当前百分比与上一次输出的百分比相同，或者小于上一次，直接返回
    if (currentPercent <= lastOutputPercent)
    {
      return;
    }
    // 确保每次只增加 1% 或更多时输出
    if (currentPercent - lastOutputPercent < 1)
    {
      return;
    }
    // 更新最后一次输出的百分比
    lastOutputPercent = currentPercent;
    // 获取时间信息
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration elapsedDuration = currentTime - lastUpdateTime;
    std::chrono::milliseconds elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedDuration);
    if (elapsedMillis < updateInterval && progress != total)
    {
      return;
    }
    lastUpdateTime = currentTime;
    double elapsedSeconds = getElapsedSeconds();
    double estimatedRemainingSeconds = getEstimatedRemainingSeconds();
    int elapsedHours = static_cast<int>(elapsedSeconds / 3600);
    int elapsedMinutes = static_cast<int>((elapsedSeconds - elapsedHours * 3600) / 60);
    int elapsedSecs = static_cast<int>(elapsedSeconds - elapsedHours * 3600 - elapsedMinutes * 60);
    int remainingHours = static_cast<int>(estimatedRemainingSeconds / 3600);
    int remainingMinutes = static_cast<int>((estimatedRemainingSeconds - remainingHours * 3600) / 60);
    int remainingSecs = static_cast<int>(estimatedRemainingSeconds - remainingHours * 3600 - remainingMinutes * 60);
    // 输出每 1% 的进度信息
    std::cout << currentPercent << "% | "
              << "Elapsed: " << elapsedHours << "h " << elapsedMinutes << "m " << elapsedSecs << "s | "
              << "Remaining: " << remainingHours << "h " << remainingMinutes << "m " << remainingSecs << "s"
              << std::endl;
  }

  void setForceComplete(bool value)
  {
    forceComplete = value;
  }
};