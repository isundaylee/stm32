#include <iostream>

#include <libsigrokcxx/libsigrokcxx.hpp>

using std::cerr;
using std::endl;

static void datafeedCallback(std::shared_ptr<sigrok::Device>,
                             std::shared_ptr<sigrok::Packet> packet) {
  static size_t totalSamples = 0;

  if (packet->type() == sigrok::PacketType::HEADER) {
    // Nothing!
  } else if (packet->type() == sigrok::PacketType::LOGIC) {
    auto logic = std::static_pointer_cast<sigrok::Logic>(packet->payload());
    totalSamples += logic->data_length();
  } else if (packet->type() == sigrok::PacketType::END) {
    cerr << "Total samples: " << totalSamples << endl;
  } else {
    cerr << "Unhandled packet type: " << packet->type()->name() << endl;
  }
}

int main(int, char*[]) {
  auto context = sigrok::Context::create();

  auto driver = context->drivers().at("demo");
  auto device = driver->scan().at(0);
  cerr << "Number of channels: " << device->channels().size() << endl;
  device->open();
  for (auto const& channel : device->channels()) {
    channel->set_enabled(channel->index() < 4);
  }
  device->config_set(sigrok::ConfigKey::SAMPLERATE,
                     Glib::Variant<guint64>::create(1000000));
  device->config_set(sigrok::ConfigKey::LIMIT_SAMPLES,
                     Glib::Variant<guint64>::create(100000));

  auto session = context->create_session();
  session->add_device(device);
  session->add_datafeed_callback(datafeedCallback);
  session->start();
  session->run();
  session->stop();

  return 0;
}
