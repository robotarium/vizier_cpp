#include "vizier/utils/mqttclient/mqttclient_async.h"
#include <iostream>
#include <chrono>
#include <memory>

auto start = std::chrono::steady_clock::now();

void callback(std::string topic, std::string message) {
  auto now = std::chrono::steady_clock::now();
  std::cout << "length: " << message.length() << std::endl;
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(now - start).count() << std::endl;
  start = now;
}

std::function<void(std::string, std::string)> stdf_callback = callback;

int main() {

  std::string topic = "test_topic";

  std::unique_ptr<MqttClientAsync> m;

  try {
    m = std::make_unique<MqttClientAsync>("192.168.1.8", 1884); 
  } catch (const std::exception& e) {
    return -1; 
  }

  std::cout << "Started mqtt client" << std::endl;

  bool ok = m->subscribe_with_callback(topic, stdf_callback);

  if(!ok) {
    return -1;
  }
  // bool ok;
  // std::shared_ptr<ThreadSafeQueue<std::string>> q_ptr;
  // std::tie(q_ptr, ok) = m.subscribe(topic);
  // if(!ok) {
    // std::cout << "couldn't subscribe!" << std::endl;
    // return -1;
  // }

  std::string message;
  message.reserve(100);
  for(int i = 0; i < 100; ++i) {
    message += "a";
  }

  while(true) {
    //std::string message = q_ptr->dequeue();
    m->async_publish(topic, message);
    //auto now = std::chrono::steady_clock::now();
    //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << std::endl;
    //start = now;

    std::this_thread::sleep_for(std::chrono::microseconds(5000));
  }

  //std::this_thread::sleep_for(std::chrono::seconds(500));

  m->unsubscribe(topic);

  return 0;
}
