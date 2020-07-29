#pragma once
#include <stdint.h>
#include <cstddef>
#include <limits>
#include <cmath>
#include <string>
#include <ostream>
#include "ns3/object.h"
namespace ns3{
class QuicProtoTime;
class QuicTimeDelta{
public:
    static constexpr QuicTimeDelta Zero(){
        return QuicTimeDelta(0);
    }
    static constexpr QuicTimeDelta Infinite(){
        return QuicTimeDelta(kInfiniteTimeUs);
    }
    static constexpr QuicTimeDelta FromSeconds(int64_t secs){
        return QuicTimeDelta(secs*1000*1000);
    }
    static constexpr QuicTimeDelta FromMilliseconds(int64_t ms){
        return QuicTimeDelta(ms*1000);
    }
    static constexpr QuicTimeDelta FromMicroseconds(int64_t us){
        return QuicTimeDelta(us);
    }
    inline int64_t ToSeconds() const{
        return time_offset_/1000/1000;
    }
    inline int64_t ToMilliseconds() const{
        return time_offset_/1000;
    }
    inline int64_t ToMicroseconds() const{
        return time_offset_;
    }
    inline bool IsZero() const { return time_offset_ == 0; }

    inline bool IsInfinite() const {
      return time_offset_ == kInfiniteTimeUs;
    }
    std::string ToDebuggingValue() const;
private:
    friend inline bool operator==(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline bool operator!=(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline bool operator<(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline bool operator<=(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline bool operator>(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline bool operator>=(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline QuicTimeDelta operator>>(QuicTimeDelta lhs, size_t rhs);
    friend inline QuicTimeDelta operator+(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline QuicTimeDelta operator-(QuicTimeDelta lhs, QuicTimeDelta rhs);
    friend inline QuicTimeDelta operator*(QuicTimeDelta lhs, int rhs);
    friend inline QuicTimeDelta operator*(QuicTimeDelta lhs, double rhs);

    friend inline QuicProtoTime operator+(QuicProtoTime lhs, QuicTimeDelta rhs);
    friend inline QuicProtoTime operator-(QuicProtoTime lhs, QuicTimeDelta rhs);
    friend inline QuicTimeDelta operator-(QuicProtoTime lhs, QuicProtoTime rhs);


    static const int64_t kInfiniteTimeUs =
    std::numeric_limits<int64_t>::max();
    explicit constexpr QuicTimeDelta(int64_t time_offset)
    :time_offset_(time_offset){
    }
    int64_t time_offset_;
    friend class QuicProtoTime;
};
class QuicProtoTime{
public:
  // Creates a new QuicProtoTime with an internal value of 0.  IsInitialized()
  // will return false for these times.
  static constexpr QuicProtoTime Zero() { return QuicProtoTime(0); }

  // Creates a new QuicProtoTime with an infinite time.
  static constexpr QuicProtoTime Infinite() {
    return QuicProtoTime(QuicTimeDelta::kInfiniteTimeUs);
  }

  QuicProtoTime(const QuicProtoTime& other) = default;

  QuicProtoTime& operator=(const QuicProtoTime& other) {
    time_ = other.time_;
    return *this;
  }

  // Produce the internal value to be used when logging.  This value
  // represents the number of microseconds since some epoch.  It may
  // be the UNIX epoch on some platforms.  On others, it may
  // be a CPU ticks based value.
  inline int64_t ToDebuggingValue() const { return time_; }

  inline bool IsInitialized() const { return 0 != time_; }
private:
    friend inline bool operator==(QuicProtoTime lhs, QuicProtoTime rhs);
    friend inline bool operator<(QuicProtoTime lhs, QuicProtoTime rhs);
    friend inline QuicProtoTime operator+(QuicProtoTime lhs, QuicTimeDelta rhs);
    friend inline QuicProtoTime operator-(QuicProtoTime lhs, QuicTimeDelta rhs);
    friend inline QuicTimeDelta operator-(QuicProtoTime lhs, QuicProtoTime rhs);
    explicit constexpr QuicProtoTime(int64_t time) : time_(time) {}
    int64_t time_{0};
};
inline bool operator==(QuicTimeDelta lhs, QuicTimeDelta rhs){
    return lhs.time_offset_==rhs.time_offset_;
}
inline bool operator!=(QuicTimeDelta lhs, QuicTimeDelta rhs) {
  return !(lhs == rhs);
}
inline bool operator<(QuicTimeDelta lhs, QuicTimeDelta rhs) {
  return lhs.time_offset_ < rhs.time_offset_;
}
inline bool operator>(QuicTimeDelta lhs, QuicTimeDelta rhs) {
  return rhs < lhs;
}
inline bool operator<=(QuicTimeDelta lhs, QuicTimeDelta rhs) {
  return !(rhs < lhs);
}
inline bool operator>=(QuicTimeDelta lhs, QuicTimeDelta rhs) {
  return !(lhs < rhs);
}
inline QuicTimeDelta operator>>(QuicTimeDelta lhs, size_t rhs) {
  return QuicTimeDelta(lhs.time_offset_ >> rhs);
}


// Non-member relational operators for QuicProtoTime.
inline bool operator==(QuicProtoTime lhs, QuicProtoTime rhs) {
  return lhs.time_ == rhs.time_;
}
inline bool operator!=(QuicProtoTime lhs, QuicProtoTime rhs) {
  return !(lhs == rhs);
}
inline bool operator<(QuicProtoTime lhs, QuicProtoTime rhs) {
  return lhs.time_ < rhs.time_;
}
inline bool operator>(QuicProtoTime lhs, QuicProtoTime rhs) {
  return rhs < lhs;
}
inline bool operator<=(QuicProtoTime lhs, QuicProtoTime rhs) {
  return !(rhs < lhs);
}
inline bool operator>=(QuicProtoTime lhs, QuicProtoTime rhs) {
  return !(lhs < rhs);
}
// Override stream output operator for gtest or CHECK macros.
inline std::ostream& operator<<(std::ostream& output, const QuicProtoTime t) {
  output << t.ToDebuggingValue();
  return output;
}
// Non-member arithmetic operators for QuicTimeDelta.
inline QuicTimeDelta operator+(QuicTimeDelta lhs, QuicTimeDelta rhs) {
  return QuicTimeDelta(lhs.time_offset_ + rhs.time_offset_);
}
inline QuicTimeDelta operator-(QuicTimeDelta lhs, QuicTimeDelta rhs) {
  return QuicTimeDelta(lhs.time_offset_ - rhs.time_offset_);
}
inline QuicTimeDelta operator*(QuicTimeDelta lhs, int rhs) {
  return QuicTimeDelta(lhs.time_offset_ * rhs);
}
inline QuicTimeDelta operator*(QuicTimeDelta lhs, double rhs) {
  return QuicTimeDelta(
      static_cast<int64_t>(std::llround(lhs.time_offset_ * rhs)));
}
inline QuicTimeDelta operator*(int lhs, QuicTimeDelta rhs) {
  return rhs * lhs;
}
inline QuicTimeDelta operator*(double lhs, QuicTimeDelta rhs) {
  return rhs * lhs;
}
// Override stream output operator for gtest.
inline std::ostream& operator<<(std::ostream& output,
                                const QuicTimeDelta delta) {
  output << delta.ToDebuggingValue();
  return output;
}
// Non-member arithmetic operators for QuicProtoTime and QuicTimeDelta.
inline QuicProtoTime operator+(QuicProtoTime lhs, QuicTimeDelta rhs) {
  return QuicProtoTime(lhs.time_ + rhs.time_offset_);
}
inline QuicProtoTime operator-(QuicProtoTime lhs, QuicTimeDelta rhs) {
  return QuicProtoTime(lhs.time_ - rhs.time_offset_);
}
inline QuicTimeDelta operator-(QuicProtoTime lhs, QuicProtoTime rhs) {
  return QuicTimeDelta(lhs.time_ - rhs.time_);
}
class RttStats:public Object
{
  public:
    static TypeId GetTypeId (void);
    RttStats();
    RttStats(const RttStats & r);
    virtual TypeId GetInstanceTypeId (void) const;
    Ptr<RttStats> Copy () const;
    // Updates the RTT from an incoming ack which is received |send_delta| after
    // the packet is sent and the peer reports the ack being delayed |ack_delay|.
    // Time Unit: ms
    void UpdateRtt(QuicTimeDelta send_delta);
    // Causes the smoothed_rtt to be increased to the latest_rtt if the latest_rtt
    // is larger. The mean deviation is increased to the most recent deviation if
    // it's larger.
    void ExpireSmoothedMetrics();

    // Called when connection migrates and rtt measurement needs to be reset.
    void OnConnectionMigration();
    //for resample rtt
    void Reset() {OnConnectionMigration();}
    // Returns the EWMA smoothed RTT for the connection.
    // May return Zero if no valid updates have occurred.
    QuicTimeDelta smoothed_rtt() const { return smoothed_rtt_; }

    // Returns the EWMA smoothed RTT prior to the most recent RTT sample.
    QuicTimeDelta previous_srtt() const { return previous_srtt_; }

    QuicTimeDelta initial_rtt() const { return initial_rtt_; }
    QuicTimeDelta SmoothedOrInitialRtt() const {
    return smoothed_rtt_.IsZero() ? initial_rtt_ : smoothed_rtt_;
  }
    // Sets an initial RTT to be used for SmoothedRtt before any RTT updates.
    void set_initial_rtt(QuicTimeDelta initial_rtt);

    // The most recent rtt measurement.
    // May return Zero if no valid updates have occurred.
    QuicTimeDelta latest_rtt() const { return latest_rtt_; }

    // Returns the min_rtt for the entire connection.
    // May return Zero if no valid updates have occurred.
    QuicTimeDelta min_rtt() const { return min_rtt_; }

    QuicTimeDelta mean_deviation() const { return mean_deviation_; }
  private:
    QuicTimeDelta latest_rtt_;
    QuicTimeDelta min_rtt_;
    QuicTimeDelta smoothed_rtt_;
    QuicTimeDelta previous_srtt_;
    // Mean RTT deviation during this session.
    // Approximation of standard deviation, the error is roughly 1.25 times
    // larger than the standard deviation, for a normally distributed signal.
    QuicTimeDelta mean_deviation_;
    QuicTimeDelta initial_rtt_;
};
}