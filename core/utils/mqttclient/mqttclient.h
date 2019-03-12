#ifndef VIZIER_MQTT_CLIENT_H
#define VIZIER_MQTT_CLIENT_H


#include <unordered_map>
#include <string>
#include <mosquitto.h>
#include <thread>
#include <tsqueue.h>
#include <promise.h>
#include <iostream>
#include <functional>
#include <spdlog/spdlog.h>


class MQTTClient {

private:

  std::string host;
  int port;
  std::pair<std::string, std::string> empty;
  ThreadSafeQueue<std::pair<std::string, std::string>> q;
  std::thread runner;
  std::unordered_map<std::string, std::function<void(std::string, std::string)>> subscriptions;
  struct mosquitto* mosq;

public:

  MQTTClient(const std::string& host, const int port) {

    this->host = host;
    this->port = port;

    int keepalive = 20;
    bool clean_session = true;

    mosquitto_lib_init();

    mosq = mosquitto_new("overhead_tracker", clean_session, this);

    if(!mosq){
      spdlog::error("Could not allocate memory");
      throw 1;
    }

    if(mosquitto_connect_bind(mosq, host.c_str(), port, keepalive, NULL)) {
      spdlog::error("Unable to connect to MQTT broker at host {0} port {1}", host, port);
      throw 2;
    }

    spdlog::info("Connected to MQTT broker at host: {0}, port: {1}", host, port);

    //Set message callback and start the loop!
    mosquitto_message_callback_set(mosq, &MQTTClient::message_callback_static);
    mosquitto_loop_start(mosq);
  }

  ~MQTTClient(void) {
    this->q.enqueue(this->empty);
    runner.join();
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
  }

  void start() {
    runner = std::thread(&MQTTClient::publish_loop, this);
  }
  
  /**
   *  Passed to a thread when the class is initialized.  Handles publication of messages from queue. 
   * */
  void publish_loop(void) {
    while(true) {
      auto message = q.dequeue();

      if(message == this->empty) {
        std::cout << "Stopping MQTT client publisher..." << std::endl;
        break;
      }

      mosquitto_publish(mosq, NULL, message.first.c_str() , message.second.length(), message.first.c_str(), 1, false);
    }
  }

  void subscribe(const std::string& topic, const std::function<void(std::string, std::string)>& f) {
    //Struct, ?, topic string, QOS
    this->subscriptions[topic] = f;
    mosquitto_subscribe(mosq, NULL, topic.c_str(), 1);
  }

  void unsubscribe(const std::string& topic) {
    mosquitto_unsubscribe(mosq, NULL, topic.c_str());
    this->subscriptions.erase(topic.c_str());
  }

  void async_publish(const std::string& topic, const std::string& message) {
    std::pair<std::string, std::string> data = {topic, message};

    if(data == this->empty) {
      spdlog::warn("Cannot publish empty message.");
      return;
    }

    // We can just move this in b.c. we don't care about dt in this scope anymore
    q.enqueue(std::move(data));
  }

private:

  // Callback for handling incoming MQTT messages
  void message_callback(struct mosquitto *mosq, const struct mosquitto_message *message) {
    if(this->subscriptions.find(message->topic) != this->subscriptions.end()) {
      this->subscriptions[message->topic](std::string((char*) message->topic), std::string((char*) message->payload));
    }
  }

  static void message_callback_static(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
	  static_cast<MQTTClient*>(userdata)->message_callback(mosq, message);
  }
};

#endif
