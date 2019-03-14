#include <mqttclient_async.h>
#include <iostream>
#include <chrono>


auto start = std::chrono::steady_clock::now();

void callback(std::string topic, std::string message) {
  auto now = std::chrono::steady_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << std::endl;
  start = now;
}

std::function<void(std::string, std::string)> stdf_callback = callback;

int main(void) {

  std::string topic = "overhead_tracker/all_robot_pose_data";

  MqttClientAsync m("192.168.1.8", 1884);

  m.start();

  std::shared_ptr<std::promise<bool>> p_ptr = std::make_shared<std::promise<bool>>();
  std::future<bool> promise_future = p_ptr->get_future();

  //m.subscribe(topic, stdf_callback, p_ptr);
  auto q_ptr = m.subscribe(topic, p_ptr);

  promise_future.wait();
  bool result = promise_future.get();

  std::cout << result << std::endl;

  while(true) {
    std::string message = q_ptr->dequeue();
    auto now = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << std::endl;
    start = now;
  }

  std::this_thread::sleep_for(std::chrono::seconds(500));

  m.unsubscribe(topic);

  return 0;
}
