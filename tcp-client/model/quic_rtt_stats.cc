#include <stdarg.h>
//__STDC_FORMAT_MACROS PRId64
#include <inttypes.h>
#include "quic_rtt_stats.h"
std::string ProtoStringPrintf(const char *format,...){
    size_t i=0;
    char buf[512]={'\0'};
    va_list ap;
    va_start(ap, format);
    i=vsnprintf(buf,512,format,ap);
    va_end(ap);
    return std::string(buf,i);
}
namespace ns3{
std::string QuicTimeDelta::ToDebuggingValue() const{
  return ProtoStringPrintf("%" PRId64 "us", time_offset_);
}
// Default initial rtt used before any samples are received.

const int kInitialRttMs = 100;
const float kAlpha = 0.125f;
const float kOneMinusAlpha = (1 - kAlpha);
const float kBeta = 0.25f;
const float kOneMinusBeta = (1 - kBeta);

NS_OBJECT_ENSURE_REGISTERED (RttStats);
TypeId  RttStats::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RttStats")
    .SetParent<Object> ();
  return tid;
}

RttStats::RttStats()
    : latest_rtt_(QuicTimeDelta::Zero()),
      min_rtt_(QuicTimeDelta::Zero()),
      smoothed_rtt_(QuicTimeDelta::Zero()),
      previous_srtt_(QuicTimeDelta::Zero()),
      mean_deviation_(QuicTimeDelta::Zero()),
      initial_rtt_(QuicTimeDelta::FromMilliseconds(kInitialRttMs)) {}

RttStats::RttStats(const RttStats & r)
:latest_rtt_(r.latest_rtt_),
    min_rtt_(r.min_rtt_),
    smoothed_rtt_(r.smoothed_rtt_),
    previous_srtt_(r.previous_srtt_),
    mean_deviation_(r.mean_deviation_),
    initial_rtt_(r.initial_rtt_){

}
TypeId RttStats::GetInstanceTypeId (void) const{
    return GetTypeId ();
}
Ptr<RttStats> RttStats::Copy () const{
    return CopyObject<RttStats> (this);
}
void RttStats::set_initial_rtt(QuicTimeDelta initial_rtt)
{
    if (initial_rtt.ToMicroseconds() <= 0)
    {
        return;
    }
    initial_rtt_= initial_rtt;
}

void RttStats::ExpireSmoothedMetrics()
{
  mean_deviation_ = std::max(
      mean_deviation_, QuicTimeDelta::FromMicroseconds(std::abs(
                           (smoothed_rtt_ - latest_rtt_).ToMicroseconds())));
  smoothed_rtt_ = std::max(smoothed_rtt_, latest_rtt_);
}

// Updates the RTT based on a new sample.
void RttStats::UpdateRtt(QuicTimeDelta send_delta)
{
  if (send_delta.IsInfinite() || send_delta <= QuicTimeDelta::Zero()) {
    return;
  }
  // Update min_rtt_ first. min_rtt_ does not use an rtt_sample corrected for
  // ack_delay but the raw observed send_delta, since poor clock granularity at
  // the client may cause a high ack_delay to result in underestimation of the
  // min_rtt_.
  if (min_rtt_.IsZero() || min_rtt_ > send_delta) {
    min_rtt_ = send_delta;
  }

  QuicTimeDelta rtt_sample(send_delta);
  previous_srtt_ = smoothed_rtt_;

  latest_rtt_ = rtt_sample;
  // First time call.
  if (smoothed_rtt_.IsZero()) {
    smoothed_rtt_ = rtt_sample;
    mean_deviation_ =
        QuicTimeDelta::FromMicroseconds(rtt_sample.ToMicroseconds() / 2);
  } else {
    mean_deviation_ = QuicTimeDelta::FromMicroseconds(static_cast<int64_t>(
        kOneMinusBeta * mean_deviation_.ToMicroseconds() +
        kBeta * std::abs((smoothed_rtt_ - rtt_sample).ToMicroseconds())));
    smoothed_rtt_ = kOneMinusAlpha * smoothed_rtt_ + kAlpha * rtt_sample;
  }
}

void RttStats::OnConnectionMigration()
{
  latest_rtt_ = QuicTimeDelta::Zero();
  min_rtt_ = QuicTimeDelta::Zero();
  smoothed_rtt_ = QuicTimeDelta::Zero();
  mean_deviation_ =QuicTimeDelta::Zero();
  initial_rtt_ = QuicTimeDelta::FromMilliseconds(kInitialRttMs);
}
}
