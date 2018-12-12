#include <mqttclient.h>
#include <iostream>


void callback(std::string topic, std::string message) {
  std::cout << topic << ": " << message << std::endl;
}

//std::function<void(std::string)> stdf_callback = &callback;

std::function<void(std::string, std::string)> stdf_callback = callback;

int main(void) {

  std::string topic = "overhead_tracker/all_robot_pose_data";

  MQTTClient m("192.168.1.8", 1884);

  m.start();

  m.subscribe(topic, stdf_callback);

  std::this_thread::sleep_for(std::chrono::seconds(30));

  m.unsubscribe(topic);

  return 0;
}
